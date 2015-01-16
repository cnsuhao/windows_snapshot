#ifndef OADRAWCOMMAND_H
#define OADRAWCOMMAND_H

//#include "../pch.h"
#include <QObject>
#include <QPen>
#include <QRect>
#include <QPoint>
#include <QBrush>
#include <QFont>

class OADrawCommand : public QObject
{
    Q_OBJECT
public:
    explicit OADrawCommand(QObject *parent = 0) :
        QObject(parent)
    {
    }
    QPen pen()
    {
        return _pen;
    }
    void setPen(QPen pen)
    {
        _pen = pen;
    }
    QBrush brush()
    {
        return _brush;
    }
    void setBrush(QBrush brush)
    {
        _brush = brush;
    }
private:
    QPen _pen;
    QBrush _brush;
signals:

public slots:

};

class OADrawRectangle:public OADrawCommand
{
    Q_OBJECT
public:
    explicit OADrawRectangle(OADrawCommand *parent = 0) :
        OADrawCommand(parent)
    {

    }
    QRect rect()
    {
        return _rect;
    }
    void setRect(QRect rect)
    {
        _rect = rect;
    }
private:
    QRect _rect;
};

class OADrawEllipse:public OADrawCommand
{
    Q_OBJECT
public:
    explicit OADrawEllipse(OADrawCommand *parent = 0) :
        OADrawCommand(parent)
    {

    }
    QRect rect()
    {
        return _rect;
    }
    void setRect(QRect rect)
    {
        _rect = rect;
    }
private:
    QRect _rect;
};


class OADrawBrush:public OADrawCommand
{
    Q_OBJECT
public:
    explicit OADrawBrush(OADrawCommand *parent = 0) :
        OADrawCommand(parent)
    {

    }
    QList<QPoint> &pointList()
    {
        return _pointList;
    }
    void setPointList(QList<QPoint> pointList)
    {
        _pointList = pointList;
    }
    QPoint start()
    {
        return _start;
    }
    void setStart(QPoint start)
    {
        _start = start;
    }
    QPoint end()
    {
        return _end;
    }
    void setEnd(QPoint end)
    {
        _end = end;
    }
private:
    QPoint _start;
    QPoint _end;
    QList<QPoint> _pointList;
};

class OADrawArrow:public OADrawCommand
{
    Q_OBJECT
public:
    explicit OADrawArrow(OADrawCommand *parent = 0) :
        OADrawCommand(parent)
    {

    }
    QPoint start()
    {
        return _start;
    }
    void setStart(QPoint start)
    {
        _start = start;
    }
    QPoint end()
    {
        return _end;
    }
    void setEnd(QPoint end)
    {
        _end = end;
    }
private:
    QPoint _start;
    QPoint _end;
};

class OADrawText:public OADrawCommand
{
    Q_OBJECT
public:
    explicit OADrawText(OADrawCommand *parent = 0) :
        OADrawCommand(parent)
    {

    }
    QRect rect()
    {
        return _rect;
    }
    void setRect(QRect rect)
    {
        _rect = rect;
    }
    QString text()
    {
        return _text;
    }
    QFont font()
    {
        return _font;
    }

    void setText(QString text)
    {
        _text = text;
    }
    void setFont(QFont font){
        _font = font;
    }

private:
    QRect _rect;
    QString _text;
    QFont _font;
};

#endif // OADRAWCOMMAND_H
