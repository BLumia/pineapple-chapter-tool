#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QStringLiteral("Pineapple Chapter Tool"));
    a.setApplicationDisplayName(QCoreApplication::translate("main", "Pineapple Chapter Tool"));

    MainWindow w;
    w.show();
    return a.exec();
}
