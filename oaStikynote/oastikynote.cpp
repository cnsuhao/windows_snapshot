#include "oastikynote.h"
#include <QTranslator>
#include <QUuid>
#include <QDebug>
#include <QUrl>
#include <QToolTip>
#include <QDir>
#include <QTextEdit>
#include <QAction>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QRect>
#include "networkhelper.h"
#include "formstikynote.h"

#define UPDATE "updateNote"
#define DELETEOne "deleteNote"
#define ADDOne "addNote"
#define GETAll "getAllNote"

oaStikyNote::oaStikyNote()
{
    this->setObjectName("oaStikyNote");
}

PluginSpec oaStikyNote::pluginSpec()
{
    PluginSpec spec;
    spec.name = tr("OAStikyNote Plugin");
    spec.info = tr("Transense windows client plugin.");
    spec.description = tr("a plugin to create the stiky note.");
    spec.version = tr("0.1.0");
    spec.author = tr("vacant chan <enaldick@hotmail.com>");
    spec.icon = QPixmap(":/res/images/sticknoteico.png");
    spec.hasSettings = true;
    spec.basename = this->objectName();
    spec.pluginIID = QUuid::createUuid().toString();

    return spec;
}

void oaStikyNote::init(OAPluginInterFace::InitState state, const QString &settingsPath)
{
    qDebug() << __FUNCTION__ << "called"<<settingsPath;
    m_savepath = settingsPath;

    Q_UNUSED(state)
}

QTranslator *oaStikyNote::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/translations/");
    return translator;
}

void oaStikyNote::updateToken(const QString &name)
{
    m_token = name;
}

void oaStikyNote::appendAPI(const QString &name, const QUrl &url)
{
//    qDebug()<<"appendAPI:"<<name<<url.toString();
    m_optApi.insert(name,url);
}

void oaStikyNote::createNote(const QString &str)
{
    //QWidget *parent = 0,QString dataText = QString(),QString filename = QString()
    FormStikyNote *note = new FormStikyNote(0,str);
//    setupStiky(note);
    if(FormStikyNote::showLastNote()){
//            //1.显示所有
//            foreach(QFileInfo filinfo,FormStikyNote::getSaveLocationList()){
//                QString filename = filinfo.absoluteFilePath();
//                FormStikyNote *notes = new FormStikyNote(filename);
//                setupStiky(notes);
//            }
        //2.显示最后关闭
        QFileInfo filinfo = FormStikyNote::getSaveLocationList().first();
        QString filename = filinfo.absoluteFilePath();
        FormStikyNote *notes = new FormStikyNote(filename);
//        setupStiky(notes);
    }
}

void oaStikyNote::setupStiky(FormStikyNote *note)
{
    if(note == NULL)
        return;

    quint64 id (note->ID());
    if(m_listStikyNote.constFind(note->ID()) != m_listStikyNote.constEnd()){
        note->close();
    }else{
        m_listStikyNote.insert(id,note);

        ///内部新增
        connect(note,&FormStikyNote::addNewOne,[=](FormStikyNote* note){
            setupStiky(note);
        });

        //! 显示已保存便签
        connect(note,&FormStikyNote::onTrushNote,[=](quint64 id){//删除网络服务器便签
            m_listStikyNote.remove(note->ID());
            QUrlQuery query;
            query.addQueryItem("id",QString::number(id));

            OANetworkReply *reply_deleteNote = NetworkHelper::instance()->get(webAPI(DELETEOne,query));
            connect(reply_deleteNote,&OANetworkReply::onSucess,[=](QByteArray &/*data*/){
                qDebug()<<"stikyNote delete from serve success.";
            });
        });

        connect(note,&FormStikyNote::onClose,[=](QString text){//先进行更新，若更新失败表示未有该便笺于服务器，则新增
            m_listStikyNote.remove(note->ID());
            //just upload local stiky note to server.
            QUrlQuery query;
            query.addQueryItem("id",QString::number(note->ID()));
            query.addQueryItem("content",text);

            OANetworkReply *reply_updateNote = NetworkHelper::instance()->get(webAPI(UPDATE,query));
            connect(reply_updateNote,&OANetworkReply::onSucess,[=](QByteArray &data){
                qDebug()<<"update note sucess to server."<<data;

                QJsonObject returnobj;
                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(data, &err);
                if(err.error == QJsonParseError::NoError){
                    returnobj = doc.object();
                    if(returnobj.contains("value")){
                        bool isok = returnobj.value("value").toBool();
                        if(!isok){
                            QUrlQuery queryadd;
                            queryadd.addQueryItem("content",text);
                            OANetworkReply *reply_addNote = NetworkHelper::instance()->get(webAPI(ADDOne,queryadd));
                            connect(reply_addNote,&OANetworkReply::onSucess,[=](QByteArray &){
                                qDebug()<<"add note success and add it to server.";
                            });
                        }
                    }
                }
            });
        });

        connect(note,&FormStikyNote::onUpdateMenu,[=]{
            OANetworkReply *reply_getAllNote = NetworkHelper::instance()->get(webAPI(GETAll));
            connect(reply_getAllNote,&OANetworkReply::onSucess,[=](QByteArray &data){
                QJsonObject returnobj;
                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(data, &err);
                if(err.error == QJsonParseError::NoError){
                    QMenu *newmenu = new QMenu(tr("System Note"));

                    returnobj = doc.object();
                    if(!returnobj.contains("error_code")){
                        QJsonArray jsarray = returnobj.value("value").toArray();
                        foreach(QJsonValue jsvalue,jsarray){
                            QJsonObject objeach = jsvalue.toObject();
                            if(objeach.value("PK_ID").toInt()<=0)
                                continue;
                            QDir usrNoteDir(m_savepath);
//                            if(!usrNoteDir.exists())
//                                usrNoteDir.mkpath(usrNoteDir.absolutePath());
                            QString filename = usrNoteDir.absoluteFilePath(QString::number(objeach.value("PK_ID").toInt())+
                                                                           "."+
                                                                           FormStikyNote::getSuffix());
                            QString datatext = objeach.value("Content").toString();

                            QAction *ac = new QAction(objeach.value("CDate").toString(),newmenu);
                            ac->setToolTip(datatext);
                            connect(ac,&QAction::triggered,[=]{
                                FormStikyNote *note = NULL;
                                QFile::exists(filename)?note = new FormStikyNote(filename):
                                                        note = new FormStikyNote(0,datatext,filename);
                                note->setLocalOrnot(false);
                                setupStiky(note);
                            });
                            connect(ac,&QAction::hovered,[=]{
                                QTextEdit textedit;
                                textedit.setHtml(datatext);
                                QString tooltext(textedit.toPlainText());
                                QToolTip::showText(QCursor::pos()+QPoint(10,0), tr("preview:") + QStringLiteral("【")+ tooltext + QStringLiteral("】") , 0 , QRect(0,0,200,300));
                            });

                            newmenu->addAction(ac);
                        }

                        note->appendExtendMenu(newmenu,true);
                        note->updateExtendMenu();
                    }
                }

                reply_getAllNote->deleteLater();
            });
            connect(reply_getAllNote,&OANetworkReply::onError,[=](QString &){
                note->updateExtendMenu();
                reply_getAllNote->deleteLater();
            });
        });

        connect(note,&FormStikyNote::onSynchronous,[=](QString text){//上传至网络
            //just upload local stiky note to server.
            QUrlQuery query;
            query.addQueryItem("id",QString::number(note->ID()));
            query.addQueryItem("content",text);
            OANetworkReply *reply_updateNote = NetworkHelper::instance()->get(webAPI(UPDATE,query));
            connect(reply_updateNote,&OANetworkReply::onSucess,[=](QByteArray &data){

                QJsonObject returnobj;
                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(data, &err);
                if(err.error == QJsonParseError::NoError){
                    returnobj = doc.object();
                    qDebug()<<"update note sucess to server."<<data <<returnobj.contains("value");
                    if(returnobj.contains("value")){
                        bool isok = returnobj.value("value").toBool();
                        if(!isok){
                            QUrlQuery queryadd;
                            queryadd.addQueryItem("content",text);
                            OANetworkReply *reply_addNote = NetworkHelper::instance()->get(webAPI(ADDOne,queryadd));
                            connect(reply_addNote,&OANetworkReply::onSucess,[=](QByteArray &){
                                qDebug()<<"add note success and add it to server.";
                            });
                        }
                    }
                }
            });
        });
    }
}

QString oaStikyNote::webAPI(QString name, QUrlQuery query)
{
    QUrl url;
    if(m_optApi.find(name) != m_optApi.end())
        url = m_optApi.find(name).value();
    if(!url.isValid())
        return QString();

    query.addQueryItem("format","json");
    QString token = m_token;
    if(token.isNull() || token.isEmpty())
        query.addQueryItem("token","-1");
    else
        query.addQueryItem("token",token);

    url.setQuery(query);

    return url.toString();
}

