#include "musicqqquerymovierequest.h"

MusicQQQueryMovieRequest::MusicQQQueryMovieRequest(QObject *parent)
    : MusicQueryMovieRequest(parent)
{
    m_pageSize = 30;
    m_queryServer = QUERY_QQ_INTERFACE;
}

void MusicQQQueryMovieRequest::startToSearch(QueryType type, const QString &text)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(text));

    deleteAll();
    m_queryText = text.trimmed();
    m_currentType = type;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_SONG_SEARCH_URL, false).arg(text).arg(0).arg(m_pageSize));
    MusicQQInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicQQQueryMovieRequest::startToPage(int offset)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(offset));

    deleteAll();
    m_totalSize = 0;
    m_pageSize = 20;

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_ARTIST_MOVIE_URL, false).arg(m_queryText).arg(m_pageSize * offset).arg(m_pageSize));
    MusicQQInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(pageDownLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicQQQueryMovieRequest::startToSingleSearch(const QString &text)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSingleSearch %2").arg(getClassName()).arg(text));

    deleteAll();
    m_queryText = text.trimmed();

    QTimer::singleShot(MT_MS, this, SLOT(singleDownLoadFinished()));
}

void MusicQQQueryMovieRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));

    MusicQueryMovieRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value.contains("data"))
            {
                value = value["data"].toMap();
                value = value["song"].toMap();
                const QVariantList &datas = value["list"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation musicInfo;
                    for(const QVariant &var : value["singer"].toList())
                    {
                        if(var.isNull())
                        {
                            continue;
                        }
                        const QVariantMap &name = var.toMap();
                        musicInfo.m_singerName = MusicUtils::String::charactersReplaced(name["name"].toString());
                        musicInfo.m_artistId = name["mid"].toString();
                    }
                    musicInfo.m_songName = MusicUtils::String::charactersReplaced(value["songname"].toString());
                    musicInfo.m_duration = MusicTime::msecTime2LabelJustified(value["interval"].toInt() * 1000);

                    musicInfo.m_songId = value["vid"].toString();
                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicMVAttribute(&musicInfo, false);
                    TTK_NETWORK_QUERY_CHECK();

                    if(musicInfo.m_songAttrs.isEmpty())
                    {
                        continue;
                    }
                    //
                    MusicSearchedItem item;
                    item.m_songName = musicInfo.m_songName;
                    item.m_singerName = musicInfo.m_singerName;
                    item.m_duration = musicInfo.m_duration;
                    item.m_type = mapQueryServerString();
                    Q_EMIT createSearchedItem(item);
                    m_musicSongInfos << musicInfo;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::pageDownLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 pageDownLoadFinished").arg(getClassName()));

    MusicPagingRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value["code"].toInt() == 0 && value.contains("data"))
            {
                value = value["data"].toMap();
                m_totalSize = value["total"].toInt();
                const QVariantList &datas = value["list"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicResultsItem info;
                    info.m_id = value["vid"].toString();
                    info.m_coverUrl = value["pic"].toString();
                    info.m_name = value["title"].toString();
                    info.m_updateTime.clear();
                    Q_EMIT createMovieInfoItem(info);
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::singleDownLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 singleDownLoadFinished").arg(getClassName()));

    MusicQueryMovieRequest::downLoadFinished();

    MusicObject::MusicSongInformation musicInfo;
    musicInfo.m_songId = m_queryText;
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVAttribute(&musicInfo, true);
    TTK_NETWORK_QUERY_CHECK();

    if(!musicInfo.m_songAttrs.isEmpty())
    {
        MusicSearchedItem item;
        item.m_songName = musicInfo.m_songName;
        item.m_singerName = musicInfo.m_singerName;
        item.m_duration = musicInfo.m_duration;
        item.m_type = mapQueryServerString();
        Q_EMIT createSearchedItem(item);
        m_musicSongInfos << musicInfo;
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicQQQueryMovieRequest::readFromMusicMVAttribute(MusicObject::MusicSongInformation *info, bool more)
{
    if(info->m_songId.isEmpty() || !m_manager)
    {
        return;
    }

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_MOVIE_INFO_URL, false).arg(info->m_songId));
    MusicQQInterface::makeRequestRawHeader(&request);

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

    QByteArray bytes = reply->readAll();
    bytes.replace("QZOutputJson=", "");
    bytes.chop(1);

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        QVariantMap value = data.toMap();
        if(value.contains("fl"))
        {
            QString urlPrefix;
            QVariantMap vlValue = value["vl"].toMap();
            QVariantList viList = vlValue["vi"].toList();
            if(!viList.isEmpty())
            {
                vlValue = viList.first().toMap();

                if(more)
                {
                    info->m_singerName = "Default";
                    info->m_songName = vlValue["ti"].toString();
                    info->m_duration = MusicTime::msecTime2LabelJustified(TTKStatic_cast(int, vlValue["td"].toString().toFloat()) * 1000);
                }

                vlValue = vlValue["ul"].toMap();
                viList = vlValue["ui"].toList();
                vlValue = viList.first().toMap();
                urlPrefix = vlValue["url"].toString();
            }

            QVariantMap flValue = value["fl"].toMap();
            const QVariantList &datas = flValue["fi"].toList();
            for(const QVariant &var : qAsConst(datas))
            {
                if(var.isNull())
                {
                    continue;
                }

                flValue = var.toMap();
                TTK_NETWORK_QUERY_CHECK();

                MusicObject::MusicSongAttribute attr;
                attr.m_size = MusicUtils::Number::sizeByte2Label(flValue["fs"].toInt());
                attr.m_format = "mp4";

                int bitrate = flValue["br"].toInt() * 10;
                if(bitrate <= 375)
                    attr.m_bitrate = MB_250;
                else if(bitrate > 375 && bitrate <= 625)
                    attr.m_bitrate = MB_500;
                else if(bitrate > 625 && bitrate <= 875)
                    attr.m_bitrate = MB_750;
                else if(bitrate > 875)
                    attr.m_bitrate = MB_1000;

                bitrate = flValue["id"].toULongLong();
                TTK_NETWORK_QUERY_CHECK();
                const QString &key = getMovieKey(bitrate, info->m_songId);
                TTK_NETWORK_QUERY_CHECK();

                if(!key.isEmpty())
                {
                    const QString &fn = QString("%1.p%2.1.mp4").arg(info->m_songId).arg(bitrate - 10000);
                    attr.m_url = QString("%1%2?vkey=%3").arg(urlPrefix).arg(fn).arg(key);
                    info->m_songAttrs.append(attr);
                }
            }
        }
    }
}

QString MusicQQQueryMovieRequest::getMovieKey(int id, const QString &videoId)
{
    if(videoId.isEmpty() || !m_manager)
    {
        return QString();
    }

    const QString &fn = QString("%1.p%2.1.mp4").arg(videoId).arg(id - 10000);

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(QQ_MOVIE_KEY_URL, false).arg(id).arg(videoId).arg(fn));
    MusicQQInterface::makeRequestRawHeader(&request);

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
        return QString();
    }

    QByteArray bytes = reply->readAll();
    bytes.replace("QZOutputJson=", "");
    bytes.chop(1);

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        if(value.contains("key"))
        {
            return value["key"].toString();
        }
    }

    return QString();
}
