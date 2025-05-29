//
// Created by bwll1 on 2024/5/27.
//

#include "BaseTimeLineWidget.h"

BaseTimelineWidget::BaseTimelineWidget(BaseTimelineModel* model, QWidget *parent) : QWidget(parent), model(model) {
    // 首先创建模型
    // model = new BaseTimelineModel();
    // 创建组件
    createComponents();
    // 连接轨道列表竖向滚动到时间线竖向滚动
    connect(tracklist, &BaseTracklistView::trackScrolled, view, &BaseTimelineView::onScroll);
    // 连接模型轨道变化到时间线更新视图
//    connect(model, &BaseTimelineModel::S_trackCountChanged, view, &BaseTimelineView::onUpdateViewport);
    // 连接轨道列表更新到时间线更新视图
    connect(tracklist, &BaseTracklistView::viewupdate, view, &BaseTimelineView::onUpdateViewport);
    // 连接工具栏设置按钮到显示设置对话框

    connect(view->toolbar, &BaseTimelineToolbar::settingsClicked, this, &BaseTimelineWidget::showSettingsDialog);
}

BaseTimelineWidget::~BaseTimelineWidget() =default;

void BaseTimelineWidget::showSettingsDialog()
{

}

QJsonObject BaseTimelineWidget::save() {
    return model->save();
}

void BaseTimelineWidget::load(const QJsonObject& json) {
    model->load(json);
}

void BaseTimelineWidget::createComponents() {
    // 创建工具栏
    view = new BaseTimelineView(model, this);
    tracklist = new BaseTracklistView(model, this);
    
    // 创建主布局
    mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);
    
    // // 创建水平分割器
    // auto* horizontalSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建左侧面板（轨道列表和时间线）
    auto* mainwidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(mainwidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    splitter->addWidget(tracklist);
    splitter->addWidget(view);
    splitter->setHandleWidth(0);
    QList<int> sizes({200,900});
    splitter->setMouseTracking(true);
    splitter->setSizes(sizes);
    mainLayout->addWidget(splitter);
    
    // 添加到水平分割器
    // horizontalSplitter->addWidget(leftPanel);
    
    // 设置分割器大小
    // horizontalSplitter->setSizes({700, 300});  // 左侧面板和属性面板的初始大小比例
    
    // 添加到主布局
    mainlayout->addWidget(mainwidget);

    // 连接模型的帧图像更新信号到舞台
//    connect(model, &BaseTimelineModel::S_frameImageUpdated,
//            model->getStage(), &TimelineStage::updateCurrentFrame);
//
//    // 注册图像提供者
//    auto engine = qmlEngine(model->getStage());
//    if (engine) {
//        engine->addImageProvider(QLatin1String("timeline"), TimelineImageProducer::instance());
//    }
}

