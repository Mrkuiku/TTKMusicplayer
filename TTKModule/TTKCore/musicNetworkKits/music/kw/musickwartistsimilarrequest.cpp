#include "musickwartistsimilarrequest.h"
#include "musickwqueryinterface.h"

MusicKWArtistSimilarRequest::MusicKWArtistSimilarRequest(QObject *parent)
    : MusicSimilarRequest(parent)
{

}

void MusicKWArtistSimilarRequest::startToSearch(const QString &text)
{
    if(!m_manager)
    {
        return;
    }

    TTK_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(text));

    deleteAll();

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_ARTIST_SIMILAR_URL, false).arg(getArtistNameById(text)));
    MusicKWInterface::makeRequestRawHeader(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    connect(m_reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#else
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
#endif
}

void MusicKWArtistSimilarRequest::downLoadFinished()
{
    TTK_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));

    MusicSimilarRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        const QString html(m_reply->readAll());
        QRegExp regx("<li><a href=.*data-src=\"([^\"]+).*<span>(.*)</span></a></li>");
        regx.setMinimal(true);

        int pos = html.indexOf(regx);
        while(pos != -1)
        {
            TTK_NETWORK_QUERY_CHECK();

            MusicResultsItem info;
            TTK_NETWORK_QUERY_CHECK();
            info.m_id = getArtistIdName(regx.cap(2));
            TTK_NETWORK_QUERY_CHECK();

            info.m_coverUrl = regx.cap(1);
            info.m_name = regx.cap(2);
            info.m_updateTime.clear();
            Q_EMIT createSimilarItem(info);

            pos += regx.matchedLength();
            pos = regx.indexIn(html, pos);
        }
    }

    Q_EMIT downLoadDataChanged(QString());
    deleteAll();
}

QString MusicKWArtistSimilarRequest::getArtistNameById(const QString &id)
{
    QString name;
    if(id.isEmpty() || !m_manager)
    {
        return name;
    }

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_ARTIST_INFO_URL, false).arg(id));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkAccessManager manager;
    MusicSemaphoreLoop loop;
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
#if TTK_QT_VERSION_CHECK(5,15,0)
    QObject::connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
#else
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
#endif
    loop.exec();

    if(!reply || reply->error() != QNetworkReply::NoError)
    {
        return name;
    }

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(reply->readAll().replace("'", "\""), &ok);
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        name = value["name"].toString();
    }

    return name;
}

QString MusicKWArtistSimilarRequest::getArtistIdName(const QString &name)
{
    if(name.isEmpty() || !m_manager)
    {
        return QString();
    }

    QNetworkRequest request;
    request.setUrl(MusicUtils::Algorithm::mdII(KW_ARTIST_INFO_NAME_URL, false).arg(name));
    MusicKWInterface::makeRequestRawHeader(&request);

    QNetworkAccessManager manager;
    MusicSemaphoreLoop loop;
    QNetworkReply *reply = manager.get(request);
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

    QJson::Parser json;
    bool ok;
    const QVariant &data = json.parse(reply->readAll().replace("'", "\""), &ok);

    QString id;
    if(ok)
    {
        const QVariantMap &value = data.toMap();
        id = value["id"].toString();
    }

    return id;
}
