#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序样式为Fusion
    a.setStyle("Fusion");
    
    MainWindow w;
    w.show();
    return a.exec();
}
