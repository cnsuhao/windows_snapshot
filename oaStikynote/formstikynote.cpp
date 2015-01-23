#include "formstikynote.h"
#include "ui_formstikynote.h"
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>
#include <QScrollBar>
#include <QDateTime>
#include <QProcess>
#include <QSettings>
#include <QShortcut>
#include <QWindow>
#include <Windows.h>
#include <QPushButton>
#include <QCheckBox>

//#define USE_BTN_ShowLocation
#define DEFAULT_WIDTH   250
#define DEFAULT_HEIGH   250
#define Suffix    "stn"
#define COLORCustumYellow "#fdfdca"
#define COLORCustumBlue "#d7f2fa"
#define COLORCustumGreen "#d0fdca"
#define COLORCustumPurple "#dcd7fe"
#define COLORCustumPink "#f4cff4"

FormStikyNote::FormStikyNote(QWidget *parent, QString dataText, QString filename) :
    QWidget(parent),
    ui(new Ui::FormStikyNote)
{
    ui->setupUi(this);
    setupUI();

//    QColor color(253, 253, 202);
//    QColor color1(215, 242, 250);
//    QColor color2(208, 253, 202);
//    QColor color3(220, 215, 254);
//    QColor color4(244, 207, 244);
//    qDebug()<<"get color name"<<color.name()//fdfdca
//           <<color1.name()//d7f2fa
//          <<color2.name()//d0fdca
//         <<color3.name()//dcd7fe
//        <<color4.name()//f4cff4
//             ;

    QDir dirsaveLocatioin(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+QDir::separator()+"stikyNote");
    if(!dirsaveLocatioin.exists())
        dirsaveLocatioin.mkpath(dirsaveLocatioin.absolutePath());
    dirsaveLocatioin.setFilter(QDir::Files|QDir::NoDotAndDotDot);
    dirsaveLocatioin.setSorting(QDir::Name);
    m_saveLocation = dirsaveLocatioin.absolutePath();//default save location;

    QFileInfo info(m_saveFilename);
//    QFontMetrics fontmetrics(ui->lbl_lcoation->font());
//    QString str = fontmetrics.elidedText(m_saveLocation,Qt::ElideRight,ui->lbl_lcoation->sizeHint().width());
    ui->lbl_lcoation->setText(info.baseName());
    ui->lbl_lcoation->setToolTip(m_saveLocation);

    if(filename.isNull() || filename.isEmpty()){
        m_saveFilename = dirsaveLocatioin.absoluteFilePath(QDateTime::currentDateTime().toString("yyMMddhhmmss")+".stn");

        QFileInfo fileinfo(saveFilename());
        setWindowTitle(QString("%1 - ").arg(fileinfo.baseName()) + windowTitle());


        if(dataText.isNull())
            dataText = "";
        ui->textEdit->setHtml(dataText);
        if(!dataText.isEmpty() && ui->btn_trash->isHidden()){
            ui->btn_trash->setVisible(true);
            emit onSynchronous(dataText);
        }
    }else{
        if(!dataText.isEmpty()){
            m_saveFilename = filename;

            QFileInfo fileinfo(saveFilename());
            setWindowTitle(QString("%1 - ").arg(fileinfo.baseName()) + windowTitle());


            ui->textEdit->setHtml(dataText);
            saveData();
            return;
        }
        if(readFilesData(filename))
            return;
    }
}

FormStikyNote::FormStikyNote(QString filename, QWidget *parent):
    QWidget(parent),
    ui(new Ui::FormStikyNote)
{
    ui->setupUi(this);
    if(filename.isNull() || filename.isEmpty())
        return;

    setupUI();

    QFileInfo fileinfo(filename);
    setWindowTitle(QString("%1 - ").arg(fileinfo.baseName()) + windowTitle());

    if(readFilesData(filename))
        return;
}

FormStikyNote::FormStikyNote(QFile file, QWidget *parent)
{
    FormStikyNote(file.fileName(),parent);
}

FormStikyNote::FormStikyNote(QByteArray byte, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormStikyNote)
{
    setupUI();

    QDir dirsaveLocatioin(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+QDir::separator()+"stikyNote");
    if(!dirsaveLocatioin.exists())
        dirsaveLocatioin.mkpath(dirsaveLocatioin.absolutePath());
    dirsaveLocatioin.setFilter(QDir::Files|QDir::NoDotAndDotDot);
    dirsaveLocatioin.setSorting(QDir::Name);
    m_saveLocation = dirsaveLocatioin.absolutePath();//default save location;

    QByteArray byte_textdata,byte_geometry;
    //1.fileName    2.TextData  3.Widget Property   4.widget color
    QString tmpfilename;
    QDataStream in(byte);
    in.setVersion(QDataStream::Qt_5_3);
    in  >>  tmpfilename;
    in  >>  byte_textdata;
    in  >>  byte_geometry;
    in  >>  m_currentBGColor;

    m_saveFilename = tmpfilename;

    QFileInfo fileinfo(saveFilename());
    setWindowTitle(QString("%1 - ").arg(fileinfo.baseName()) +windowTitle());

    ui->textEdit->setHtml(QString::fromLocal8Bit(byte_textdata));
    if(!ui->textEdit->toPlainText().isEmpty() && ui->btn_trash->isHidden()){
        ui->btn_trash->setVisible(true);
        emit onSynchronous(ui->textEdit->toHtml());
    }
    restoreGeometry(byte_geometry);
}

void FormStikyNote::setupUI()
{
    m_GlobalMenu = NULL;
    wndId = 0;
    m_autoBackup = true;
    isLocal = true;//默认标志为本地便笺
    m_currentBGColor = COLORCustumYellow;

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setMinimumSize(180,120);
    resize(DEFAULT_WIDTH,DEFAULT_HEIGH);

    setWindowTitle("OA "+windowTitle());

    ui->wnd_top->installEventFilter(this);
    ui->textEdit->installEventFilter(this);
    ui->btn_resize->installEventFilter(this);
    ui->btn_pin->setCheckable(true);
    ui->btn_pin->setChecked(false);
    ui->btn_trash->setVisible(false);
    ui->textEdit->setMouseTracking(true);
#ifdef USE_BTN_ShowLocation
    ui->btn_showLocation->setVisible(true);
#else
    ui->btn_showLocation->setVisible(false);
#endif
//    ui->lbl_Title->setAttribute(Qt::WA_TransparentForMouseEvents);//不接收鼠标事件
//    ui->lbl_Title->setAttribute(Qt::WA_TranslucentBackground);//不接收鼠标事件

    ui->textEdit->verticalScrollBar()->setVisible(false);

    if(this->isHidden())
        showNormal();

    //load setting from settings
    QSettings set;
    set.beginGroup("MainWindow/stikyNOte");
    m_autoBackup = set.value("autoBackup",true).toBool();
    set.endGroup();

    connect(ui->btn_close,&QToolButton::clicked,[=]{
        close();
    });
    connect(ui->btn_pin,&QToolButton::clicked,[=]{
        Qt::WindowFlags flags = windowFlags();
        if(ui->btn_pin->isChecked()){
#ifdef Q_OS_WIN
            SetWindowPos((HWND)(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
            this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
#endif
        }
        else
        {
#ifdef Q_OS_WIN
            SetWindowPos((HWND)(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#else
            this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
#endif
        }
    });

    connect(ui->btn_addnote,&QToolButton::clicked,[=]{
        FormStikyNote *note = new FormStikyNote(0,"");
        emit addNewOne(note);
    });
    connect(ui->btn_trash,&QToolButton::clicked,[=]{
        QSettings set;
        set.beginGroup("MainWindow/stikyNOte");
        bool show = set.value("noTipsOfDeleteDlg",true).toBool();
        set.endGroup();

        if(show){
            QWidget *wndMsg = new QWidget(0);

            Qt::WindowFlags flags = windowFlags();
            if(ui->btn_pin->isChecked()){
    #ifdef Q_OS_WIN
                SetWindowPos((HWND)(wndMsg->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    #else
                wndMsg->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
    #endif
            }
            else
            {
    #ifdef Q_OS_WIN
                SetWindowPos((HWND)(wndMsg->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    #else
                wndMsg->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
    #endif
            }
            QShortcut *escKey = new QShortcut(QKeySequence(Qt::Key_Escape), wndMsg);
            connect(escKey,&QShortcut::activated,[=]{
                wndMsg->close();
            });

            wndMsg->setWindowFlags(Qt::FramelessWindowHint);
            wndMsg->setAttribute(Qt::WA_DeleteOnClose);
            wndMsg->setWindowModality(Qt::ApplicationModal);
            QWidget *wndMain = new QWidget(wndMsg);
            QPushButton *btnok = new QPushButton(tr("OK"),wndMain);
            QPushButton *btnno = new QPushButton(tr("Cancel"),wndMain);
            QCheckBox *ckbox = new QCheckBox(tr("Do not show this dialog."),wndMain);
            QVBoxLayout *mainlayout = new QVBoxLayout(wndMsg);
            QHBoxLayout *hbox = new QHBoxLayout(wndMsg);
            QVBoxLayout *vbox_main = new QVBoxLayout(wndMain);
            QVBoxLayout *vbox_msg = new QVBoxLayout(wndMain);
            wndMain->setObjectName("StikyNoteMsg");
            wndMain->setStyleSheet(QString("QWidget#%1{border:1px solid lightgray;background:rgb(243,243,243);}").arg(wndMain->objectName()));
            btnok->setFixedSize(85,30);
            btnno->setFixedSize(85,30);
            //local or system note.
            if(isLocal)//您确定要删除？
                vbox_msg->addWidget(new QLabel(tr("Are you sure you want to delete it?")));
            else//该便笺是系统便笺，删除时，系统里的也会被删除掉，您确定要删除？
                vbox_msg->addWidget(new QLabel(tr("It's a system notes ,the system will also be deleted when you deleted it, are you sure you want to delete it?")));
            hbox->addSpacerItem(new QSpacerItem(2,2));
            hbox->addWidget(btnok);
            hbox->addWidget(btnno);

            vbox_main->addLayout(vbox_msg);
            vbox_main->addWidget(ckbox);
            vbox_main->addLayout(hbox);

            wndMain->setLayout(hbox);
            mainlayout->addWidget(wndMain);
            mainlayout->setMargin(1);
            wndMsg->setLayout(mainlayout);
            wndMsg->setMinimumSize(300,100);
            wndMsg->show();
            wndMsg->move(pos()+QPoint(width()/2-wndMsg->width()/2,height()/2-wndMsg->height()/2));
            connect(btnok,&QPushButton::clicked,[=]{
                ui->textEdit->clear();
                QFile file(m_saveFilename);
                if(file.exists()){
                    file.remove();
                }
                wndMsg->close();
                this->close();
                emit onTrushNote(ID());

                QSettings set;
                set.beginGroup("MainWindow/stikyNOte");
                set.setValue("noTipsOfDeleteDlg",!ckbox->isChecked());
                set.endGroup();
            });
            connect(btnno,&QPushButton::clicked,[=]{
                wndMsg->close();
            });
        }else{
            ui->textEdit->clear();
            QFile file(m_saveFilename);
            if(file.exists()){
                file.remove();
            }
            this->close();
            emit onTrushNote(ID());
        }
    });
    connect(ui->btn_menu,&QToolButton::clicked,[=]{
        if(!m_GlobalMenu){
            m_GlobalMenu = new QMenu(this);
            QAction *accoloryellow = new QAction(QIcon(getColorIcon(QColor(COLORCustumYellow))),tr("yellow"),this);
            QAction *accolorblue = new QAction(QIcon(getColorIcon(QColor(COLORCustumBlue))),tr("blue"),this);
            QAction *accolorgreen = new QAction(QIcon(getColorIcon(QColor(COLORCustumGreen))),tr("green"),this);
            QAction *accolorpurple = new QAction(QIcon(getColorIcon(QColor(COLORCustumPurple))),tr("purple"),this);
            QAction *accolorpink = new QAction(QIcon(getColorIcon(QColor(COLORCustumPink))),tr("pink"),this);
            QAction *aceditselectall = new QAction(tr("select all"),this);
            QAction *aceditundo = new QAction(tr("undo"),this);
            QAction *aceditredo = new QAction(tr("redo"),this);
            QAction *aceditcopy = new QAction(tr("copy"),this);
            QAction *aceditcut = new QAction(tr("cut"),this);
            QAction *aceditpaste = new QAction(tr("paste"),this);
            QAction *aceditdelete = new QAction(tr("delete"),this);//m_ExtendAction
            QMenu *aceditmore = new QMenu(tr("more..."),m_GlobalMenu);
            subMenuExtend = new QMenu(tr("Extend"),m_GlobalMenu);

            m_GlobalMenu->addAction(accoloryellow);
            m_GlobalMenu->addAction(accolorblue);
            m_GlobalMenu->addAction(accolorgreen);
            m_GlobalMenu->addAction(accolorpurple);
            m_GlobalMenu->addAction(accolorpink);
            m_GlobalMenu->insertSection(accoloryellow,tr("Change color"));
            m_GlobalMenu->addAction(aceditselectall);
            m_GlobalMenu->addAction(aceditcopy);
            m_GlobalMenu->addAction(aceditpaste);
            m_GlobalMenu->addMenu(aceditmore);
            aceditmore->addAction(aceditundo);
            aceditmore->addAction(aceditredo);
            aceditmore->addAction(aceditcut);
            aceditmore->addAction(aceditdelete);

            menu_lcoallist = new QMenu(tr("Local"),m_GlobalMenu);

            //扩展菜单
            if(m_ExtendMenu.count()){
                foreach(QMenu *menu, m_ExtendMenu)
                    subMenuExtend->addMenu(menu);
            }
            if(m_ExtendAction.count()){
                foreach(QAction *ac , m_ExtendAction)
                    subMenuExtend->addAction(ac);
            }

            setupExtendPreAction();//m_ExtendPreAction setup.

            if(m_ExtendPreAction.count()){
                foreach(QAction *ac ,m_ExtendPreAction)
                    subMenuExtend->addAction(ac);
            }

            m_GlobalMenu->insertSection(aceditselectall,tr("Edit"));
            m_GlobalMenu->addSeparator();
            m_GlobalMenu->addMenu(subMenuExtend);

            aceditselectall->setEnabled(false);
            aceditundo->setEnabled(false);
            aceditredo->setEnabled(false);
            aceditcopy->setEnabled(false);
            aceditcut->setEnabled(false);
            aceditpaste->setEnabled(false);
            aceditdelete->setEnabled(false);

            //connect

            //! [textEdit]
            connect(ui->textEdit,&QTextEdit::redoAvailable,[=](bool isok){
                aceditredo->setEnabled(isok);
            });
            connect(ui->textEdit,&QTextEdit::undoAvailable,[=](bool isok){
                aceditundo->setEnabled(isok);
            });
            connect(ui->textEdit,&QTextEdit::copyAvailable,[=](bool isok){
                aceditcopy->setEnabled(isok);
                aceditcut->setEnabled(isok);
                aceditdelete->setEnabled(isok);
            });
            //! [textEdit]
            connect(subMenuExtend,&QMenu::aboutToShow,[=]{
                emit onUpdateMenu();
            });
            //! [m_GlobalMenu]

            connect(m_GlobalMenu,&QMenu::aboutToShow,[=]{
                //显示本地记录文件菜单
                menu_lcoallist->clear();
                menu_lcoallist->setEnabled(false);
                if(getSaveLocationList().count())
                {
                    menu_lcoallist->setEnabled(true);

                    //! 显示已保存便签（本地path）
                    foreach(QFileInfo filinfo,FormStikyNote::getSaveLocationList()){
                        QAction *ac = new QAction(QString("%1 (%2)").arg(filinfo.baseName(),getPlaintText(filinfo.absoluteFilePath())),menu_lcoallist);
                        ac->setData(filinfo.baseName());
//                        QFontMetrics fontmetrics(ui->lbl_lcoation->font());
//                        QString str = fontmetrics.elidedText(m_saveFilename,Qt::ElideRight,ui->lbl_lcoation->sizeHint().width());
//                        ac->setToolTip();
                        if(saveFilename() == filinfo.absoluteFilePath())
                        {
                            ac->setCheckable(true);
                            ac->setEnabled(false);
                            ac->setChecked(true);
                        }

                        menu_lcoallist->addAction(ac);
                        connect(ac,&QAction::triggered,[=]{
                            if(ac->isChecked())//在当前窗口打开的不作任何操作
                                return;
                            //查找窗口-是否已打开
                            FormStikyNote* form = NULL;
                            foreach(QWidget *wnd,qApp->allWidgets()){
                                QRegExp reg("(.*)-.*");
                                QString title(wnd->windowTitle());
                                if(reg.indexIn(title) != -1){
                                    if(ac->data().toString() == reg.cap(1).trimmed())
                                    {
                                        form = qobject_cast<FormStikyNote*>(wnd);
                                        if(form != NULL){
                                            if(form->isHidden())
                                                form->setVisible(true);
                                            form->activateWindow();
                                            form->raise();
                                            return;
                                        }
                                    }
                                }
                            }
                            if(form == NULL){
                                form = new FormStikyNote(filinfo.absoluteFilePath());
                                emit addNewOne(form);
                            }
                            if(form->isHidden()){
                                form->show();
                                form->activateWindow();
                            }
                        });
                    }
                    m_GlobalMenu->insertMenu(subMenuExtend->menuAction(),menu_lcoallist);
                }

                aceditpaste->setEnabled(ui->textEdit->canPaste());
                aceditselectall->setEnabled(!ui->textEdit->toPlainText().isEmpty());
            });
            //! [m_GlobalMenu]
            //! [action]
            //color
            connect(accoloryellow,&QAction::triggered,[=]{
                m_currentBGColor = COLORCustumYellow;
                repaint();
            });
            connect(accolorblue,&QAction::triggered,[=]{
                m_currentBGColor = COLORCustumBlue;
                repaint();
            });
            connect(accolorgreen,&QAction::triggered,[=]{
                m_currentBGColor = COLORCustumGreen;
                repaint();
            });
            connect(accolorpurple,&QAction::triggered,[=]{
                m_currentBGColor = COLORCustumPurple;
                repaint();
            });
            connect(accolorpink,&QAction::triggered,[=]{
                m_currentBGColor = COLORCustumPink;
                repaint();
            });

            //edit
            connect(aceditselectall,&QAction::triggered,ui->textEdit,&QTextEdit::selectAll);
            connect(aceditundo,&QAction::triggered,ui->textEdit,&QTextEdit::undo);
            connect(aceditredo,&QAction::triggered,ui->textEdit,&QTextEdit::redo);
            connect(aceditcopy,&QAction::triggered,ui->textEdit,&QTextEdit::copy);
            connect(aceditcut,&QAction::triggered,ui->textEdit,&QTextEdit::cut);
            connect(aceditpaste,&QAction::triggered,ui->textEdit,&QTextEdit::paste);
            connect(aceditdelete,&QAction::triggered,[=]{
                QTextCursor textcur = ui->textEdit->textCursor();
                textcur.removeSelectedText();
                ui->textEdit->setTextCursor(textcur);
            });
            //sub menu action
            //! [action]
        }
        m_GlobalMenu->exec(ui->btn_menu->mapToGlobal(QPoint(0,-m_GlobalMenu->sizeHint().height())));
    });

    connect(ui->btn_showLocation,&QToolButton::clicked,[=]{
#ifdef Q_OS_WIN
//        QFile file(ui->lbl_lcoation->toolTip());
        QDir dir(ui->lbl_lcoation->toolTip());
        if(!dir.exists(dir.absolutePath()))
            return;
        QFileInfo fileinfo(dir.absolutePath());

        qDebug()<<dir.absolutePath()<<QDir::toNativeSeparators(dir.absolutePath())<<fileinfo.absolutePath();
        QProcess::startDetached("explorer "+QDir::toNativeSeparators(fileinfo.absolutePath()));
#else
        ;
#endif
    });
}

FormStikyNote::~FormStikyNote()
{
    delete ui;
}

void FormStikyNote::setSaveLocation(const QString &path)
{
    m_saveLocation = path;
}

void FormStikyNote::setSaveFilename(const QString &path)
{
    m_saveFilename = path;
}

quint64 FormStikyNote::ID()
{
    if(wndId == 0){
        quint64 id = 0;
        QFileInfo fileinfo(m_saveFilename);
        id = fileinfo.completeBaseName().toULongLong();
        wndId = id;
    }
    return wndId;
}

void FormStikyNote::setID(quint64 id)
{
    wndId = id;
    QFile file(m_saveFilename);
    file.setFileName(QString::number(wndId) + "." + Suffix);
    m_saveFilename = file.fileName();
}

void FormStikyNote::appendActionToMenu(QAction *action)
{
    m_ExtendAction.append(action);
}

void FormStikyNote::appendExtendMenu(QMenu *menu, bool isclear)
{
    if(isclear){
        m_ExtendMenu.clear();
        m_ExtendMenu.append(menu);
    }else
        m_ExtendMenu.append(menu);

}

QPixmap FormStikyNote::getColorIcon(QColor color, QSize size)
{
    QPixmap pixcolor(size);
    QPainter painter(&pixcolor);
    painter.fillRect(pixcolor.rect(), QBrush(color));
    painter.setPen(QColor(Qt::red));
    painter.drawPixmap(pixcolor.rect(),pixcolor);
    return pixcolor;
}

QFileInfoList FormStikyNote::getSaveLocationList(QString location)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+QDir::separator()+"stikyNote";
    if(!location.isNull())
        path = location;
    QDir dirsaveLocatioin(path);
    if(!dirsaveLocatioin.exists())
        dirsaveLocatioin.mkpath(dirsaveLocatioin.absolutePath());
    dirsaveLocatioin.setFilter(QDir::Files|QDir::NoDotAndDotDot);
    dirsaveLocatioin.setSorting(QDir::Time);

    //load save data.
    QFileInfoList list = dirsaveLocatioin.entryInfoList();
    return list;
}

QString FormStikyNote::getSuffix()
{
    return Suffix;
}

bool FormStikyNote::showLastNote()
{
    QSettings set;
    set.beginGroup("MainWindow/stikyNOte");
    bool showlast = set.value("showlastnote",false).toBool();
    set.endGroup();
    return showlast;
}

void FormStikyNote::setAutoBackup(bool isauto)
{
    m_autoBackup = isauto;
    QSettings set;
    set.beginGroup("MainWindow/stikyNOte");
    set.setValue("autoBackup",isauto);
    set.endGroup();
}

int FormStikyNote::getMenuID(QMenu *menu)
{
    for(int i = 0;i<m_ExtendMenu.count();i++){
        if(m_ExtendMenu.at(i) == menu){
            return i;
        }
    }

    return -1;
}

void FormStikyNote::updateExtendMenu(/*QMenu *newMenu*/)
{
    subMenuExtend->clear();
    if(m_ExtendMenu.count()){
        foreach(QMenu *menu, m_ExtendMenu)
            subMenuExtend->addMenu(menu);
    }
    if(m_ExtendAction.count()){
        foreach(QAction *ac , m_ExtendAction)
            subMenuExtend->addAction(ac);
    }
    setupExtendPreAction();//m_ExtendPreAction setup.
    if(m_ExtendPreAction.count()){
        foreach(QAction *ac ,m_ExtendPreAction)
            subMenuExtend->addAction(ac);
    }

//    if(m_ExtendPreAction.count()){
//        foreach(QAction *ac , m_ExtendPreAction)
//        {
//            qDebug()<<m_ExtendPreAction.count()<<ac->text();
//            subMenuExtend->addAction(ac);
//        }
//    }
}

void FormStikyNote::updateExtendAction(QAction *oriAction, QAction *newActon)
{
    Q_UNUSED(oriAction)
    Q_UNUSED(newActon)
}

void FormStikyNote::mousePressEvent(QMouseEvent *event){
    m_windowPos = event->globalPos() - pos();
}

void FormStikyNote::mouseMoveEvent(QMouseEvent *event){
    move(event->globalPos() - m_windowPos);
}

bool FormStikyNote::eventFilter(QObject *obj, QEvent *ev)
{
    if(obj == ui->btn_resize){
        QMouseEvent *mv = dynamic_cast<QMouseEvent*>(ev);
        if(ev->type() == QEvent::MouseButtonPress){
            if(mv->buttons() == Qt::LeftButton){
                ui->btn_resize->grabMouse();
                m_pressoriPos = mv->globalPos();
                m_wndOriRect = this->geometry();
                return true;
            }
        }
        if(ev->type() == QEvent::MouseButtonRelease){
            ui->btn_resize->releaseMouse();
            return true;
        }
        if(ev->type() == QEvent::MouseMove){
            if(mv->buttons() == Qt::LeftButton){
                QPoint pos = QPoint(m_wndOriRect.x(),m_wndOriRect.y());
                QPoint currentGloPos = mv->globalPos() - m_pressoriPos;
                this->setGeometry(QRect(pos,
                                        QSize(m_wndOriRect.width()+currentGloPos.x(),m_wndOriRect.height()+currentGloPos.y())));
                return true;
            }
        }
        return false;
    }
    if(obj == ui->textEdit){
//        QMouseEvent *mvT = static_cast<QMouseEvent*>(ev);
        if(ev->type() == QEvent::FocusOut){
            saveData();
        }
//        if(ev->type() == QEvent::MouseMove){
////			QRect ret(QPoint(0,0),ui->lbl_Title->size());
//            qDebug()<<ret.contains(mvT->pos())<<mvT->pos();
////			return true;
//		}
		if(ev->type() == QEvent::Enter){
            if(!ui->textEdit->verticalScrollBar()->isVisible())
                ui->textEdit->verticalScrollBar()->setVisible(true);
        }
        if(ev->type() == QEvent::Leave){
            if(ui->textEdit->verticalScrollBar()->isVisible())
                ui->textEdit->verticalScrollBar()->setVisible(false);
        }

        return false;
    }
    if(obj == ui->wnd_top){
        if(ev->type() == QEvent::MouseButtonRelease /*&& dynamic_cast<QMouseEvent*>(ev)->buttons() == Qt::LeftButton*/){
            saveData();
        }
        return false;
    }
    return QWidget::eventFilter(obj,ev);
}

void FormStikyNote::focusOutEvent(QFocusEvent *)/*e*/
{
    saveData();
}

void FormStikyNote::closeEvent(QCloseEvent *)
{
    saveData();
    if(!ui->textEdit->toPlainText().isEmpty()){
        QString outstr = ui->textEdit->toHtml();
        QRegExp reg("<body.*</body>");
        if(reg.indexIn(outstr)!=-1)
            outstr = reg.cap();
        emit onClose(outstr);
    }
}

void FormStikyNote::saveData(QString text)
{
    QByteArray savebyte;
    if(text.isEmpty())
        savebyte = ui->textEdit->toHtml().toLocal8Bit();
    else
        savebyte = text.toLocal8Bit();

    if(!ui->textEdit->toPlainText().isEmpty()){
        QFile file(m_saveFilename);
        if(file.open(QIODevice::WriteOnly)){
            //1.fileName    2.TextData  3.Widget Property   4.widget color
            QDataStream out(&file);
            out.setVersion(QDataStream::Qt_5_3);
            out <<  m_saveFilename;
            out <<  savebyte;
            out <<  saveGeometry();
            out <<  m_currentBGColor;
            file.flush();
            file.close();
            if(!ui->btn_trash->isVisible()){
                ui->btn_trash->setVisible(true);
                emit onSynchronous(ui->textEdit->toHtml());
//                QFontMetrics fontmetrics(ui->lbl_lcoation->font());
//                QString str = fontmetrics.elidedText(m_saveFilename,Qt::ElideRight,ui->lbl_lcoation->sizeHint().width());
                QFileInfo info(file);
                ui->lbl_lcoation->setText(info.baseName());
                ui->lbl_lcoation->setToolTip(m_saveFilename);
            }
        }
    }
}

QByteArray FormStikyNote::getGeometryFromStream()
{
    QByteArray byte;
    QFile file_load(m_saveFilename);
    if(file_load.open(QIODevice::ReadOnly)){
        QString str;
        QByteArray bytetmp;
        QDataStream in(&file_load);
        in.setVersion(QDataStream::Qt_5_3);
        in  >>  str;
        in  >>  bytetmp;
        in  >>  byte;
        file_load.close();
    }
    return byte;
}

QString FormStikyNote::getPlaintText(QString filename, ulong lenght)
{
    QTextEdit textedit;
    QByteArray htmlbyte;
    QFile file_load(filename);
    if(file_load.open(QIODevice::ReadOnly)){
        //1.fileName    2.TextData  3.Widget Property   4.widget color
        QDataStream in(&file_load);
        in.setVersion(QDataStream::Qt_5_3);
        in  >>  QString();
        in  >>  htmlbyte;
        file_load.close();
    }
    textedit.setHtml(QString::fromLocal8Bit(htmlbyte));
    QString returnstr = textedit.toPlainText().left(lenght);
    QFontMetrics fontmetrics(ui->lbl_lcoation->font());
	int Metricslenght = ui->lbl_lcoation->sizeHint().width();
	if(Metricslenght<50)
		Metricslenght = 80;
    QString str = fontmetrics.elidedText(returnstr,Qt::ElideRight,Metricslenght);
    return str;
//    return returnstr;
}

void FormStikyNote::setupExtendPreMenu()
{
//    m_ExtendPreAction;
}

void FormStikyNote::setupExtendPreAction()
{
    m_ExtendPreAction.clear();
    QAction *subMenuSetBackup = new QAction(tr("Synchronous network notes"),this);
    QAction *subMenuShowLastNote = new QAction(tr("show last stiky notes"),this);
    subMenuShowLastNote->setCheckable(true);
    QSettings set;
    set.beginGroup("MainWindow/stikyNOte");
    subMenuShowLastNote->setChecked(set.value("showlastnote",false).toBool());
    set.endGroup();
    //auto backup
    connect(subMenuSetBackup,&QAction::triggered,[=]{
        QString outstr = ui->textEdit->toHtml();
        QRegExp reg("<body.*</body>");
        if(reg.indexIn(outstr)!=-1)
            outstr = reg.cap();
        emit onSynchronous(outstr);
    });
    connect(subMenuShowLastNote,&QAction::triggered,[=]{
        QSettings set;
        set.beginGroup("MainWindow/stikyNOte");
        set.setValue("showlastnote",subMenuShowLastNote->isChecked());
        set.endGroup();
    });
    m_ExtendPreAction.append(subMenuSetBackup);
    m_ExtendPreAction.append(subMenuShowLastNote);
}

bool FormStikyNote::readFilesData(QString filename)
{
    QFile file_load(filename);
    QFileInfo fileInfo(file_load);
    if(file_load.exists() && fileInfo.suffix().contains(Suffix)){
        if(file_load.open(QIODevice::ReadOnly))
        {
            QByteArray byte_textdata,byte_geometry;
            QString tmpfilename;
            //1.fileName    2.TextData  3.Widget Property
            QDataStream in(&file_load);
            in.setVersion(QDataStream::Qt_5_3);
            in  >>  tmpfilename;
            in  >>  byte_textdata;
            in  >>  byte_geometry;
            in  >>  m_currentBGColor;
            file_load.close();

            ui->textEdit->setHtml(QString::fromLocal8Bit(byte_textdata));
            restoreGeometry(byte_geometry);
            m_saveFilename = filename;
            if(!ui->textEdit->toPlainText().isEmpty() && ui->btn_trash->isHidden()){
                ui->btn_trash->setVisible(true);
                emit onSynchronous(ui->textEdit->toHtml());
            }

            QFileInfo info(m_saveFilename);
            if(m_saveFilename.compare(m_saveFilename,ui->lbl_lcoation->toolTip()) != 0){
//                QFontMetrics fontmetrics(ui->lbl_lcoation->font());
//                QString str = fontmetrics.elidedText(m_saveFilename,Qt::ElideRight,ui->lbl_lcoation->sizeHint().width());
                ui->lbl_lcoation->setText(info.baseName());
                ui->lbl_lcoation->setToolTip(m_saveFilename);
            }

            return true;
        }
    }
    return false;
}


void FormStikyNote::paintEvent(QPaintEvent *)/*event*/
{
    QPainter painter(this);
    QPoint start_point(0, 0);
    QPoint end_point(0, height());

    //QLinearGradient进行渐变色设置
    QLinearGradient linear_gradient(start_point, end_point);
    if(m_currentBGColor.compare(COLORCustumYellow) == 0){
        linear_gradient.setColorAt(0, QColor(253, 253, 202, 255));
        linear_gradient.setColorAt(0.5, QColor(253, 251, 182, 255));
        linear_gradient.setColorAt(1, QColor(252, 249, 164, 255));
    }
    else if(m_currentBGColor.compare(COLORCustumBlue) == 0){
        linear_gradient.setColorAt(0, QColor(215, 242, 250, 255));
        linear_gradient.setColorAt(0.5, QColor(199, 230, 247, 255));
        linear_gradient.setColorAt(1, QColor(186, 220, 244, 255));
    }
    else if(m_currentBGColor.compare(COLORCustumGreen) == 0){
        linear_gradient.setColorAt(0, QColor(208, 253, 202, 255));
        linear_gradient.setColorAt(0.5, QColor(194, 243, 189, 255));
        linear_gradient.setColorAt(1, QColor(179, 233, 176, 255));
    }
    else if(m_currentBGColor.compare(COLORCustumPurple) == 0){
        linear_gradient.setColorAt(0, QColor(220, 215, 254, 255));
        linear_gradient.setColorAt(0.5, QColor(209, 199, 254, 255));
        linear_gradient.setColorAt(1, QColor(199, 186, 254, 255));
    }
    else if(m_currentBGColor.compare(COLORCustumPink) == 0){
        linear_gradient.setColorAt(0, QColor(244, 207, 244, 255));
        linear_gradient.setColorAt(0.5, QColor(239, 190, 239, 255));
        linear_gradient.setColorAt(1, QColor(235, 176, 235, 255));
    }

    painter.save();
    painter.setBrush(linear_gradient);
//    painter.setPen(QPen(QBrush(QColor(255, 0, 0, 210)), 2));
    painter.setPen(QPen(QBrush(QColor(m_currentBGColor)), 1));
    painter.fillRect(this->rect(), QBrush(linear_gradient));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.drawRect(this->rect());
    painter.restore();
    painter.end();
}

