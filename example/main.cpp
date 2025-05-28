#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 创建并加载插件
   
    
    MainWindow w;
    w.show();
    return a.exec();
}


