#include "musickwqueryplaylistrequest.h"

MusicKWQueryPlaylistRequest::MusicKWQueryPlaylistRequest(QObject *parent)
    : MusicQueryPlaylistRequest(parent)
{
    m_pageSize = 30;
    m_queryServer = QUERY_KW_INTERFACE;
}

void MusicKWQueryPlaylistRequest::startToSearch(QueryType type, const QString &playlist)
{
    if(type == MusicQuery)
    {
        startToSearch(playlist);
    }
    else
    {
        m_queryText = playlist.isEmpty() ? "167" : playlist;
        startToPage(0);
    }
}

void MusicKWQueryPlaylistRequest::startToPage(int offset)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(offset));

    deleteAll();
    m_totalSize = 0;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_URL, false).arg(m_queryText).arg(offset).arg(m_pageSize));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicKWQueryPlaylistRequest::startToSearch(const QString &playlist)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(playlist));

    deleteAll();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(playlist));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(getDetailsFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicKWQueryPlaylistRequest::getPlaylistInfo(MusicResultsItem &item)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 getPlaylistInfo %2").arg(getClassName()).arg(item.m_id));

    MusicPagingRequest::downLoadFinished();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(item.m_id));
    MusicKWInterface::makeRequestRawHeader(&request);

    MusicSemaphoreLoop loop;
    QNetworkReply *reply = m_manager->get(request);
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    QObject::connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
#else
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
#endif
    loop.exec();

    if(!reply || reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(reply->readAll(), &ok);
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        if(!value.isEmpty())
        {
            item.m_coverUrl = value["pic"].toString();
            item.m_name = value["title"].toString();
            item.m_playCount = QString::number(value["playnum"].toULongLong());
            item.m_description = value["info"].toString();
            item.m_updateTime = QDateTime::fromMSecsSinceEpoch(value["ctime"].toULongLong() * 1000).toString(MUSIC_YEAR_FORMAT);
            item.m_nickName = value["uname"].toString();
        }
    }
}

void MusicKWQueryPlaylistRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));

    MusicQueryPlaylistRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            m_totalSize = value["total"].toString().toLongLong();
            if(value.contains("child"))
            {
                m_tags = value["ninfo"].toMap()["name"].toString();
                const QVariantList &datas = value["child"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();
                    getMorePlaylistDetails(value["sourceid"].toString());
                    TTK_NETWORK_QUERY_CHECK();
                }
            }
        }
    }

//    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicKWQueryPlaylistRequest::getDetailsFinished()
{
    TTK_LOGGER_INFO(QString("%1 getDetailsFinished").arg(getClassName()));

    MusicQueryPlaylistRequest::downLoadFinished();
    QNetworkReply *reply = TTKObject_cast(QNetworkReply*, QObject::sender());
    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(!value.isEmpty() && value.contains("musiclist"))
            {
                const QVariantList &datas = value["musiclist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation musicInfo;
                    musicInfo.m_singerName = MusicUtils::String::charactersReplaced(value["artist"].toString());
                    musicInfo.m_songName = MusicUtils::String::charactersReplaced(value["name"].toString());
                    musicInfo.m_duration = MusicTime::msecTime2LabelJustified(value["duration"].toInt() * 1000);

                    musicInfo.m_songId = value["id"].toString();
                    musicInfo.m_artistId = value["artistid"].toString();
                    musicInfo.m_albumId = value["albumid"].toString();
                    musicInfo.m_albumName = MusicUtils::String::charactersReplaced(value["album"].toString());

                    musicInfo.m_year = QString();
                    musicInfo.m_discNumber = "1";
                    musicInfo.m_trackNumber = "0";

                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicSongPicture(&musicInfo);
                    TTK_NETWORK_QUERY_CHECK();
                    musicInfo.m_lrcUrl = MusicUtils::Algorithm::mdII(KW_SONG_LRC_URL, false).arg(musicInfo.m_songId);
                    readFromMusicSongAttribute(&musicInfo, value["formats"].toString(), m_queryQuality, m_queryAllRecords);
                    TTK_NETWORK_QUERY_CHECK();

                    if(musicInfo.m_songAttrs.isEmpty())
                    {
                        continue;
                    }
                    //
                    if(!findUrlFileSize(&musicInfo.m_songAttrs)) return;
                    //
                    MusicSearchedItem item;
                    item.m_songName = musicInfo.m_songName;
                    item.m_singerName = musicInfo.m_singerName;
                    item.m_albumName = musicInfo.m_albumName;
                    item.m_duration = musicInfo.m_duration;
                    item.m_type = mapQueryServerString();
                    Q_EMIT createSearchedItem(item);
                    m_musicSongInfos << musicInfo;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
}

void MusicKWQueryPlaylistRequest::getMorePlaylistDetailsFinished()
{
    TTK_LOGGER_INFO(QString("%1 getMorePlaylistDetailsFinished").arg(getClassName()));

    QNetworkReply *reply = TTKObject_cast(QNetworkReply*, QObject::sender());
    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(reply->readAll(), &ok);
        if(ok)
        {
            const QVariantMap &value = data.toMap();
            if(value["result"].toString() == "ok")
            {
                MusicResultsItem item;
                item.m_tags = m_tags;
                item.m_coverUrl = value["pic"].toString();
                item.m_id = value["id"].toString();
                item.m_name = value["title"].toString();
                item.m_playCount = QString::number(value["playnum"].toULongLong());
                item.m_description = value["info"].toString();
                item.m_updateTime = QDateTime::fromMSecsSinceEpoch(value["ctime"].toULongLong() * 1000).toString(MUSIC_YEAR_FORMAT);
                item.m_nickName = value["uname"].toString();
                Q_EMIT createPlaylistItem(item);
            }
        }
    }
}

void MusicKWQueryPlaylistRequest::getMorePlaylistDetails(const QString &pid)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 getMorePlaylistDetails %2").arg(getClassName()).arg(pid));

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_PLAYLIST_INFO_URL, false).arg(pid));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, SIGNAL(finished()), SLOT(getMorePlaylistDetailsFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}
