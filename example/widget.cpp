#include "widget.h"
#include "ui_widget.h"
#include "oasnapshotinterface.h"
#include <QPluginLoader>
#include <QFileDialog>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    OASnapshotInterface *oasnapshotInterface;
    QString absfilename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                       QApplication::applicationDirPath(),
                                                       tr("all (*.*)"));
    if(!absfilename.isEmpty()){
        QPluginLoader pluginLoader(absfilename);
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            oasnapshotInterface = qobject_cast<OASnapshotInterface *>(plugin);
        }
    }

    //connect button of capture
    connect(ui->btncapture,&QToolButton::clicked,[=]{
        if(oasnapshotInterface){
            oasnapshotInterface->capture(ui->textEdit);
        }
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
