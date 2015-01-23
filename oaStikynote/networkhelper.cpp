/*
http://192.168.1.61/OM/WebApi/Handler/1.0/Organization/GetList.ashx?format=json&token=-1
http://192.168.1.61/OM/WebApi/Handler/1.0/Token/Get.ashx?format=json&org_id=7&login_name=zxg&password=111
http://192.168.1.61/OM/WebApi/Handler/1.0/Message/GetChatorList.ashx?format=json&token=e356db2b-faa9-4808-82f8-048cc57f289d&org=&dept=&online=1
*/
#include "networkhelper.h"

#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QNetworkDiskCache>
#include <QStandardPaths>
//#include "../util/oahelper.h"
#include <QNetworkCookieJar>
class NetworkHelper::Private
{
public:
    Private(NetworkHelper *q) :
        manager(new QNetworkAccessManager(q))
    {
        QNetworkDiskCache *diskCache = new QNetworkDiskCache(q);
        QString cachdir = QString();
        QStringList list = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
        if(list.count() > 0)
            cachdir = list[0];
        diskCache->setCacheDirectory(cachdir);

        manager->setCache(diskCache);

        cookieJar = new QNetworkCookieJar();
        manager->setCookieJar(cookieJar);
    }

    QNetworkAccessManager *manager;
    QMap<QNetworkReply*,Callback*> replyMap;
    QNetworkCookieJar *cookieJar;
};

NetworkHelper *NetworkHelper::instance()
{
    static NetworkHelper mgr;
    return &mgr;
}

NetworkHelper::NetworkHelper(QObject *parent) :
    QObject(parent),
    d(new NetworkHelper::Private(this))
{
    connect(d->manager, &QNetworkAccessManager::finished,
            this, &NetworkHelper::onFinished);
}

NetworkHelper::~NetworkHelper()
{
    if(d)
        delete d;
    d = NULL;
}

OANetworkReply *NetworkHelper::get(const QString &url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QNetworkReply *reply = d->manager->get(request);
    return new OANetworkReply(reply);
}

QNetworkReply *NetworkHelper::get(const QString &url, Functor sucess, Functor fail)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QNetworkReply *reply = d->manager->get(request);
    d->replyMap.insert(reply,new Callback(sucess,fail,this));
    return reply;
}

QNetworkReply *NetworkHelper::get(const QNetworkRequest &request, Functor sucess, Functor fail)
{
    QNetworkReply *reply =  d->manager->get(request);
    d->replyMap.insert(reply,new Callback(sucess,fail,this));
    return reply;
}

OANetworkReply *NetworkHelper::post(const QString &url, const QByteArray &content)
{
//    int contentLength = content.length();
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
//    req.setHeader(QNetworkRequest::ContentLengthHeader,contentLength);


    QNetworkReply *reply =  d->manager->post(req,content);
    return new OANetworkReply(reply);
}

QNetworkReply *NetworkHelper::post(const QString &url, const QByteArray &data, Functor sucess, Functor fail)
{
    QNetworkReply *reply =  d->manager->post(QNetworkRequest(QUrl(url)),data);
    d->replyMap.insert(reply,new Callback(sucess,fail,this));
    return reply;
}

QNetworkReply *NetworkHelper::post(const QNetworkRequest &request, const QByteArray &data, Functor sucess, Functor fail)
{
    QNetworkReply *reply =  d->manager->post(request,data);
    d->replyMap.insert(reply,new Callback(sucess,fail,this));
    return reply;
}

OANetworkReply *NetworkHelper::post(const QString &url, QHttpMultiPart *multiPart)
{
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    //req.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data");
    QNetworkReply *reply =  d->manager->post(req,multiPart);
    return new OANetworkReply(reply);
}

QByteArray NetworkHelper::syncGet(const QString url,const QHash<QByteArray, QByteArray> &headers)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    QHashIterator<QByteArray, QByteArray> i(headers);
    while (i.hasNext()) {
        i.next();
        request.setRawHeader(i.key(),i.value());
    }

    QNetworkReply *reply = d->manager->get(request);
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec();
    if(reply->error() == QNetworkReply::NoError)
        return reply->readAll();
    else
        return QByteArray("err");
}

QNetworkAccessManager *NetworkHelper::manager()
{
    return d->manager;
}

void NetworkHelper::onFinished(QNetworkReply *reply)
{
    if(d->replyMap.contains(reply)){
        Callback *callback = d->replyMap.value(reply);
        if(reply->error() == QNetworkReply::NoError)
            callback->sucess(reply->readAll());
        else
            callback->fail(reply->readAll());

        d->replyMap.remove(reply);
        reply->deleteLater();
    }
}



OANetworkReply::OANetworkReply(QNetworkReply *reply, QObject *parent):QObject(parent)
{
    this->_isDebug = true;
    this->reply = reply;
    connect(reply,&QNetworkReply::finished,this,[=](){
        QByteArray data = this->reply->readAll();
//        if(_isDebug)
//            qDebug()<<"OANetworkReply data:"<<data;

        if(this->reply->error() == QNetworkReply::NoError)
            emit onSucess(data);
//        else{
//            emit onError(this->reply->errorString());
//        }
    });

    connect(reply,&QNetworkReply::downloadProgress,[=](qint64 bytesReceived, qint64 bytesTotal){
//        qDebug()<<reply->isOpen()<<reply->isRunning()<<reply->isReadable();
        emit downloadProgress(bytesReceived,bytesTotal);
    });
    connect(reply,&QNetworkReply::uploadProgress,[=](qint64 bytesSent, qint64 bytesTotal){
        emit uploadProgress(bytesSent,bytesTotal);
    });
    connect(reply,&QNetworkReply::metaDataChanged,[=](){
        emit metaDataChanged();
    });
    connect(reply,static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code){
        emit onError(tr("Error code : %1").arg((int)code));
    });
#ifndef QT_NO_SSL
    connect(reply,&QNetworkReply::sslErrors,[=](const QList<QSslError> & errors){
        emit sslErrors(errors);
    });
    connect(reply,&QNetworkReply::encrypted,[=](){
        emit encrypted();
    });
#endif
}

void OANetworkReply::setIsDebug(bool isDebug)
{
    _isDebug = isDebug;
}

void OANetworkReply::abort()
{
    this->reply->abort();
}
