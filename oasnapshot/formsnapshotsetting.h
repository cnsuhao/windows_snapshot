#ifndef FORMSNAPSHOTSETTING_H
#define FORMSNAPSHOTSETTING_H

//#include "../pch.h"
#include <QDialog>
class QLabel;
class QLineEdit;
class QPushButton;
class FormSnapshotSetting : public QDialog
{
    Q_OBJECT
public:
    explicit FormSnapshotSetting(QWidget *parent = 0);
    ~FormSnapshotSetting();

    int getWidth(){return width;}
    void setWidth(int value){this->width=value;}
    int getHeight(){return height;}
    void setHeight(int value){this->height=value;}

signals:

public slots:
    bool isOk();
private:
    QLabel *WLabel;
    QLineEdit* inputW;
    QLabel *HLabel;
    QLineEdit* inputH;
    QPushButton *okButton;
    QPushButton *noButton;

    int width, height;
};

#endif // FORMSNAPSHOTSETTING_H
