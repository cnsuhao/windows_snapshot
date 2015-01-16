#ifndef OASNAPDEFINE_H
#define OASNAPDEFINE_H
#include <QObject>
#include <QList>

class _OASnapshotInterface;
class OAColorInterface;
class OASnapDefine : public QObject
{
    Q_OBJECT

public:
    enum ResizeDirection{
        ResizeDirection_NONE    = 0,
        ResizeDirection_UP      = 1,
        ResizeDirection_DOWN    = 2,
        ResizeDirection_LEFT,
        ResizeDirection_RIGHT,
        ResizeDirection_LEFTTOP,
        ResizeDirection_LEFTBOTTOM,
        ResizeDirection_RIGHTBOTTOM,
        ResizeDirection_RIGHTTOP
    };

public:
    explicit OASnapDefine(QObject *parent = 0);
    ~OASnapDefine();

    QList<_OASnapshotInterface> snapshotDrawSizeTypes();
    QList<_OASnapshotInterface> snapshotActionList();
    QList<OAColorInterface> oaColorList();

private:
    QList<_OASnapshotInterface>  _snapshotDrawSizeTypes;
    QList<_OASnapshotInterface>  _snapshotActionList;
    QList<OAColorInterface> _oaColorList;
};

class _OASnapshotInterface:public QObject
{
    Q_OBJECT
public:

    enum ActionType{
        ActionType_None     = 0,
        ActionType_Select      = 1,
        ActionType_Drag        = 2,
        ActionType_Draw        = 3,
        ActionType_Resize      = 4
    };
    enum DrawType{
        DrawType_None       = 0,
        DrawType_Rectangle  = 1,
        DrawType_Ellipse    = 2,
        DrawType_Arrow      = 3,
        DrawType_Brush      = 4,
        DrawType_Filter     = 5,
        DrawType_Text       = 6,
        DrawType_Undo    = 101,
        DrawType_Save    = 102,
        DrawType_Exit    = 103,
        DrawType_Finish  = 104
    };
    enum Draw_SizeType{
        Draw_SizeType_None       = 0,
        Draw_SizeType_Small =1,
        Draw_SizeType_Middle=2,
        Draw_SizeType_Huge=3
    };

    explicit _OASnapshotInterface(QString name,QString text,DrawType value,QString shortcut=QString(),bool isCheckable=false,QObject *parent =0):QObject(parent)
    {
        this->name = name;
        this->text = text;
        this->value = value;
        this->shortcut = shortcut;
        this->isCheckable = isCheckable;
    }

    explicit _OASnapshotInterface(QString name,Draw_SizeType value,QString text = "",bool isCheckable=false,QObject *parent =0):QObject(parent)
    {
        this->Draw_ST_name = name;
        this->Draw_ST_value = value;
        this->text = text;//tooltip
        this->isCheckable = isCheckable;
    }

    QString Draw_ST_name;
    Draw_SizeType Draw_ST_value;

    QString name,text;
    DrawType value;
    QString shortcut;
    bool isCheckable;

    _OASnapshotInterface(const _OASnapshotInterface &that){
        copy(that);
    }

    _OASnapshotInterface &operator=(const _OASnapshotInterface & that){
        copy(that);

        return *this;
    }
private:
    void copy(const _OASnapshotInterface &that){
        this->name = that.name;
        this->text = that.text;
        this->value = that.value;
        this->shortcut = that.shortcut;
        this->isCheckable = that.isCheckable;
        this->Draw_ST_name = that.Draw_ST_name;
        this->Draw_ST_value = that.Draw_ST_value;
    }
};
class OAColorInterface:public QObject{
    Q_OBJECT
public:
    enum ColorType{
        OAColor_black = 2,
        OAColor_darkgray = 3,
        OAColor_darkred,
        OAColor_darkyellow,
        OAColor_darkgreen,
        OAColor_darkblue,
        OAColor_darkpin,
        OAColor_darkcyan,
        OAColor_white,
        OAColor_gray,
        OAColor_red,
        OAColor_yellow,
        OAColor_green,
        OAColor_blue,
        OAColor_pin,
        OAColor_cyan    =   17,
        OAColor_none    =   0
    };

    explicit OAColorInterface(QString name,QString description,ColorType type){
        this->name = name;
        this->description = description;
        this->type = type;
    }
    static QString OAColorName(ColorType type){
        QString name;
        OASnapDefine oadef;
        foreach(OAColorInterface oacolor,oadef.oaColorList()){
            if(oacolor.type == type && oacolor.type != OAColorInterface::OAColor_none){
                name = oacolor.name;
                break;
            }
        }
        return name;
    }

    OAColorInterface(const OAColorInterface &that){
        copy(that);
    }
    OAColorInterface &operator=(const OAColorInterface & that){
        copy(that);
        return *this;
    }
    QString name;/*color name #000000*/
    QString description;
    ColorType type;
private:
    void copy(const OAColorInterface &that){
        this->name = that.name;
        this->description = that.description;
        this->type = that.type;
    }
};
#endif // OASNAPDEFINE_H
