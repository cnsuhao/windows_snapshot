#ifndef FORMSTIKYNOTE_H
#define FORMSTIKYNOTE_H

#include <QWidget>
#include <QFileInfoList>

class QFile;
class QMenu;
namespace Ui {
class FormStikyNote;
}

class FormStikyNote : public QWidget
{
    Q_OBJECT
signals:
    void onClose(QString);
    void onTrushNote(quint64);
    void onSynchronous(QString);//同步网络便签
    void onUpdateMenu();//展开菜单时触发,用于外部更新Extend菜单项
    void addNewOne(FormStikyNote*);

public:
    explicit FormStikyNote(QWidget *parent,QString dataText = QString(),QString filename = QString());
    explicit FormStikyNote(QString filename = QString(),QWidget *parent = 0);
    explicit FormStikyNote(QFile file,QWidget *parent = 0);
    explicit FormStikyNote(QByteArray byte, QWidget *parent = 0);
    ~FormStikyNote();

    void setSaveLocation(const QString &path);//设置保存路径
    QString saveLocation(){return m_saveLocation;}
    void setSaveFilename(const QString &path);
    QString saveFilename(){return m_saveFilename;}

    quint64 ID();
    void setID(quint64 id);

    /*!
     * 追加扩展菜单项至功能按钮下的Extend菜单项下
     * \brief appendActionToMenu
     * \param action
     */
    void appendActionToMenu(QAction *action);
    /*!
     * 追加扩展菜单至功能按钮下的Extend菜单项下
     * \brief appendExtendMenu
     * \param menu
     */
    void appendExtendMenu(QMenu *menu,bool isclear = false);

    /*!
     * 返回指定大小 size 的单色图像
     * \brief getColorIcon
     * \param color
     * \param size
     * \return
     */
    static QPixmap getColorIcon(QColor color,QSize size = QSize(15,15));
    /*!
     * 返回指定路径 location 下所有便笺文件（不包含子目录），默认路径为StandaPath::data目录
     * \brief getSaveLocationList
     * \param location
     * \return
     */
    static QFileInfoList getSaveLocationList(QString location = QString());
    /*!
     * 返回当前所用便笺文件后缀（不包含 "." ）
     * \brief getSuffix
     * \return
     */
    static QString getSuffix();
    static bool showLastNote();

    int getMenuID(QMenu *menu);

    void setAutoBackup(bool isauto);

    void setLocalOrnot(bool ok){isLocal = ok;}//设置标志：是否网络系统便笺

public slots:
    void updateExtendMenu();
    void updateExtendAction(QAction *oriAction,QAction *newActon);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *, QEvent *);
    void focusOutEvent(QFocusEvent * e);
    void closeEvent(QCloseEvent *);
private:
    /*!
     * 基本设置、信号关联
     * \brief setupUI
     */
    void setupUI();
    /*!
     * 保存数据（文件名、内容、其他窗口配置）
     * \brief saveData
     * \param text
     */
    void saveData(QString text = QString());

    /*!
     * 遍历m_saveLocation目录下所有便签，并加载对应文件内容
     * \brief readFilesData
     * \param list
     */
    bool readFilesData(QString filename);

    /*!
     * 从文件中读取出窗口配置内容
     * \brief getGeometryFromStream
     */
    QByteArray getGeometryFromStream();

    /*!
     * 从文件中读出Stream中文本内容
     * \brief getPlaintText
     * \param filename  指定读取的文件
     * \param lenght    指定读取的最大长度
     * \return
     */
    QString getPlaintText(QString filename,ulong lenght = 20);

    void setupExtendPreMenu();
    void setupExtendPreAction();
private:
    Ui::FormStikyNote *ui;
    QPoint m_windowPos;
    QPoint m_pressoriPos;
    QRect m_wndOriRect;//按下按钮时，记录当时窗体大小及位置

    QString m_saveLocation;
    QString m_saveFilename;//absoult file path with filename.

    QMenu *m_GlobalMenu;
    QMenu *menu_lcoallist;
    QMenu *subMenuExtend;//extendMenu
    QString m_currentBGColor;
    bool m_autoBackup;//backto cloud [OA and other cloud.]
    bool isLocal;//本地文件:true;/网络便笺:false;

    quint64 wndId;//要获取相应ID，需使用ID()函数
    QList<QAction*> m_ExtendPreAction;//扩展功能内置菜单项
    QList<QAction*> m_ExtendAction;
    QList<QMenu*> m_ExtendMenu;
};

#endif // FORMSTIKYNOTE_H
