#ifndef OAPLUGININTERFACE
#define OAPLUGININTERFACE
#include <QObject>
#include <QUrl>
#include <QtPlugin>
#include <QPixmap>

struct PluginSpec {
    QString name;//插件显示名称
    QString basename;//插件基本名称
    QString pluginIID;
    QString info;
    QString description;
    QString author;
    QString version;
    QPixmap icon;
    bool hasSettings;

    PluginSpec() {
        hasSettings = false;
    }

    bool operator==(const PluginSpec &other) const {
        return (this->name == other.name &&
                this->basename == other.basename &&
                this->pluginIID == other.pluginIID &&
                this->info == other.info &&
                this->description == other.description &&
                this->author == other.author &&
                this->version == other.version);
    }
};

class QTranslator;
//! [0]
class OAPluginInterFace
{
public:
    //Enum
    enum InitState { StartupInitState, LateInitState };

    //base
    virtual ~OAPluginInterFace() {}
    virtual void init(InitState state, const QString &settingsPath) = 0;
    virtual PluginSpec pluginSpec() = 0;
    virtual QTranslator* getTranslator(const QString &locale) { Q_UNUSED(locale) return 0; }

//    //oasnapshot
//     virtual void capture(QWidget *wnd = 0) {Q_UNUSED(wnd)}

//    //stikynote
//    virtual void createNote(const QString &str = QString()) {Q_UNUSED(str)}
//    virtual void appendAPI(const QString &name,const QUrl &url) {Q_UNUSED(name) Q_UNUSED(url)}
//    virtual void updateToken(const QString &name) {Q_UNUSED(name)}
};

QT_BEGIN_NAMESPACE
#define OAPluginInterFace_iid "com.24om.oa.client.OAPluginInterFace/1.0"

Q_DECLARE_INTERFACE(OAPluginInterFace, OAPluginInterFace_iid)
QT_END_NAMESPACE
#endif // OAPLUGININTERFACE
