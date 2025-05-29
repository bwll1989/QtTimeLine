#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "BaseTrackListView.h"
#include "BaseTimeLineWidget.h"
#include "BaseTimeLineModel.h"
#include "BasePluginLoader.h"
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new BaseTimeLineModel();
    //给模型初始化插件加载器
    model->setPluginLoader(new BasePluginLoader());

    timeline = new BaseTimelineWidget(model);
    ui->verticalLayout->addWidget(timeline);
    connect(ui->actionsave_2, &QAction::triggered, this, &MainWindow::onSaveScene);
    connect(ui->actionload, &QAction::triggered, this,&MainWindow::onLoadScene);
    // // 连接开始动作
//    connect(ui->actionstart, &QAction::triggered, timeline, &BaseTimelineWidget::start);
    ui->actionstart->setShortcut(QKeySequence(Qt::Key_Space));

    // // 连接视频播放器显示切换动作
    connect(ui->actionShowVideoPlayer, &QAction::toggled,
            this, &MainWindow::onVideoPlayerActionToggled);

    // // 当视频窗口被关闭时，取消选中菜单项
    // connect(timeline, &BaseTimelineWidget::videoWindowClosed,
    //         [this]() {
    //             ui->actionShowVideoPlayer->setChecked(false);
    //         });
}

void MainWindow::onLoadScene()
{
    // 打开文件加载对话框
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("加载时间线"),
        QDir::homePath(),
        tr("时间线文件 (*.json)"));
    if (fileName.isEmpty()) {
        return;
    }
    // 打开文件
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("加载失败"),
            tr("无法打开文件 %1 进行读取：%2")
           .arg(fileName)
           .arg(file.errorString()));
        return;
    }
    // 读取文件内容
    QByteArray data = file.readAll();
    file.close();
    // 解析 JSON 数据
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::warning(this, tr("加载失败"),
            tr("无法解析文件 %1 的 JSON 数据")
          .arg(fileName));
        return;
    }
    // 加载时间线数据
    timeline->load(doc.object());
}

MainWindow::~MainWindow()
{
    // 确保视频播放器关闭

    delete ui;
}
void MainWindow::onSaveScene()
{
    // 打开文件保存对话框
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("保存时间线"),
        QDir::homePath(),
        tr("时间线文件 (*.json)"));

    if (fileName.isEmpty()) {
        return;
    }

    // 确保文件名以 .json 结尾
    if (!fileName.endsWith(".json", Qt::CaseInsensitive)) {
        fileName += ".json";
    }

    // 获取时间线数据
    QJsonObject timelineData = timeline->save();

    // 创建 JSON 文档
    QJsonDocument doc(timelineData);

    // 打开文件
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("保存失败"),
            tr("无法打开文件 %1 进行写入：%2")
            .arg(fileName)
            .arg(file.errorString()));
        return;
    }

    // 写入 JSON 数据
    file.write(doc.toJson());
    file.close();
}

void MainWindow::onVideoPlayerActionToggled(bool checked)
{
    qDebug() << "onVideoPlayerActionToggled: " << checked;

}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 确保视频播放器关闭

    QMainWindow::closeEvent(event);
}
