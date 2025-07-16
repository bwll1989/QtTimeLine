//
// Created by bwll1 on 2024/5/27.
//

#include "BaseTimeLineWidget.h"

BaseTimelineWidget::BaseTimelineWidget(BaseTimeLineModel* model, QWidget *parent) : QWidget(parent), model(model) {
    // 首先创建模型
    // model = new BaseTimelineModel();
    // 创建组件
    createComponents();
   
}

BaseTimelineWidget::~BaseTimelineWidget() =default;

void BaseTimelineWidget::showSettingsDialog()
{
    qDebug()<<"showSettingsDialog";
}

QJsonObject BaseTimelineWidget::save() {
    return model->save();
}

void BaseTimelineWidget::load(const QJsonObject& json) {
    model->load(json);
}

void BaseTimelineWidget::createComponents() {
    // 创建视图组件
    view = new BaseTimelineView(model, this);
    tracklist = new BaseTracklistView(model, this);
    
    // 创建水平分割器
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(tracklist);
    splitter->addWidget(view);
    splitter->setHandleWidth(0);
    splitter->setMouseTracking(true);
    splitter->setSizes({200, 900});
    
    // 创建主布局
    mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);
    
    // 创建主容器并设置布局
    auto* mainWidget = new QWidget(this);
    auto* mainWidgetLayout = new QVBoxLayout(mainWidget);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);
    mainWidgetLayout->setSpacing(0);
    mainWidgetLayout->addWidget(splitter);
    
    // 添加到主布局
    mainlayout->addWidget(mainWidget);
    
    // 连接信号和槽
    connect(tracklist, &BaseTracklistView::trackScrolled, view, &BaseTimelineView::onScroll);
    // 连接模型轨道变化到时间线更新视图
//    connect(model, &BaseTimelineModel::S_trackCountChanged, view, &BaseTimelineView::onUpdateViewport);
    // 连接轨道列表更新到时间线更新视图
    connect(tracklist, &BaseTracklistView::viewUpdate, view, &BaseTimelineView::onUpdateViewport);
    // 连接工具栏设置按钮到显示设置对话框

    connect(view->m_toolbar, &BaseTimelineToolbar::settingsClicked, this, &BaseTimelineWidget::showSettingsDialog);

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

