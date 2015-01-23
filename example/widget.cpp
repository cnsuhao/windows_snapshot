#include "widget.h"
#include "ui_widget.h"
#include "oasnapshotinterface.h"
#include "oastikynoteinterface.h"
#include <QPluginLoader>
#include <QFileDialog>
#include <QDebug>
#include <QMetaMethod>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    oastikynote = 0;
    oasnapshotInterface = 0;

    //connect button of capture
    connect(ui->btncapture,&QToolButton::clicked,[=]{
        if(!oasnapshotInterface){
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
        }
        if(oasnapshotInterface){
            oasnapshotInterface->capture(ui->textEdit);
        }
    });

    connect(ui->btnstikynote,&QToolButton::clicked,[=]{
        if(!oastikynote){
            QString absfilename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                               QApplication::applicationDirPath(),
                                                               tr("all (*.*)"));
            ;
            if(absfilename.isEmpty())
                return;
            QPluginLoader pluginLoader(absfilename);
            QObject *plugin = pluginLoader.instance();

            QObject *obj = new QObject(0);
            for(int i = 0;i<obj->metaObject()->methodCount();i++)
                qDebug()<<obj->metaObject()->method(i).name();

            if (plugin) {
                oastikynote = qobject_cast<OAStikynoteInterface *>(plugin);
                if(oastikynote){
                    oastikynote->init(OAPluginInterFace::StartupInitState,QApplication::applicationDirPath());
                }
            }
        }
        if(oastikynote)
            oastikynote->createNote(0);
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
