#include "mainwindow.h"
#include <QApplication>
#include "DarkStyle.h"
#include "framelesswindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(new DarkStyle);


//    MainWindow w;
//    w.show();

    FramelessWindow framelessWindow;
    framelessWindow.setWindowState(Qt::WindowFullScreen);
    framelessWindow.setWindowTitle("AImageLab Bridge by Prof. Vezzani");
    framelessWindow.setWindowIcon(a.style()->standardIcon(QStyle::SP_DesktopIcon));

    // create our mainwindow instance
    MainWindow *mainWindow = new MainWindow;

    // add the mainwindow to our custom frameless window
    framelessWindow.setContent(mainWindow);
    framelessWindow.show();


    return a.exec();
}
