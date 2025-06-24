#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 创建并加载插件
    QFile qssFile(":/icons/icons/DefaultDark.qss");
    if(qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&qssFile);
        qApp->setStyleSheet(stream.readAll()); // 使用qApp全局设置
        qssFile.close();
    } else {
        qWarning() << "Failed to load QSS file:" << qssFile.errorString();
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}


