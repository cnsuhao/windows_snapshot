#ifndef OASNAPSHOTINTERFACE
#define OASNAPSHOTINTERFACE

#include <QPixmap>

class QTranslator;
//! [0]
class OASnapshotInterface
{
public:
    virtual ~OASnapshotInterface() {}
    virtual void capture(QWidget *wnd = 0) = 0;
    virtual QTranslator* getTranslator(const QString &locale) { Q_UNUSED(locale) return 0; }

    //test
    virtual QString echo(const QString &message) = 0;
};

QT_BEGIN_NAMESPACE
#define OASnapshotInterface_iid "com.24om.oa.client.OASnapshotInterface"

Q_DECLARE_INTERFACE(OASnapshotInterface, OASnapshotInterface_iid)
QT_END_NAMESPACE
#endif // OASNAPSHOTINTERFACE
