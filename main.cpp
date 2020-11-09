#include "pyrosqt.h"

#include <QApplication>
#include <QPalette>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    std::setlocale(LC_NUMERIC, "C");

    QCoreApplication::setOrganizationName("PyrosQT");
    QCoreApplication::setApplicationName("Pyros");

    PyrosQT w;
    w.show();
    return a.exec();
}
