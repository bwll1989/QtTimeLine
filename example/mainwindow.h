#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "BaseTimeLineWidget.h"
#include "BaseTimeLineModel.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onVideoPlayerActionToggled(bool checked);  // 添加视频播放器切换槽
    void onSaveScene();
    void onLoadScene();
private:
    Ui::MainWindow *ui;
    BaseTimelineModel* model;
    BaseTimelineWidget *timeline;
};

#endif // MAINWINDOW_H
