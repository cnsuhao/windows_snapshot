#ifndef OASNAPSHOT_H
#define OASNAPSHOT_H

#include <QObject>
#include <QtPlugin>
#include "oasnapshot_global.h"
#include "oasnapshotinterface.h"

//public QObject, EchoInterface
class OASNAPSHOTSHARED_EXPORT OaSnapshot :public QObject,OASnapshotInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.24om.oa.client.OASnapshotInterface" FILE "oasnapshotplugin.json")
    Q_INTERFACES(OASnapshotInterface)

public:
    OaSnapshot();
    QString echo(const QString &message = QString());//test.

    void capture(QWidget *wnd = 0);
    QTranslator* getTranslator(const QString &locale);
};

#endif // OASNAPSHOT_H
