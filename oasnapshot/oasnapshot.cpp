#include "oasnapshot.h"
#include "formsnapshot.h"

#include <QTranslator>

#include <QApplication>
#include <QDateTime>
#include <QSettings>
#include <QToolButton>
#include <qDebug>

OaSnapshot::OaSnapshot()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("general"));
    QString langName = settings.value(QLatin1String("Language"), QLocale::system().name()).toString();
    qDebug()<<"OaSnapshot"<<QDateTime::currentDateTime()<<"langName"<<langName<<
              QApplication::applicationDisplayName()<<QApplication::applicationName()<<QApplication::organizationName();

//    QStringList list;
//    list << "%1/../lang/qtbasesubset_%2.qm"
//         << "%1/../lang/qt_%2.qm"
//         << "%1/../lang/qtconfig_%2.qm"
//         << "%1/../lang/browser_%2.qm"
//         << "%1/../lang/qtwebkit_%2.qm";
//    foreach(QString fileName,list){
//        QString langFile =  fileName.arg(qApp->applicationDirPath()).arg(langName);
//        QFileInfo fileInfo(langFile);
//        if(fileInfo.exists()){
//            QTranslator *translator = new QTranslator(qApp);
//            translator->load(langFile);
//            qApp->installTranslator(translator);
//        }
//    }
}

//test.
QString OaSnapshot::echo(const QString &message)
{
//    QString var = QString()<<"OaSnapshot_echo:"<<QDateTime::currentDateTime().toString()<<message;
    QString var = QString("OaSnapshot_echo:%1 \t").arg(QDateTime::currentDateTime().toString());
    if(!message.isNull())
        var.append(message);
    qDebug()<<var<<"OaSnapshot";
    return var;
}

void OaSnapshot::capture(QWidget *wnd)
{
    FormSnapshot *snapshot = new FormSnapshot();
    snapshot->window()->setWindowFlags(Qt::WindowStaysOnTopHint);//截图时窗口隐时窗口置顶
    snapshot->window()->setWindowState(Qt::WindowActive);
    snapshot->setAttribute(Qt::WA_DeleteOnClose);
    snapshot->capture(wnd);
}

QTranslator *OaSnapshot::getTranslator(const QString &locale)
{
    QTranslator* translator = new QTranslator(this);
    translator->load(locale, ":/translations/");
    return translator;
}
