#include "formsnapshotsetting.h"
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
//#include "../manager/windowmanager.h"

FormSnapshotSetting::FormSnapshotSetting(QWidget *parent) :
    QDialog(parent)
{
    int width = 210, height = 120;
    this->setWindowTitle(tr("setup width and height"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->setFixedSize(width,height);

    resize(width,height);

    //width
    WLabel = new QLabel(this);
    WLabel->setText(tr("width: "));
    WLabel->setGeometry(10,10,50,30);
    inputW = new QLineEdit(this);
    inputW->setGeometry(60,10,130,30);
    inputW->setFocus();

    //height
    HLabel = new QLabel(this);
    HLabel->setText(tr("height: "));
    HLabel->setGeometry(10,50,50,30);
    inputH = new QLineEdit(this);
    inputH->setGeometry(60,50,130,30);

    //button OK
    okButton = new QPushButton(this);
    okButton->setText(tr("OK"));
    okButton->setGeometry(10,90,75,23);

    //button Cancel
    noButton = new QPushButton(this);
    noButton->setText(tr("Cancel"));
    noButton->setGeometry(115,90,75,23);

    connect(okButton,&QPushButton::clicked,[=](){
        if( inputW->text()!="" && inputH->text()!="" )
        {
            this->setWidth(inputW->text().toInt());
            this->setHeight(inputH->text().toInt());
            hide();
        }
        else
        {
//            WinMgr.showMessage(this,tr("There's no data inputed!"),tr("Notify"));
            QMessageBox::about(this,tr("Notify"),tr("There's no data inputed!"));
        }
    });
    connect(noButton,&QPushButton::clicked,[=](){
        this->setWidth(-1);
        this->setHeight(-1);

        inputW->setText("");
        inputH->setText("");
        hide();
    });

    width = 0;
    height = 0;
}

FormSnapshotSetting::~FormSnapshotSetting()
{

}

bool FormSnapshotSetting::isOk()
{
    bool b;
    if( inputW->text()!="" && inputH->text()!="" )
    {
        b=true;
    }
    // canceled:
    else if( width==-1 && height==-1 )
    {
        width=0;
        height=0;
        b=false;
    }
    return b;
}
