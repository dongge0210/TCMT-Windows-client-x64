#include "stdafx.h"  // ������ǰ��
#include <QtWidgets/QApplication>
#include "QtWidgetsTCmonitor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtWidgetsTCmonitor w;
    w.show();
    return a.exec();
}
