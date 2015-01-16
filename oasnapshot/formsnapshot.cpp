//see also : diagram scene example
//todo: check memory leak(method of exclusion)
//http://qt-project.org/forums/viewthread/7910
//http://stackoverflow.com/questions/17342232/qlist-avoid-memory-leak
//draw a arrow:http://www.codeproject.com/Articles/3274/Drawing-Arrows
#include "formsnapshot.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QToolButton>
#include <QComboBox>
#include <QDateTime>
#include <QPainter>
#include <QRect>
#include <QApplication>
#include <QMouseEvent>
#include <QTextEdit>
#include <QFontDialog>
#include <QDesktopWidget>
#include <QScreen>
#include <QClipboard>
#include <QStack>
#include <math.h>

const qreal Pi = 3.14;
QIcon FormSnapshot::snapshotToolIcon(const QString &name)
{
    return QIcon((QString(":/res/images/%1.png").arg(name)));
}

FormSnapshot::FormSnapshot(QWidget *parent) :
    QDialog(parent),flag_change(false)
{
    this->setMouseTracking(true);
    this->setWindowFlags(Qt::FramelessWindowHint);

    //this->grabKeyboard();
    this->setObjectName("FormSnapshot");

    m_pressCtrlKey = Qt::Key_unknown;

    _action = _OASnapshotInterface::ActionType_None;
    _nextAction = _OASnapshotInterface::ActionType_None;
    _drawType = _OASnapshotInterface::DrawType_None;
    _resizeDirection = OASnapDefine::ResizeDirection_NONE;
    _drawCommandStack = new QStack<OADrawCommand*>();

//    cursorColorfulArrow = ResMgr.cursorColorfulArrow();
    cursorColorfulArrow = QCursor(QPixmap((":/res/images/cursor_colorful_arrow.png")),0,0);
//    cursorCrosshair = ResMgr.cursorCrosshair();
    cursorCrosshair = QCursor(QPixmap((":/res/images/cursor_crosshair.png")),16,16);

    toolbar_more = 0;
    toolbar = 0;
    infobar = 0;
    textEditor = 0;
    target = 0;

    contextMenu = new QMenu(this);
    _toolButtonDataList = oadef.snapshotActionList();
    _drawActionList = QList<QAction*>();
    QActionGroup * actionGroup = new QActionGroup(this);
    QAction *action = NULL;
    foreach(_OASnapshotInterface item,_toolButtonDataList){
        if(item.name == "|"){
            action = new QAction(actionGroup);
            action->setSeparator(true);
            actionGroup = new QActionGroup(this);
        }else{
            action = new QAction(snapshotToolIcon(item.name),item.text,actionGroup);
            action->setData(item.value);
            action->setCheckable(item.isCheckable);
            action->setShortcut(QKeySequence(item.shortcut));
        }
        _drawActionList.append(action);
    }

    contextMenu->addActions(_drawActionList);
    connect(contextMenu,&QMenu::triggered,this,&FormSnapshot::toolButtonTriggered);

    finishpix=new QPixmap();
    fullScreen = new QPixmap();
    selectedArea = new OARect(QApplication::desktop()->size());
    infoPix = new QPixmap(WIDTH_SHOW,HEIGHT_SHOW);
    sizeSetting = new FormSnapshotSetting(this);

    resize(selectedArea->maxWidth(),selectedArea->maxHeight());

    *fullScreen = qApp->primaryScreen()->grabWindow(QApplication::desktop()->winId(),0,0,selectedArea->maxWidth(),selectedArea->maxHeight());
    setWindowOpacity(0.9);
}

FormSnapshot::~FormSnapshot()
{
    qDeleteAll(_drawCommandStack->begin(),_drawCommandStack->end());
    _drawCommandStack->clear();
    if(_drawCommandStack)
        delete _drawCommandStack;
    _drawCommandStack = 0;

//    qDeleteAll(_toolButtonDataList);
//    _toolButtonDataList.clear();

    qDeleteAll(_drawActionList);
    _drawActionList.clear();

    if(fullScreen)
        delete fullScreen;
    fullScreen = NULL;

    if(finishpix)
        delete finishpix;
    finishpix = NULL;

    if(selectedArea)
        delete selectedArea;
    selectedArea = NULL;

    if(bgScreen)
        delete bgScreen;
    bgScreen = NULL;

    if(sizeSetting)
        delete sizeSetting;
    sizeSetting = NULL;

    if(infoPix)
        delete infoPix;
    infoPix = NULL;

    if(toolbar_more)
        delete toolbar_more;
    toolbar_more = 0;

    if(toolbar)
        delete toolbar;
    toolbar = 0;

    if(contextMenu)
        delete contextMenu;
    contextMenu = 0;

    if(textEditor)
        delete textEditor;
    textEditor = 0;
}

void FormSnapshot::toolButtonTriggered(QAction *action)
{
    _drawType = (_OASnapshotInterface::DrawType)(action->data().toInt());
    //setCurrentAction(_OASnapshotInterface::Draw);
    setNextAction(_OASnapshotInterface::ActionType_Draw);
    this->showTextEditor(false);//截图时文本输入框问题-切换MenuAction后文本框内文字显示于截图窗口中
    showToolbarExtend(_drawType);
    switch (_drawType) {
    case _OASnapshotInterface::DrawType_Rectangle:
    case _OASnapshotInterface::DrawType_Ellipse:
    case _OASnapshotInterface::DrawType_Arrow:
    case _OASnapshotInterface::DrawType_Brush:
    case _OASnapshotInterface::DrawType_Text:
    {
        setDrawType(_drawType);
    }
        break;
    case _OASnapshotInterface::DrawType_Exit:
        this->close();
        break;
    case _OASnapshotInterface::DrawType_Save:
        this->saveScreen();
        break;
    case _OASnapshotInterface::DrawType_Finish:
        this->FinishCapture();//..（1）批注未显示在结果图片中
        break;
    case _OASnapshotInterface::DrawType_Undo:
        if(!(_drawCommandStack->isEmpty())){
            _drawCommandStack->pop();
            update();
        }
        break;
    default:
        setNextAction(_OASnapshotInterface::ActionType_None);
        setDrawType(_OASnapshotInterface::DrawType_None);
        break;
    }
}

void FormSnapshot::mousePressEvent(QMouseEvent *e)
{
    if(toolbar && toolbar->isVisible() )
    {
        QPoint toolbarLeftTop = mapToGlobal(QPoint(toolbar->x(),toolbar->y()));
        QRect toolbarRect = QRect(toolbarLeftTop.x(),toolbarLeftTop.y(),
                                  toolbar->rect().width(),toolbar->rect().height());
        if(toolbarRect.contains(e->pos()))
            return ;
    }

    if(e->buttons()&Qt::RightButton)
    {
        if(selectedArea->rect().contains(e->pos())){
            //contextMenu->exec(e->pos());
//            contextMenu->popup(e->pos());
            return;
        }else
            this->close();
    }
    if(!(e->buttons()&Qt::LeftButton))
        return;
    setCurrentAction(nextAction());

    switch (nextAction()) {
    case _OASnapshotInterface::ActionType_Resize:
        if(rectLeftTop.contains(e->pos())){//To do（3）截图区域最小化后无法反向增大"
            setResizeDirection(OASnapDefine::ResizeDirection_LEFTTOP);
        }else if(rectTopMiddle.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_UP);
        }else if(rectRightTop.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_RIGHTTOP);
        }else if(rectLeftMiddle.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_LEFT);
        }else if(rectRightMiddle.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_RIGHT);
        }else if(rectLeftBottom.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_LEFTBOTTOM);
        }else if(rectBottomMiddle.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_DOWN);
        }else if(rectRightBottom.contains(e->pos())){
            setResizeDirection(OASnapDefine::ResizeDirection_RIGHTBOTTOM);
        }else{
            setResizeDirection(OASnapDefine::ResizeDirection_NONE);
        }
        showToolbar(false);
        break;
    case _OASnapshotInterface::ActionType_Drag:
        movPos = e->pos();
        showToolbar(false);
        break;
    case _OASnapshotInterface::ActionType_Draw:{
        movPos = e->pos();
        _OASnapshotInterface::DrawType type=drawType();
//        if(type==_OASnapshotInterface::DrawType_Text){
//            qDebug()<<"_OASnapshotInterface::DrawType_Text";
//            this->setFocus();
//            this->showTextEditor(false);
//        }
        {
            switch (type) {
            case _OASnapshotInterface::DrawType_Rectangle:
            {
                OADrawRectangle *cmdRect = new OADrawRectangle();
                QRect r;
                r.setX(movPos.x()-selectedArea->x1());
                r.setY(movPos.y()-selectedArea->y1());
                cmdRect->setPen(m_pen);
                cmdRect->setRect(r);
                //cmdRect->deleteLater();
                _drawCommandStack->push(cmdRect);
                break;
            }
            case _OASnapshotInterface::DrawType_Ellipse:
            {
                OADrawEllipse *cmd= new OADrawEllipse();
                QRect r;
                r.setX(movPos.x()-selectedArea->x1());
                r.setY(movPos.y()-selectedArea->y1());
                cmd->setPen(m_pen);
                cmd->setRect(r);
                //cmd->deleteLater();
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Arrow:{
                OADrawArrow *cmd = new OADrawArrow();
                cmd->setStart(movPos-selectedArea->leftTop());
                cmd->setPen(m_pen);
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Brush:
            {
                OADrawBrush *cmd = new OADrawBrush();
                cmd->setStart(e->pos()-QPoint(selectedArea->x1(),selectedArea->y1()));
                cmd->pointList().append(QPoint(e->pos().x()-selectedArea->x1(),e->pos().y()-selectedArea->y1()));
                cmd->setPen(m_pen);
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Text:
            {
                showTextEditor();

                break;
            }
            }
        }
    }
        break;
    case _OASnapshotInterface::ActionType_None:
    case _OASnapshotInterface::ActionType_Select:
        setCurrentAction(_OASnapshotInterface::ActionType_Select);
        selectedArea->setStartPosition( e->pos() );
        showToolbar(false);
        break;
    default:
        break;
    }
    QDialog::mousePressEvent(e);
}

void FormSnapshot::mouseMoveEvent(QMouseEvent *e)
{
    QPoint p(e->x()-movPos.x(),e->y()-movPos.y());
    switch(currentAction()){
    case _OASnapshotInterface::ActionType_Select:
        selectedArea->setEndPosition( e->pos() );       
        update();
        break;
    case _OASnapshotInterface::ActionType_Drag:
        selectedArea->move(p);
        movPos = e->pos();
        update();
        break;
    case _OASnapshotInterface::ActionType_Resize:{//! 截图区域最小化后无法反向增大
        if((selectedArea->width()<10) && (selectedArea->height()<10) ){
//            if(!flag_change){
//                selectedArea->setStartPosition( e->pos());
                flag_change=true;
//            }
        }

        switch (resizeDirection()) {
        case OASnapDefine::ResizeDirection_LEFTTOP://1
//                if((selectedArea->width()<10) && (selectedArea->height()<10)){
//                    selectedArea->setStartPosition( e->pos());
//                    selectedArea->resize(selectedArea->x1(),selectedArea->y1(),selectedArea->x2()+10,selectedArea->y2()+10);
//                    setResizeDirection(OASnapDefine::ResizeDirection_RIGHTBOTTOM);
//                    break;
//                }
                selectedArea->resize(e->x(), e->y(), selectedArea->x2(), selectedArea->y2() );
            break;
        case OASnapDefine::ResizeDirection_UP:
            selectedArea->resize(selectedArea->x1(), e->y(), selectedArea->x2(), selectedArea->y2() );
            break;
        case OASnapDefine::ResizeDirection_RIGHTTOP://2
            selectedArea->resize(selectedArea->x1(), e->y(), e->x(), selectedArea->y2() );
            break;
        case OASnapDefine::ResizeDirection_LEFT:
            selectedArea->resize(e->x(), selectedArea->y1(), selectedArea->x2(), selectedArea->y2() );
            break;
        case OASnapDefine::ResizeDirection_RIGHT:
            selectedArea->resize(selectedArea->x1(), selectedArea->y1(), e->x(), selectedArea->y2() );
            break;
        case OASnapDefine::ResizeDirection_LEFTBOTTOM://3
            selectedArea->resize(e->x(), selectedArea->y1(), selectedArea->x2(), e->y() );
            break;
        case OASnapDefine::ResizeDirection_DOWN:
            selectedArea->resize(selectedArea->x1(), selectedArea->y1(), selectedArea->x2(), e->y() );
            break;
        case OASnapDefine::ResizeDirection_RIGHTBOTTOM://4
            if((selectedArea->width()<10) && (selectedArea->height()<10)){
                selectedArea->setStartPosition( e->pos());
                selectedArea->resize(e->x(), e->y(), selectedArea->x2()-10, selectedArea->y2()-10);
                flag_change=false;
                setResizeDirection(OASnapDefine::ResizeDirection_LEFTTOP);
                break;
            }
            selectedArea->resize(selectedArea->x1(), selectedArea->y1(), e->x(), e->y() );
            break;
        default:
            break;
        }
        update();
        break;
    }
    case _OASnapshotInterface::ActionType_Draw:
        if(selectedArea->rect().contains(e->pos())){
            setCursor(cursorCrosshair);
            if(_drawCommandStack->isEmpty())
                break;

            switch(drawType()){
            case _OASnapshotInterface::DrawType_Arrow:{
                OADrawCommand *drawcmd = _drawCommandStack->pop();
                OADrawArrow * cmd = dynamic_cast<OADrawArrow*>(drawcmd);
                cmd->setEnd(calcDestPointOfDraw(e->pos(),drawcmd));
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Brush:{
                OADrawCommand *drawcmd = _drawCommandStack->pop();
                OADrawBrush *cmd= dynamic_cast<OADrawBrush*>(drawcmd);
                cmd->pointList().append(calcDestPointOfDraw(e->pos(),drawcmd));
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Ellipse:{
                OADrawEllipse *cmd= dynamic_cast<OADrawEllipse*>(_drawCommandStack->pop());
                QRect r = cmd->rect();
                r.setWidth(e->pos().x()-selectedArea->x1()-r.x());
                r.setHeight(e->pos().y()-selectedArea->y1()-r.y());
                cmd->setRect(r);
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Filter:
                break;
            case _OASnapshotInterface::DrawType_Rectangle:{

                OADrawRectangle *cmdRect = dynamic_cast<OADrawRectangle*>(_drawCommandStack->pop());
                QRect r = cmdRect->rect();
                r.setWidth(e->pos().x()-selectedArea->x1()-r.x());
                r.setHeight(e->pos().y()-selectedArea->y1()-r.y());
                cmdRect->setRect(r);
                _drawCommandStack->push(cmdRect);
                break;
            }
            }
        }
        update();
        break;
    default://_OASnapshotInterface::NONE:
        /*if(rectLeftTop.contains(e->pos())     ||
                rectTopMiddle.contains(e->pos())    ||
                rectRightTop.contains(e->pos())     ||
                rectLeftMiddle.contains(e->pos())   ||
                rectRightMiddle.contains(e->pos())  ||
                rectLeftBottom.contains(e->pos())   ||
                rectBottomMiddle.contains(e->pos()) ||
                rectRightBottom.contains(e->pos()))
        {
            setCursor(Qt::SizeFDiagCursor);
            setNextAction(_OASnapshotInterface::RESIZE);
        }*/

        break;
    }

    if(selectedArea->rect().contains(e->pos())){
        if(drawType() == _OASnapshotInterface::DrawType_None){
            setNextAction(_OASnapshotInterface::ActionType_Drag);
            setCursor(Qt::SizeAllCursor);
        }else{
            setCursor(cursorCrosshair);
        }
    }else if(rectLeftTop.contains(e->pos())){
        setCursor(Qt::SizeFDiagCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectTopMiddle.contains(e->pos())){
        setCursor(Qt::SizeVerCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectRightTop.contains(e->pos())){
        setCursor(Qt::SizeBDiagCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectLeftMiddle.contains(e->pos())){
        setCursor(Qt::SizeHorCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectRightMiddle.contains(e->pos())){
        setCursor(Qt::SizeHorCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectLeftBottom.contains(e->pos())){
        setCursor(Qt::SizeBDiagCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectBottomMiddle.contains(e->pos())){
        setCursor(Qt::SizeVerCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else if(rectRightBottom.contains(e->pos())){
        setCursor(Qt::SizeFDiagCursor);
        setNextAction(_OASnapshotInterface::ActionType_Resize);
    }else{
        if(toolbar && toolbar->isVisible() )
        {
            QPoint toolbarLeftTop = mapToGlobal(QPoint(toolbar->x(),toolbar->y()));
            QRect toolbarRect = QRect(toolbarLeftTop.x(),toolbarLeftTop.y(),
                                      toolbar->rect().width(),toolbar->rect().height());
            if(toolbarRect.contains(e->pos())){
                setCursor(Qt::ArrowCursor);
                toolbar->setCursor(Qt::ArrowCursor);
            }else{
                setCursor(cursorColorfulArrow);
                if(drawType() == _OASnapshotInterface::DrawType_None){
                    setNextAction(_OASnapshotInterface::ActionType_Select);
                }
            }
        }else{
            setCursor(cursorColorfulArrow);
            if(drawType() == _OASnapshotInterface::DrawType_None){
                setNextAction(_OASnapshotInterface::ActionType_Select);
            }
        }
    }
    QDialog::mouseMoveEvent(e);
}

void FormSnapshot::mouseReleaseEvent(QMouseEvent *e)
{
//    qDebug()<<"mouseMoveEvent:"<<selectedArea->y1()<<selectedArea->y2()<<e->pos()<<target->pos()<<this->pos();
//    qDebug()<<"mouseReleaseEvent";

    if(e->button()&Qt::RightButton)
    {
        if(selectedArea->rect().contains(e->pos())){
            contextMenu->popup(e->pos()+QPoint(5,5));
            return;
        }
    }

    switch (currentAction()) {
    case _OASnapshotInterface::ActionType_Resize:
        showToolbar();
        break;
    case _OASnapshotInterface::ActionType_Drag:
        this->setCursor(Qt::ArrowCursor);
        showToolbar();
        break;
    case _OASnapshotInterface::ActionType_Draw:
        if(selectedArea->rect().contains(e->pos())){
            setCursor(cursorCrosshair);

            if(_drawCommandStack->isEmpty())
                break;

            switch(drawType()){
            case _OASnapshotInterface::DrawType_Arrow:{
                    OADrawCommand *drawcmd = _drawCommandStack->pop();
                    OADrawArrow * cmd = dynamic_cast<OADrawArrow*>(drawcmd);
                cmd->setEnd(calcDestPointOfDraw(e->pos(),drawcmd));//calcDestPointOfDraw
                _drawCommandStack->push(cmd);
//                qDebug()<<"push stack to var :"<<cmd->start()<<cmd->end();
                break;
            }
            case _OASnapshotInterface::DrawType_Brush:{
                OADrawCommand *drawcmd = _drawCommandStack->pop();
                OADrawBrush *cmd= dynamic_cast<OADrawBrush*>(drawcmd);
                cmd->setEnd(e->pos()-QPoint(selectedArea->x1(),selectedArea->y1()));
//                cmd->pointList().append(QPoint(e->pos().x()-selectedArea->x1(),e->pos().y()-selectedArea->y1()));
                cmd->pointList().append(calcDestPointOfDraw(e->pos(),drawcmd));
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Ellipse:{
                OADrawEllipse *cmd= dynamic_cast<OADrawEllipse*>(_drawCommandStack->pop());
                QRect r = cmd->rect();
                r.setWidth(e->pos().x()-selectedArea->x1()-r.x());
                r.setHeight(e->pos().y()-selectedArea->y1()-r.y());
                cmd->setRect(r);
                _drawCommandStack->push(cmd);
                break;
            }
            case _OASnapshotInterface::DrawType_Filter:
                break;
            case _OASnapshotInterface::DrawType_Rectangle:{

                OADrawRectangle *cmdRect = dynamic_cast<OADrawRectangle*>(_drawCommandStack->pop());
                QRect r = cmdRect->rect();
                r.setWidth(e->pos().x()-selectedArea->x1()-r.x());
                r.setHeight(e->pos().y()-selectedArea->y1()-r.y());
                cmdRect->setRect(r);
                _drawCommandStack->push(cmdRect);
                break;
            }
            }
        }
        break;
    case _OASnapshotInterface::ActionType_Select:
        _drawCommandStack->clear();
        showToolbar();
        break;
    default:
        _drawCommandStack->clear();
        showToolbar();
        break;
    }
    update();
    setCurrentAction(_OASnapshotInterface::ActionType_None);
    QDialog::mouseReleaseEvent(e);
}


void FormSnapshot::paintEvent(QPaintEvent *)
{
    int x = selectedArea->x1();
    int y = selectedArea->y1();
    int w = selectedArea->width();
    int h = selectedArea->height();

    QPainter painter(this);
    QPen pen; pen.setColor(QColor::fromRgb(0,174,255)); pen.setWidth(2); pen.setStyle(Qt::SolidLine);

    painter.setPen(pen);

    painter.drawPixmap(0,0,*bgScreen);  //draw blur background

    painter.drawRect(x,y,w,h);          //draw snapshot area border

    //show snapshot area information: width, height
    painter.drawPixmap(x,y - HEIGHT_SHOW-2,*infoPix);
    painter.drawText(x+6,y-HEIGHT_SHOW/3,QString("%1 x %2").arg(w).arg(h));
//    painter.drawText(x+6,y-HEIGHT_SHOW/3,QString("%1 x %2, %3 x %4")
//                     .arg(x).arg(y).arg(x+w).arg(y+h));

    if( w!=0 && h!=0 )
    {
        QPixmap snapshot(w,h);//snapshot area
        snapshot.fill();
        QPainter snapshotPainter(&snapshot);
        snapshotPainter.drawPixmap(0,0,fullScreen->copy(x,y,w,h));
        snapshotPainter.setBrush(Qt::NoBrush);

        foreach(OADrawCommand *cmd,_drawCommandStack->toList())
        {
            QPen pen = cmd->pen();

            snapshotPainter.setPen(pen);
            QString cmdName(cmd->metaObject()->className());
            if(cmdName == "OADrawRectangle")
            {
                OADrawRectangle *cmdRect = dynamic_cast<OADrawRectangle*>(cmd);
//                if(cmdRect->rect().width()>0 && cmdRect->rect().height()>0)
                    snapshotPainter.drawRect(cmdRect->rect());
            }else if(cmdName == "OADrawEllipse")
            {
                OADrawEllipse *cmdEllipse= dynamic_cast<OADrawEllipse*>(cmd);
//                if(cmdEllipse->rect().width()>0 && cmdEllipse->rect().height()>0)
                    snapshotPainter.drawEllipse(cmdEllipse->rect());
            }else if(cmdName == "OADrawBrush")
            {
                OADrawBrush *cmdBrush= dynamic_cast<OADrawBrush*>(cmd);
                snapshotPainter.setPen(pen);

                QPoint pointStart = cmdBrush->pointList().at(0);
                //path.moveTo(pointStart);
                QPainterPath path(pointStart);
                foreach(QPoint point,cmdBrush->pointList()){
                    //path.cubicTo(pointStart,*point,*point);
                    //pointStart = *point;
                    path.lineTo(point);
                }
                snapshotPainter.drawPath(path);
                /*
            foreach(QPoint *point,cmdBrush->pointList()){
                snapshotPainter.drawPoint(*point);
            }*/
            }else if(cmdName == "OADrawArrow")
            {
                OADrawArrow *cmdArrow= dynamic_cast<OADrawArrow*>(cmd);
                //snapshotPainter.drawLine(cmdArrow->start(),cmdArrow->end());

                QLineF line(cmdArrow->start(),cmdArrow->end());
                qreal arrowSize = 20;
                double angle = ::acos(line.dx() / line.length());
                if (line.dy() >= 0)
                    angle = (Pi * 2) - angle;

                QPointF arrowP1,arrowP2;
//                QPointF arrowP1 = line.p2() - QPointF(sin(angle + Pi / 3) * arrowSize,
//                                                      cos(angle + Pi / 3) * arrowSize);
//                QPointF arrowP2 = line.p2() - QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
//                                                      cos(angle + Pi - Pi / 3) * arrowSize);

                qreal value = (abs((arrowP1.y()-line.p2().y())*(arrowP1.x()-line.p2().x()))
                         +abs((arrowP2.y()-line.p2().y())*(arrowP2.x()-line.p2().x())));
                if(sqrt(value)>arrowSize)
                {
                    arrowP1 = line.p2() - QPointF(sin(angle + Pi / 3) * arrowSize,
                                                  cos(angle + Pi / 3) * arrowSize);
                    arrowP2 = line.p2() - QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                                  cos(angle + Pi - Pi / 3) * arrowSize);
                    /*QPolygonF arrowHead;
                    arrowHead << line.p2() << arrowP1 << arrowP2;
                    snapshotPainter.drawLine(line);
                    snapshotPainter.drawPolygon(arrowHead);
                    */
                    snapshotPainter.drawLine(line);
                    snapshotPainter.drawLine(line.p2(),arrowP1);
                    snapshotPainter.drawLine(line.p2(),arrowP2);
                }
            }else if(cmdName == "OADrawText")
            {
                OADrawText *cmdText= dynamic_cast<OADrawText*>(cmd);
                //! problem....if change text size...?
                //! （2）文本框定位不准-Temp finish
                snapshotPainter.setFont(cmdText->font());
                snapshotPainter.drawText(QRect(QRect(cmdText->rect().x()+2,cmdText->rect().y()+8,cmdText->rect().width(),cmdText->rect().height())),cmdText->text());
            }

//            cmd->setPen(pen);

            //delete cmd;
            //cmd->deleteLater();
        }

        snapshotPainter.end();
        painter.drawPixmap(x,y,snapshot);
        *finishpix=snapshot;

        //snapshot.detach();
    }

    rectLeftTop = QRect(x-(HANDLER_SIZE-1)/2,y-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectTopMiddle = QRect(x + w/2 -(HANDLER_SIZE-1)/2,y-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectRightTop = QRect(x + w -(HANDLER_SIZE-1)/2,y-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectLeftMiddle = QRect(x-(HANDLER_SIZE-1)/2,y+h/2-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectRightMiddle = QRect(x+w-(HANDLER_SIZE-1)/2,y+h/2-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectLeftBottom = QRect(x-(HANDLER_SIZE-1)/2,y+h-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectBottomMiddle = QRect(x+w/2-(HANDLER_SIZE-1)/2,y+h-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    rectRightBottom = QRect(x+w-(HANDLER_SIZE-1)/2,y+h-(HANDLER_SIZE-1)/2,HANDLER_SIZE,HANDLER_SIZE);
    painter.setBrush(QBrush(QColor::fromRgb(0,174,255)));
    painter.drawRect(rectLeftTop);
    painter.drawRect(rectTopMiddle);
    painter.drawRect(rectRightTop);
    painter.drawRect(rectLeftMiddle);
    painter.drawRect(rectRightMiddle);
    painter.drawRect(rectLeftBottom);
    painter.drawRect(rectBottomMiddle);
    painter.drawRect(rectRightBottom);
}

void FormSnapshot::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Shift || e->key() == Qt::Key_Alt)
        m_pressCtrlKey = static_cast<Qt::Key>(e->key());
    else
        m_pressCtrlKey = Qt::Key_unknown;
    QDialog::keyPressEvent(e);
}

void FormSnapshot::keyReleaseEvent(QKeyEvent *e)
{
    m_pressCtrlKey = Qt::Key_unknown;
    if( e->key()==Qt::Key_Escape )// esc quit
    {
        if(contextMenu->isVisible())
            contextMenu->hide();
        else
            this->close();
    }
//    else if( e->key()==Qt::Key_F )// f save screen image
//    {
//        saveFullScreen();
//        this->close();
//    }
//    else if( e->key()==Qt::Key_S )// s save selected image
//    {
//        saveScreen();
//        this->close();
//    }
}

void FormSnapshot::mouseDoubleClickEvent(QMouseEvent *e)
{
//    qDebug()<<"mouseDoubleClickEvent";
    if( currentAction()!= _OASnapshotInterface::ActionType_None )
    {
        setNextAction(_OASnapshotInterface::ActionType_None);

        sizeSetting->show();
        sizeSetting->exec();

        if( sizeSetting->isOk()==true )
        {
            QPoint pos = selectedArea->leftTop();
            selectedArea->setStartPosition(pos);
            selectedArea->setEndPosition(QPoint(pos.x()+sizeSetting->getWidth(),pos.y()+sizeSetting->getHeight()));
        }

        setNextAction(_OASnapshotInterface::ActionType_None);
    }
    else{
        if(selectedArea->rect().contains(e->pos())){
            setNextAction(_OASnapshotInterface::ActionType_None);
            this->FinishCapture();
        }
    }

}

//void FormSnapshot::closeEvent(QCloseEvent *evt)
//{
//    //this->releaseKeyboard();
//    hide();
////    if(contextMenu)
////        contextMenu->close();
//    if(toolbar)//if toolbar's parent is "this", then no need to close here:
//        toolbar->close();
//    evt->ignore();//will make form not destroied and memory leak;
//}

void FormSnapshot::saveScreen()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString str = QString("OAScreenshot_%1.png").arg(QDateTime().currentDateTime().toString("yyyyMMddHHmmss"));
    QString fileName = str;
    fileName = QFileDialog::getSaveFileName(this, tr("save images"), fileName,
                                            tr("PNG (*.png);;BMP (*.bmp);;JPEG (*.jpg);;GIF (*.gif)"), &selectedFilter, options);
    if(fileName.isEmpty())
        return;

    int x = selectedArea->x1();
    int y = selectedArea->y1();
    int w = selectedArea->width();
    int h = selectedArea->height();

    finishpix->copy(x,y,w,h).save(fileName);

//    fullScreen->copy(x,y,w,h).save(fileName);
    this->close();
}

void FormSnapshot::FinishCapture()
{
    //! （1）批注未显示在结果图片中 -finished
//    int x = selectedArea->x1();
//    int y = selectedArea->y1();
//    int w = selectedArea->width();
//    int h = selectedArea->height();

//    QPixmap img = fullScreen->copy(x,y,w,h);
    if(target){
//        OATextEdit *edit = qobject_cast<OATextEdit*>(target);
        QTextEdit *edit = qobject_cast<QTextEdit*>(target);
        if(edit){
            QUrl url(QUrl(QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz")));
            QImage image = finishpix->toImage();
            edit->document()->addResource(QTextDocument::ImageResource, url, image);
            edit->textCursor().insertImage(url.toString());
//            edit->dropImage(QUrl(QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz")),finishpix->toImage());
            close();
        }
    }
//    QMimeData *data = new QMimeData;
//    data->setImageData(finishpix->toImage());
//    QApplication::clipboard()->setMimeData(data);//finishpix->toImage());
    QApplication::clipboard()->setImage(finishpix->toImage());
    emit finishedCapture();
    this->close();
}

void FormSnapshot::saveFullScreen()
{
    fullScreen->save("fullScreen.jpg","JPG");
}

void FormSnapshot::showToolbarExtend(_OASnapshotInterface::DrawType type,bool /*visible*/)
{
    if(!toolbar_more){
        toolbar_more = new QToolBar(this);
        toolbar_more->setObjectName("FormSnapshot_toolbar_more");
        toolbar_more->setMinimumHeight(30);
        toolbar_more->setMouseTracking(true);
        toolbar_more->setStyleSheet(QString("QToolBar#%1{background-color:rgb(222,238,255);border:1px solid rgb(0,174,255);border-radius:2px;}").arg(toolbar_more->objectName()));
        toolbar_more->setAutoFillBackground(true);
        toolbar_more->setFocusPolicy(Qt::NoFocus);
        toolbar_more->setCursor(Qt::ArrowCursor);
    }
    toolbar_more->setProperty("show",true);
    toolbar_more->clear();

    QString pix(":/res/images/SinglePoint_%1.png");
    switch(type){
    case _OASnapshotInterface::DrawType_Rectangle:
    case _OASnapshotInterface::DrawType_Ellipse:
    case _OASnapshotInterface::DrawType_Arrow:
    case _OASnapshotInterface::DrawType_Brush:{
        foreach(_OASnapshotInterface snap,oadef.snapshotDrawSizeTypes()){
            QToolButton *btn = new QToolButton(toolbar_more);
            btn->setObjectName(QString("SnapShot_DrawSize_%1").arg(snap.text));
            btn->setIcon(QIcon(pix.arg(snap.Draw_ST_value)));

            btn->setProperty("sizetype",(int)snap.Draw_ST_value);
            btn->setProperty("sizetext",snap.text.toInt());
            btn->setStyleSheet("QToolButton:unchecked{border-image:transparent;background:transparent;border:0px;}"
                               "QToolButton:hover{border-image:transparent;border:1px solid gray;}"
                               "QToolButton:checked{border:1px solid gray;background:transparent;margin:0px;}"
                               "QToolButton:pressed{margin:1px;}");
            btn->setCheckable(true);
            btn->setProperty("checked",false);
            connect(btn,&QToolButton::clicked,[=]{
                foreach(QAction*ac,toolbar_more->actions()){
                    if(qobject_cast<QWidget*>(toolbar_more->widgetForAction(ac))->objectName().contains("SnapShot_DrawSize_"))
                        qobject_cast<QToolButton*>(toolbar_more->widgetForAction(ac))->setProperty("checked",false);
                }
                btn->setProperty("checked",btn->property("checked").toBool());
                btn->setChecked(!btn->isChecked());
                m_pen.setWidth(btn->property("sizetext").toInt());
            });
            if(snap.Draw_ST_value == (int)_OASnapshotInterface::Draw_SizeType_Small){
                btn->setChecked(true);
                m_pen.setWidth(btn->property("sizetext").toInt());
            }
            toolbar_more->addWidget(btn);
        }
    }
        break;
    case _OASnapshotInterface::DrawType_Text:{
        QComboBox *combox = new QComboBox(toolbar_more);
        for(int i=0;i<9;i++){
            combox->addItem(QString::number(i+12));
        }
        connect(combox,static_cast<void (QComboBox::*) (const QString &)>(&QComboBox::currentIndexChanged),[=]{
            QFont font;
            font.setPointSize(combox->currentText().toInt());
            if(textEditor){
                textEditor->setFont(font);
            }
            m_pen.setWidth(combox->currentIndex()+1);
        });

        QToolButton *lblBGFont = new QToolButton(toolbar_more);
        lblBGFont->setIcon(QPixmap(":/theme/res/theme/default/images/snapshot/text.png"));
        connect(lblBGFont,&QToolButton::clicked,[=]{
            bool ok;
            QFont font = textEditor->font();
            font = QFontDialog::getFont(&ok,font,this);
            if(ok){
                textEditor->setFont(font);
            }
        });
        combox->setFixedSize(50,20);
        toolbar_more->addWidget(lblBGFont);
        toolbar_more->addWidget(combox);
    }
        break;
    default:
        toolbar_more->setProperty("show",false);
        break;
    }

    //color select Area.
    toolbar_more->addSeparator();
    QWidget *colorWidget = new QWidget(toolbar_more);
    QHBoxLayout *hboxlayout = new QHBoxLayout;
    QGridLayout *grid = new QGridLayout;
    QToolButton *colorBtn = nullptr;
    int valueOfQtColor = OAColorInterface::OAColor_black;
    QString colorname;
    for(int i=0;i<2;i++){
        for(int j=1;j<9;j++){
            colorBtn = new QToolButton(colorWidget);//rgb(0,124,200,100)
            colorBtn->setObjectName(QString("snapshot_colorBtn_%1").arg(i+j));

            foreach(OAColorInterface oacolor,oadef.oaColorList()){
                if(oacolor.type == valueOfQtColor && oacolor.type != OAColorInterface::OAColor_none){
                    colorname = oacolor.name;
                    valueOfQtColor++;
                    break;
                }
            }

            QColor color(colorname);
//            qDebug()<<color.rgba()<<color.name()<<valueOfQtColor;
            colorBtn->setStyleSheet(QString("QToolButton#%1{border-image:transparent;background:%2;border:1px solid lightgray;}"
                                            "QToolButton#%1:hover{margin:1px;}").arg(colorBtn->objectName(),color.name()));
            colorBtn->resize(12,12);
            colorBtn->setProperty("colorName",color.name());
            grid->addWidget(colorBtn,i,j);

            connect(colorBtn,&QToolButton::clicked,[=]{
                QString colorname = colorBtn->property("colorName").toString();
                m_pen.setColor(QColor(colorname));
                QToolButton *currentBtn = colorWidget->findChild<QToolButton*>("Snap_colorBtn_Current");
                if(currentBtn){
                    currentBtn->setStyleSheet(QString("QToolButton#%1{background:%2;}QToolButton#%1:hover{border-image:transparent;}").arg(currentBtn->objectName(),colorBtn->property("colorName").toString()));
                }
                if(textEditor){
                    textEditor->setTextColor(m_pen.color());
                }
            });
        }
    }
    QColor defaultColor = QColor(OAColorInterface::OAColorName(OAColorInterface::OAColor_red));
    QToolButton *colorCurrentBtn = new QToolButton(colorWidget);
    colorCurrentBtn->setObjectName("Snap_colorBtn_Current");
    colorCurrentBtn->setMinimumSize(25,25);
    colorCurrentBtn->setStyleSheet(QString("QToolButton#%1{background:%2;border-image:transparent;border:1px solid rgb(255,0,100,100);}QToolButton#%1:hover{border-image:transparent;border:1px solid gray;}").
                                   arg(colorCurrentBtn->objectName(),defaultColor.name()));
    colorCurrentBtn->setProperty("colorName",defaultColor.name());
    //default color
    m_pen.setColor(defaultColor);
    grid->addWidget(colorCurrentBtn,0,0,2,1);
    grid->setSpacing(2);
    hboxlayout->addItem(grid);
    hboxlayout->setMargin(0);
    hboxlayout->setSpacing(6);
    colorWidget->setLayout(hboxlayout);
    toolbar_more->addWidget(colorWidget);
    if(textEditor){
        textEditor->setTextColor(m_pen.color());
    }
    //connect
    connect(colorCurrentBtn,&QToolButton::clicked,[=]{
        QString colorname = colorCurrentBtn->property("colorName").toString();
        m_pen.setColor(QColor(colorname));
        if(textEditor){
            textEditor->setTextColor(m_pen.color());
        }
    });

    toolbar_more->move(toolbar->pos()+this->pos() + QPoint(0,toolbar->height()));
    toolbar_more->adjustSize();
    toolbar_more->setVisible(toolbar_more->property("show").toBool());
}

void FormSnapshot::showToolbar(bool visible)
{
    if(!toolbar){
        toolbar = new QToolBar(this);
        toolbar->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        //toolbar->setWindowOpacity(1);
        toolbar->sizePolicy().setHorizontalPolicy(QSizePolicy::Preferred);
        toolbar->setCursor(Qt::ArrowCursor);
        toolbar->setMouseTracking(true);
//        toolbar->setContentsMargins(0,0,0,0);
//        toolbar->layout()->setContentsMargins(0,0,0,0);
//        toolbar->layout()->setSpacing(0);

        toolbar->addActions(_drawActionList);
        //could not connect the actionTriggered signal here, or the slot will be invoke twice:
        //connect(toolbar,&QToolBar::actionTriggered,this,&FormSnapshot::toolButtonTriggered);

        toolbar->setStyleSheet("background-color:rgb(222,238,255)");
        toolbar->setAutoFillBackground(true);
        toolbar->setFocusPolicy(Qt::NoFocus);
        toolbar->actions().at(0)->setChecked(true);

    }

    if(visible)
    {
        toolbar->actions().at(0)->setChecked(true);
        //!position change while cature window on bottom--vacant-finished
        this->toolbar->setWindowFlags(toolbar->windowFlags() | Qt::WindowStaysOnTopHint);
        int pos_selected_x=selectedArea->x1();
        if((selectedArea->width())>(this->toolbar->width()))
            pos_selected_x=(selectedArea->x2()-selectedArea->x1())/2-toolbar->width()/2+selectedArea->x1();

        if(selectedArea->height()>(QApplication::desktop()->height()-this->toolbar->height()))
            toolbar->move(pos_selected_x,5);
        else{
            if(selectedArea->y2()>QApplication::desktop()->height()-this->toolbar->height()){
                toolbar->move(pos_selected_x,selectedArea->y1()-this->toolbar->height()-2);
            }else toolbar->move(pos_selected_x,selectedArea->y1()+selectedArea->height()+4);
        }
        toolbar->show();
        //toolbar->grabMouse();
    }else{
        toolbar->hide();
        //toolbar->releaseMouse();
    }
}

//To do-->（2）文本框定位不准
void FormSnapshot::showTextEditor(bool visible)
{
    if(!textEditor)
    {
        textEditor = new QTextEdit(this);
        textEditor->setObjectName("txtEditor");
        textEditor->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        //textEditor->setVerticalScrollBar();
        textEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff) ;
        textEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        textEditor->setLineWrapMode(QTextEdit::NoWrap);
//        textEditor->setContentsMargins(0,0,0,0);
//        textEditor->setAlignment(Qt::AlignTop);

//        QSizePolicy localSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//        textEditor->setSizePolicy(localSizePolicy);
//        QPalette plt = textEditor->palette();
//        plt.setColor(QPalette::Background,Qt::transparent);
//        textEditor->setPalette(plt);
//        textEditor->setStyleSheet("border:1px rgb(0,0,0) solid;");
        //textEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        connect(textEditor,&QTextEdit::textChanged,[=](){
            textEditor->setFixedSize(textEditor->document()->size().width(),//+textEditor->contentsMargins().left()*2,
                            textEditor->document()->size().height() //+ textEditor->contentsMargins().top()*2
                        );
        });

    }
    if(!(textEditor->document()->isEmpty()))
    {
        OADrawText *cmd = new OADrawText();
        cmd->setText(textEditor->toPlainText());
        QRect rect;
        rect.setX(textEditor->x()-selectedArea->rect().x());
        rect.setY(textEditor->y()-selectedArea->rect().y());
        rect.setWidth(textEditor->width());
        rect.setHeight(textEditor->height());
        cmd->setRect(rect);
        textEditor->setTextColor(m_pen.color());
        cmd->setPen(m_pen);
        cmd->setFont(textEditor->font());
        _drawCommandStack->push(cmd);
        textEditor->clear();
    }
    if(visible)
    {
        //textEditor->move(QCursor::pos());
        textEditor->setGeometry(QCursor::pos().x(),QCursor::pos().y(), 60, 30);
        textEditor->show();
        textEditor->setFocus();
    }else{
        textEditor->hide();
        textEditor->clear();
    }
}

QPoint FormSnapshot::calcDestPointOfDraw(QPoint dstpos, OADrawCommand *drawcmd)
{
    QPoint dstPoint;
    switch(drawType()){
        case _OASnapshotInterface::DrawType_Arrow:{
            OADrawArrow * cmd = dynamic_cast<OADrawArrow*>(drawcmd);
            if(m_pressCtrlKey == Qt::Key_Shift){
                dstPoint = QPoint(dstpos.x()-selectedArea->leftTop().x(),cmd->start().y());
            }else if(m_pressCtrlKey == Qt::Key_Alt)
                dstPoint = QPoint(cmd->start().x(),dstpos.y()-selectedArea->leftTop().y());
            else
                dstPoint = dstpos-selectedArea->leftTop();
            break;
        }
        case _OASnapshotInterface::DrawType_Brush:{
            OADrawBrush * cmd = dynamic_cast<OADrawBrush*>(drawcmd);
            if(m_pressCtrlKey == Qt::Key_Shift){
                int xpos = dstpos.x()-selectedArea->x1();
                dstPoint = QPoint(xpos,cmd->start().y());
            }else if(m_pressCtrlKey == Qt::Key_Alt){
                int ypos = dstpos.y()-selectedArea->y1();
                dstPoint = QPoint(cmd->start().x(),ypos);
            }
            else
                dstPoint = QPoint(dstpos.x()-selectedArea->x1(),dstpos.y()-selectedArea->y1());
            break;
        }
//        case _OASnapshotInterface::DrawType_Ellipse:{
//            OADrawEllipse *cmd= dynamic_cast<OADrawEllipse*>(drawcmd);
//            if(m_pressCtrlKey == Qt::Key_Shift){
//                dstPoint = QPoint(dstpos->pos().x()-selectedArea->leftTop().x(),cmd->start().y());
//            }else if(m_pressCtrlKey == Qt::Key_Alt)
//                dstPoint = QPoint(cmd->start().x(),dstpos->pos().y()-selectedArea->leftTop().y());

//            QRect r = cmd->rect();
//            r.setWidth(dstpos.pos().x()-selectedArea->x1()-r.x());
//            r.setHeight(dstpos.pos().y()-selectedArea->y1()-r.y());
//            cmd->setRect(r);
//            _drawCommandStack->push(cmd);
//            break;
//        }
//        case _OASnapshotInterface::DrawType_Rectangle:{
//            OADrawArrow * cmd = dynamic_cast<OADrawRectangle*>(drawcmd);
//            if(m_pressCtrlKey == Qt::Key_Shift){
//                dstPoint = QPoint(dstpos->pos().x()-selectedArea->leftTop().x(),cmd->start().y());
//            }else if(m_pressCtrlKey == Qt::Key_Alt)
//                dstPoint = QPoint(cmd->start().x(),dstpos->pos().y()-selectedArea->leftTop().y());

//            QRect r = cmdRect->rect();
//            r.setWidth(dstpos->pos().x()-selectedArea->x1()-r.x());
//            r.setHeight(dstpos->pos().y()-selectedArea->y1()-r.y());
//            cmdRect->setRect(r);
//            _drawCommandStack->push(cmdRect);
//            break;
//        }
    }
    return dstPoint;
}

_OASnapshotInterface::ActionType FormSnapshot::currentAction()
{
    return _action;
}

void FormSnapshot::setCurrentAction(_OASnapshotInterface::ActionType action)
{
    _action=action;
}

_OASnapshotInterface::ActionType FormSnapshot::nextAction()
{
    return _nextAction;
}

void FormSnapshot::setNextAction(_OASnapshotInterface::ActionType nextAction)
{
    _nextAction = nextAction;
}

_OASnapshotInterface::DrawType FormSnapshot::drawType()
{
    return _drawType;
}

OASnapDefine::ResizeDirection FormSnapshot::resizeDirection()
{
    return _resizeDirection;
}

void FormSnapshot::setDrawType(_OASnapshotInterface::DrawType type)
{
    _drawType = type;
}

void FormSnapshot::setResizeDirection(OASnapDefine::ResizeDirection resizeDirection)
{
    _resizeDirection = resizeDirection;
}

void FormSnapshot::capture(QWidget *target)
{
//    setVisible(true);
//    raise();
//    activateWindow();
    this->target = target;    

    //create blur background image
    QPixmap pix(selectedArea->maxWidth(),selectedArea->maxHeight());
    pix.fill((QColor(0,0,0,128)));
    bgScreen = new QPixmap(*fullScreen);
    //bgScreen = new QPixmap(QPixmap::fromImage(changeBrightness(fullScreen->toImage(),10)));
    QPainter p(bgScreen);
    p.drawPixmap(0,0,pix);

    //draw snapshot information
    QPainter infoP(infoPix);
    infoP.setBrush(QBrush(QColor(Qt::black),Qt::SolidPattern));
    infoP.drawRect(0,0,WIDTH_SHOW,HEIGHT_SHOW);

    showFullScreen();
}
