#ifndef OASCREEN_H
#define OASCREEN_H

//#include "../pch.h"
//#include "../oadefine.h"
#include <QObject>
#include <QRect>
#include <QPoint>

class OARect : public QObject
{
    Q_OBJECT
public:
    OARect(QSize size)
    {
        _maxWidth = size.width();
        _maxHeight = size.height();

        //init:
        startPoint = QPoint(-1,-1);
        endPoint = startPoint;
        _leftTop = startPoint;
        _rightBottom = startPoint;
    }

    void setStartPosition(QPoint pos)
    {
        startPoint=pos;
    }
    void setEndPosition(QPoint pos)
    {
        endPoint = pos;
        //invoke method cmpPoint, and setup left-top and right-bottom point
        _leftTop = startPoint;
        _rightBottom = endPoint;
        adjustToRect(_leftTop,_rightBottom);
    }
    QPoint startPosition()
    {
        return startPoint;
    }
    QPoint endPosition()
    {
        return endPoint;
    }

    QPoint leftTop()
    {
        return _leftTop;
    }
    QPoint rightBottom()
    {
        return _rightBottom;
    }


    int x1()
    {
        return _leftTop.x();
    }
    int y1()
    {
        return _leftTop.y();
    }
    int x2()
    {
        return _rightBottom.x();
    }
    int y2()
    {
        return _rightBottom.y();
    }
    int width()
    {
        return rightBottom().x()-leftTop().x();
    }
    int height()
    {
        return rightBottom().y()-leftTop().y();
    }
    int maxWidth()
    {
        return _maxWidth;
    }
    int maxHeight()
    {
        return _maxHeight;
    }
    bool contains(QPoint pos)
    {
        if( pos.x() > _leftTop.x() && pos.x() < _rightBottom.x() &&\
                pos.y() > _leftTop.y() && pos.y() < _rightBottom.y() )
            return true;
        return false;
    } //check pos is in snapshot area.
    void move(QPoint p)
    {
        int lx = _leftTop.x() + p.x();
        int ly = _leftTop.y() + p.y();
        int rx = _rightBottom.x() + p.x();
        int ry = _rightBottom.y() + p.y();

        if( lx<0 ) { lx = 0; rx -= p.x(); }  //no more moving when it is end of left
        if( ly<0 ) { ly = 0; ry -= p.y(); }  //end of top
        if( rx>_maxWidth )  { rx = _maxWidth; lx -= p.x(); }  //end of right
        if( ry>_maxHeight ) { ry = _maxHeight; ly -= p.y(); } //end of bottom

        _leftTop = QPoint(lx,ly);
        _rightBottom = QPoint(rx,ry);
        startPoint = _leftTop; //reset start point
        endPoint = _rightBottom;
    }       //move snapshot area by p.
    void resize(int x1,int y1,int x2,int y2)
    {
        _leftTop = QPoint(x1,y1);
        _rightBottom = QPoint(x2,y2);
        adjustToRect(_leftTop,_rightBottom);
        startPoint = _leftTop; //reset start point
        endPoint = _rightBottom;
    }
    QRect rect()
    {
        return QRect(x1(),y1(),width(),height());
    }

signals:

public slots:

private:
    QPoint _leftTop, _rightBottom;//snapshot area left-top and right-bottom position
    QPoint startPoint, endPoint;//mouse begin and end position
    int _maxWidth, _maxHeight;  //screen width and height

    //compare two point ,calc out left-top and right-bottom point,
    //and set them to QPoint &s and QPoint &e
    //compare two point,detect left-top and right-bottom point
    void adjustToRect(QPoint &leftTop, QPoint &rightBottom)
    {
        int x1 = leftTop.x(),y1=leftTop.y(),x2=rightBottom.x(),y2=rightBottom.y();

        if( x1<=x2  )  //start point should be in left
        {
            if( y1<=y2 ) //left top
            {
                /*if(x1==x2 && y1==y2){
                rightBottom.setX(x2+1);
                rightBottom.setY(y2+1);
            }else if(x1==x2){
                rightBottom.setX(x2+1);
            }else if(y1==y2){
                rightBottom.setY(y2+1);
            }*/
                ;
            }
            else              //left bottom
            {
                leftTop.setY(y2);
                rightBottom.setY(y1);
            }
        }
        else                //right
        {
            if( y1<y2 ) //right top
            {
                leftTop.setX(x2);
                rightBottom.setX(x1);
            }
            else             //right bottom
            {
                QPoint tmp;
                tmp = leftTop;
                leftTop = rightBottom;
                rightBottom = tmp;
            }
        }
    }


};

#endif // OASCREEN_H
