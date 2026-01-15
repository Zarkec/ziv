#include "ui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    a.setStyle("Fusion");
    
    MainWindow w;
    w.show();
    
    if (argc > 1) {
        QString fileName = QString::fromLocal8Bit(argv[1]);
        w.openFile(fileName);
    }
    
    return a.exec();
}
