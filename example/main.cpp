#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString QT_MESSAGE_PATTERN="[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}";
    qSetMessagePattern(QT_MESSAGE_PATTERN);

    Widget w;
    w.show();

    return a.exec();
}
