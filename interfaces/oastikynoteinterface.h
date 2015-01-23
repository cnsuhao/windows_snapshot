#ifndef OASTIKYNOTEINTERFACE
#define OASTIKYNOTEINTERFACE
#include <QObject>
#include "oaplugininterface.h"

class QTranslator;
class FormStikyNote;
//! [0]
class OAStikynoteInterface:public OAPluginInterFace
{
public:
    virtual ~OAStikynoteInterface() {}
    virtual void createNote(const QString &str = QString()) = 0;
    virtual void appendAPI(const QString &name,const QUrl &url) = 0;
    virtual void updateToken(const QString &name) = 0;
};

QT_BEGIN_NAMESPACE
#define OAStikynoteInterface_iid "com.24om.oa.client.OAStikynoteInterface"

Q_DECLARE_INTERFACE(OAStikynoteInterface, OAStikynoteInterface_iid)
QT_END_NAMESPACE
#endif // OASTIKYNOTEINTERFACE
