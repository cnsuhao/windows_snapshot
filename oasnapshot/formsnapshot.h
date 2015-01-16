#ifndef FORMSNAPSHOT_H
#define FORMSNAPSHOT_H

#define WIDTH_SHOW 74
#define HEIGHT_SHOW 21
#define HANDLER_SIZE 5
#define MIN_SIZE 4

//#include "../../lib/OMChain/controls/formsnapshotsetting.h"
//#include "../../lib/OMChain/entities/oarect.h"
//#include "../../lib/OMChain/entities/oadrawcommand.h"
#include "formsnapshotsetting.h"
#include "oarect.h"
#include "oadrawcommand.h"

#include "oasnapdefine.h"

#include <QPen>
#include <QToolBar>
#include <QMenu>
#include <QTextEdit>
//#include <QToolBar>

class FormSnapshot : public QDialog
{
    Q_OBJECT

public:
    explicit FormSnapshot(QWidget *parent = 0);
    ~FormSnapshot();

signals:
    void finishedCapture(void);

protected slots:
    void toolButtonTriggered(QAction *action);

private:
    OASnapDefine oadef;
    OARect *selectedArea;      //screen rectagle,record x,y,w,h
    FormSnapshotSetting *sizeSetting;     //dialog to set width and height
    QPixmap *infoPix;         //snapshot information background
    QPixmap *fullScreen;      //fullscreen image
    QPixmap *finishpix;
    QPixmap *bgScreen;        //blur background
    QPoint movPos;

    QToolBar *toolbar_more;
    QToolBar *toolbar;
    QWidget *infobar;
    QCursor cursorColorfulArrow;
    QCursor cursorCrosshair;
    QMenu *contextMenu;
    QTextEdit *textEditor;
    QWidget *target;

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    //void closeEvent(QCloseEvent *evt);
private:
    QIcon snapshotToolIcon(const QString &name);
    void saveScreen();
    void FinishCapture();
    void saveFullScreen();
    void showToolbarExtend(_OASnapshotInterface::DrawType type, bool visible=true);
    void showToolbar(bool visible=true);
    void showTextEditor(bool visible=true);
    QPoint calcDestPointOfDraw(QPoint dstpos,OADrawCommand *drawcmd);

    QPen m_pen;
    QFont m_font;
    QRect rectLeftTop,rectTopMiddle,rectRightTop,
            rectLeftMiddle,rectRightMiddle,
            rectLeftBottom,rectBottomMiddle,rectRightBottom;

    _OASnapshotInterface::ActionType currentAction();
    void setCurrentAction(_OASnapshotInterface::ActionType currentAction);
    _OASnapshotInterface::ActionType nextAction();
    void setNextAction(_OASnapshotInterface::ActionType nextAction);
    _OASnapshotInterface::DrawType drawType();
    OASnapDefine::ResizeDirection resizeDirection();
    void setDrawType(_OASnapshotInterface::DrawType type);
    void setResizeDirection(OASnapDefine::ResizeDirection resizeDirection);
    _OASnapshotInterface::ActionType _action;//select,move, setup width and height
    _OASnapshotInterface::ActionType _nextAction;//select,move, setup width and height
    _OASnapshotInterface::DrawType _drawType;
    OASnapDefine::ResizeDirection _resizeDirection;
    QStack<OADrawCommand*> *_drawCommandStack;
    QList<_OASnapshotInterface> _toolButtonDataList;
    QList<QAction*> _drawActionList;

    Qt::Key m_pressCtrlKey;//辅助按键

    bool flag_change;
public slots :
    void capture(QWidget *target = 0);
    QPixmap *GetFinishPix(){return finishpix;}
};
#endif // FORMSNAPSHOT_H
