#ifndef OASTIKYNOTE_H
#define OASTIKYNOTE_H

#include "oastikynote_global.h"
#include <oaplugininterface.h>
#include <oastikynoteinterface.h>
#include <QUrlQuery>

extern "C" Q_DECL_EXPORT void testFunc();

class OASTIKYNOTESHARED_EXPORT oaStikyNote:public QObject, OAStikynoteInterface
//class OASTIKYNOTESHARED_EXPORT oaStikyNote:public QObject, OAPluginInterFace
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.24om.oa.client.OAStikynotePlugin" FILE "oastikynoteplugin.json")
    Q_INTERFACES(OAStikynoteInterface)

public:
    explicit oaStikyNote();

    //from interface.
    PluginSpec pluginSpec();
    void init(InitState state, const QString &settingsPath);
    QTranslator* getTranslator(const QString &locale);

    void updateToken(const QString &name);
    void appendAPI(const QString &name,const QUrl &url);
    void createNote(const QString &str = QString());

    void testFunc();

private:
    QHash<quint64,FormStikyNote*> m_listStikyNote;
    QHash<QString,QUrl> m_optApi;//key->name;value:url.
    QString m_savepath;
    QString m_token;

    void setupStiky(FormStikyNote *note);
    QString webAPI(QString name,QUrlQuery query = QUrlQuery());
};

#endif // OASTIKYNOTE_H
