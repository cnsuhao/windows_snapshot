#include "oasnapdefine.h"

OASnapDefine::OASnapDefine(QObject *parent) : QObject(parent)
{
    _snapshotActionList << _OASnapshotInterface("select",tr("select/move"), _OASnapshotInterface::DrawType_None,"Ctrl+M",true)
                        << _OASnapshotInterface("rectangle",tr("rectangle"),_OASnapshotInterface::DrawType_Rectangle,"Ctrl+R",true)
                        << _OASnapshotInterface("circle",tr("circle"),_OASnapshotInterface::DrawType_Ellipse,"Ctrl+C",true)
                        << _OASnapshotInterface("arrow",tr("arrow"),_OASnapshotInterface::DrawType_Arrow,"Ctrl+A",true)
                        << _OASnapshotInterface("brush",tr("brush"),_OASnapshotInterface::DrawType_Brush,"Ctrl+B",true)
                        << _OASnapshotInterface("text",tr("text"),_OASnapshotInterface::DrawType_Text,"Ctrl+T",true)
                        << _OASnapshotInterface("|",tr("|"),_OASnapshotInterface::DrawType_None)
                        << _OASnapshotInterface("undo",tr("undo"),_OASnapshotInterface::DrawType_Undo,"Ctrl+Z")
                        << _OASnapshotInterface("save",tr("save"),_OASnapshotInterface::DrawType_Save,"Ctrl+S")
                        << _OASnapshotInterface("exit",tr("exit"),_OASnapshotInterface::DrawType_Exit,"Esc")
                        << _OASnapshotInterface("finish",tr("finish"),_OASnapshotInterface::DrawType_Finish,"Enter");

    _snapshotDrawSizeTypes<<_OASnapshotInterface("small",_OASnapshotInterface::Draw_SizeType_Small,"1")
                         << _OASnapshotInterface("middle",_OASnapshotInterface::Draw_SizeType_Middle,"3")
                         << _OASnapshotInterface("huge",_OASnapshotInterface::Draw_SizeType_Huge,"7");

    _oaColorList<<OAColorInterface("#000000","black",OAColorInterface::OAColor_black)
               <<OAColorInterface("#808080","darkgray",OAColorInterface::OAColor_darkgray)
              <<OAColorInterface("#800000","darkred",OAColorInterface::OAColor_darkred)
             <<OAColorInterface("#f7883a","darkyellow",OAColorInterface::OAColor_darkyellow)
            <<OAColorInterface("#308430","darkgreen",OAColorInterface::OAColor_darkgreen)
           <<OAColorInterface("#385ad3","darkblue",OAColorInterface::OAColor_darkblue)
          <<OAColorInterface("#800080","darkpin",OAColorInterface::OAColor_darkpin)
         <<OAColorInterface("#009999","darkcyan",OAColorInterface::OAColor_darkcyan)
        <<OAColorInterface("#ffffff","white",OAColorInterface::OAColor_white)
       <<OAColorInterface("#c0c0c0","gray",OAColorInterface::OAColor_gray)
      <<OAColorInterface("#fb3838","red",OAColorInterface::OAColor_red)
     <<OAColorInterface("#ffff00","yellow",OAColorInterface::OAColor_yellow)
    <<OAColorInterface("#99cc00","green",OAColorInterface::OAColor_green)
    <<OAColorInterface("#3894e4","blue",OAColorInterface::OAColor_blue)
    <<OAColorInterface("#f31bf3","pin",OAColorInterface::OAColor_pin)
    <<OAColorInterface("#16dcdc","none",OAColorInterface::OAColor_cyan);
}

OASnapDefine::~OASnapDefine()
{

}

QList<_OASnapshotInterface> OASnapDefine::snapshotActionList()
{
    return _snapshotActionList;
}

QList<_OASnapshotInterface> OASnapDefine::snapshotDrawSizeTypes()
{
    return _snapshotDrawSizeTypes;
}

QList<OAColorInterface> OASnapDefine::oaColorList()
{
    return _oaColorList;
}

