#include "musickwquerymovierequest.h"

#include "qalgorithm/qdeswrapper.h"

MusicKWQueryMovieRequest::MusicKWQueryMovieRequest(QObject *parent)
    : MusicQueryMovieRequest(parent)
{
    m_pageSize = 40;
    m_queryServer = QUERY_KW_INTERFACE;
}

void MusicKWQueryMovieRequest::startToSearch(QueryType type, const QString &text)
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
    request.setUrl(MusicUtils::Algorithm::mdII(KW_SONG_SEARCH_URL, false).arg(text).arg(0).arg(m_pageSize));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicKWQueryMovieRequest::startToPage(int offset)
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
    request.setUrl(MusicUtils::Algorithm::mdII(KW_ARTIST_MOVIE_URL, false).arg(m_queryText).arg(m_pageSize).arg(offset));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(pageDownLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicKWQueryMovieRequest::startToSingleSearch(const QString &text)
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

void MusicKWQueryMovieRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));

    MusicQueryMovieRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll().replace("'", "\""), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value.contains("abslist"))
            {
                const QVariantList &datas = value["abslist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicObject::MusicSongInformation musicInfo;
                    musicInfo.m_singerName = MusicUtils::String::charactersReplaced(value["ARTIST"].toString());
                    musicInfo.m_songName = MusicUtils::String::charactersReplaced(value["SONGNAME"].toString());
                    musicInfo.m_duration = MusicTime::msecTime2LabelJustified(value["DURATION"].toInt() * 1000);

                    musicInfo.m_songId = value["MUSICRID"].toString().remove("MUSIC_");
                    TTK_NETWORK_QUERY_CHECK();
                    readFromMusicMVAttribute(&musicInfo, value["FORMATS"].toString());
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

void MusicKWQueryMovieRequest::pageDownLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 pageDownLoadFinished").arg(getClassName()));

    MusicPagingRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok;
        const QVariant &data = json.parse(m_reply->readAll().replace("'", "\""), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            m_totalSize = value["total"].toString().toLongLong();
            if(value.contains("mvlist"))
            {
                const QVariantList &datas = value["mvlist"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    MusicResultsItem info;
                    info.m_id = value["musicid"].toString();
                    info.m_coverUrl = value["pic"].toString();
                    if(!info.m_coverUrl.contains(TTK_HTTP) && !info.m_coverUrl.contains(COVER_URL_NULL))
                    {
                        info.m_coverUrl = MusicUtils::Algorithm::mdII(KW_MOVIE_COVER_URL, false) + info.m_coverUrl;
                    }
                    info.m_name = value["name"].toString();
                    info.m_updateTime.clear();
                    Q_EMIT createMovieInfoItem(info);
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

void MusicKWQueryMovieRequest::singleDownLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 singleDownLoadFinished").arg(getClassName()));

    MusicQueryMovieRequest::downLoadFinished();

    MusicObject::MusicSongInformation musicInfo;
    musicInfo.m_songId = m_queryText;
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVInfo(&musicInfo);
    TTK_NETWORK_QUERY_CHECK();
    readFromMusicMVAttribute(&musicInfo, QString("MP4UL|MP4L|MP4HV|MP4"));
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

void MusicKWQueryMovieRequest::readFromMusicMVAttribute(MusicObject::MusicSongInformation *info, const QString &format)
{
    if(info->m_songId.isEmpty() || !m_manager)
    {
        return;
    }

    for(const QString &v : format.split("|"))
    {
        if(v.contains("MP4L"))
        {
            readFromMusicMVAttribute(info, "MP4L", MB_250);
        }
        else if(v.contains("MP4HV"))
        {
            readFromMusicMVAttribute(info, "MP4HV", MB_750);
        }
        else if(v.contains("MP4UL"))
        {
            readFromMusicMVAttribute(info, "MP4UL", MB_1000);
        }
        else if(v.contains("MP4"))
        {
            readFromMusicMVAttribute(info, "MP4", MB_500);
        }
    }
}

void MusicKWQueryMovieRequest::readFromMusicMVAttribute(MusicObject::MusicSongInformation *info, const QString &format, int bitrate)
{
    if(info->m_songId.isEmpty() || !m_manager)
    {
        return;
    }

    QDesWrapper des;
    const QByteArray &parameter = des.encrypt(MusicUtils::Algorithm::mdII(KW_MOVIE_ATTR_URL, false).arg(info->m_songId).arg(format).toUtf8(),
                                              MusicUtils::Algorithm::mdII(_SIGN, ALG_UNIMP_KEY, false).toUtf8());

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_MOVIE_URL, false).arg(QString(parameter)));
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

    const QByteArray &bytes = reply->readAll();
    if(!bytes.isEmpty() && !bytes.contains("res not found"))
    {
        const QString text(bytes);
        QRegExp regx(".*url=(.*)\r\nsig=");

        if(text.indexOf(regx) != -1)
        {
            MusicObject::MusicSongAttribute attr;
            attr.m_url = regx.cap(1);
            attr.m_bitrate = bitrate;
            attr.m_format = "mp4";
            if(attr.m_url.isEmpty() || info->m_songAttrs.contains(attr))
            {
                return;
            }
            //
            if(!findUrlFileSize(&attr)) return;
            //
            info->m_songAttrs.append(attr);
        }
    }
}

void MusicKWQueryMovieRequest::readFromMusicMVInfo(MusicObject::MusicSongInformation *info)
{
    if(info->m_songId.isEmpty() || !m_manager)
    {
        return;
    }

    info->m_songName = "Not Found";
    info->m_singerName = "Anonymous";

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_MOVIE_HOME_URL, false).arg(info->m_songId));
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

    const QString text(reply->readAll());
    QRegExp regx("<h1 title=\"([^<]+)\">[^>]+>([^<]+)</span></h1>");

    if(text.indexOf(regx) != -1)
    {
        info->m_songName = regx.cap(1);
        info->m_singerName = regx.cap(2);
    }
}
