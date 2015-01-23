#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

//#include "../pch.h"
#include <QObject>
#include <QHash>
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QHttpMultiPart;
class QSslError;
typedef void (*Functor)(QByteArray&);
class OANetworkReply:public QObject
{
    Q_OBJECT
public:
    explicit OANetworkReply(QNetworkReply *reply, QObject *parent=0);
    void setIsDebug(bool setIsDebug);
    void abort();
signals:
    void onSucess(QByteArray &data);
    void onError(QString &errorMessage);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void encrypted();
    void finished();
    void metaDataChanged();
    void sslErrors(const QList<QSslError> & errors);
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
private:
    QNetworkReply *reply;
    Functor sucess;
    Functor fail;
    bool _isDebug;
};



class Callback:public QObject
{
    Q_OBJECT
public:
    explicit Callback(Functor sucess,Functor fail,QObject *parent=0):QObject(parent)
    {
        this->sucess=sucess;
        this->fail=fail;
    }

    Functor sucess;
    Functor fail;
};

class NetworkHelper : public QObject
{
    Q_OBJECT
public:
    static NetworkHelper * instance();
    ~NetworkHelper();

    OANetworkReply * get(const QString &url);
    QNetworkReply * get(const QString &url,Functor sucess,Functor fail);
    QNetworkReply * get(const QNetworkRequest &request,Functor sucess,Functor fail);
    OANetworkReply * post(const QString &url, const QByteArray &content);
    QNetworkReply * post(const QString &url,const QByteArray & data,Functor sucess,Functor fail);
    QNetworkReply * post(const QNetworkRequest & request, const QByteArray & data,Functor sucess,Functor fail);
    OANetworkReply * post(const QString &url, QHttpMultiPart *multiPart);
    QByteArray syncGet(const QString url, const QHash<QByteArray, QByteArray> &headers=QHash<QByteArray,QByteArray>());

    QNetworkAccessManager *manager();
signals:
    void finished(QNetworkReply *reply);
private slots:
    void onFinished(QNetworkReply *reply);
private:
    class Private;
    friend class Private;
    Private *d;

    explicit NetworkHelper(QObject *parent = 0);
    NetworkHelper(const NetworkHelper &) Q_DECL_EQ_DELETE;
    NetworkHelper& operator=(NetworkHelper rhs) Q_DECL_EQ_DELETE;
};

#endif // NETWORKMANAGER_H
