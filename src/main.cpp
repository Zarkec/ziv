#include "ui/mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    QTranslator qtTranslator;
    QString locale = QLocale::system().name();
    if (qtTranslator.load(QLocale::system(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(&qtTranslator);
    }
    
    a.setStyle("Fusion");
    
    MainWindow w;
    w.show();
    
    if (argc > 1) {
        QString fileName = QString::fromLocal8Bit(argv[1]);
        w.openFile(fileName);
    }
    
    return a.exec();
}
