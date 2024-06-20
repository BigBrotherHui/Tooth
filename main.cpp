#include "mainwindow.h"

#include <QApplication>
#include <QmitkRegisterClasses.h>
#include <vtkOutputWindow.h>

int main(int argc, char *argv[])
{
    QmitkRegisterClasses();
    vtkOutputWindow::GlobalWarningDisplayOff();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
