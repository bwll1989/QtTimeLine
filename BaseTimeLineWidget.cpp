//
// Created by bwll1 on 2024/5/27.
//

#include "BaseTimeLineWidget.h"

#include <QScrollBar>

namespace {

class TimelineBindingsController final : public QObject {
public:
    TimelineBindingsController(BaseTimeLineModel* model,
                              BaseTimelineView* view,
                              BaseTracklistView* tracklist,
                              QItemSelectionModel* selection,
                              QObject* parent)
        : QObject(parent)
        , m_model(model)
        , m_view(view)
        , m_tracklist(tracklist)
        , m_selection(selection)
    {
        bindModel();
        bindSelection();
        bindScroll();
    }

private:
    void bindModel()
    {
        /**
         * @brief 统一管理“模型变化 -> 视图刷新”的绑定
         * @details
         * 这里要避免一个常见性能坑：
         * - TrackListView::onUpdateViewport() 内会确保 persistent editor 存在（可能遍历所有轨道），
         *   如果把它绑定到 dataChanged 或播放头移动，会导致高频全量遍历。
         *
         * 因此刷新策略分层：
         * - 轨道结构变化（reset/insert/remove/move）：Timeline + TrackList 全量刷新。
         * - 轨道数据变化（改名等）：TrackList 轻量重绘（必要时由 delegate setEditorData 刷新文本）。
         * - 剪辑几何/长度变化：只刷新 Timeline。
         * - 播放头变化：Timeline 刷新 + TrackList 仅重绘标题区域。
         */
        if (!m_model) {
            return;
        }

        auto refreshTimeline = [this]() {
            if (m_view) {
                m_view->onUpdateViewport();
            }
        };

        auto refreshTracklistFull = [this]() {
            if (m_tracklist) {
                m_tracklist->onUpdateViewport();
            }
        };

        auto refreshBothFull = [this, refreshTimeline, refreshTracklistFull]() {
            refreshTimeline();
            refreshTracklistFull();
        };

        // 1) 轨道结构变化：全量刷新；剪辑层变化：只刷新 Timeline
        connect(m_model, &QAbstractItemModel::modelReset, this, refreshBothFull);

        connect(m_model, &QAbstractItemModel::rowsInserted,
                this, [this, refreshTimeline, refreshTracklistFull](const QModelIndex& parent, int, int) {
                    if (parent.isValid()) {
                        refreshTimeline();
                        return;
                    }
                    refreshTimeline();
                    refreshTracklistFull();
                });

        connect(m_model, &QAbstractItemModel::rowsRemoved,
                this, [this, refreshTimeline, refreshTracklistFull](const QModelIndex& parent, int, int) {
                    if (parent.isValid()) {
                        refreshTimeline();
                        return;
                    }
                    refreshTimeline();
                    refreshTracklistFull();
                });

        connect(m_model, &QAbstractItemModel::rowsMoved,
                this, [this, refreshTimeline, refreshTracklistFull](const QModelIndex& sourceParent,
                                                                   int,
                                                                   int,
                                                                   const QModelIndex& destinationParent,
                                                                   int) {
                    if (sourceParent.isValid() || destinationParent.isValid()) {
                        refreshTimeline();
                        return;
                    }
                    refreshTimeline();
                    refreshTracklistFull();
                });

        // 2) 数据变化：按需刷新（避免 TrackList 全量）
        connect(m_model, &QAbstractItemModel::dataChanged,
                this, [this, refreshTimeline](const QModelIndex& topLeft,
                                             const QModelIndex& bottomRight,
                                             const QList<int>& roles) {
                    Q_UNUSED(bottomRight);
                    refreshTimeline();

                    if (!m_tracklist) {
                        return;
                    }

                    // Clip 层的数据变化不影响 TrackList 的显示
                    if (topLeft.parent().isValid()) {
                        return;
                    }

                    // Track 层数据变化（名称/类型/tooltip 等）：轻量重绘即可
                    const bool rolesUnknown = roles.isEmpty();
                    const bool affectsTrackName = rolesUnknown || roles.contains(QtTimeline::TrackNameRole) || roles.contains(QtTimeline::TrackTypeRole);
                    if (affectsTrackName) {
                        m_tracklist->viewport()->update();
                    }
                });

        // 3) 聚合信号：不适合用 dataChanged 表达，保留
        connect(m_model, &BaseTimeLineModel::S_LengthChanged,       this, refreshTimeline);

        connect(m_model, &BaseTimeLineModel::S_playheadMoved,
                this, [this, refreshTimeline](int) {
                    refreshTimeline();
                    if (m_tracklist) {
                        m_tracklist->viewport()->update();
                    }
                });
    }

    void bindSelection()
    {
        if (!m_selection) {
            return;
        }

        connect(m_selection, &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex&, const QModelIndex&) {
                    if (m_tracklist) {
                        m_tracklist->viewport()->update();
                    }
                });
    }

    void bindScroll()
    {
        if (!m_view || !m_tracklist) {
            return;
        }

        connect(m_tracklist->verticalScrollBar(), &QScrollBar::valueChanged,
                this, [this](int value) {
                    if (!m_view || m_isSyncingScroll) return;
                    if (m_view->verticalScrollBar()->value() == value) return;

                    m_isSyncingScroll = true;
                    m_view->verticalScrollBar()->setValue(value);
                    m_isSyncingScroll = false;
                });

        connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged,
                this, [this](int value) {
                    if (!m_tracklist || m_isSyncingScroll) return;
                    if (m_tracklist->verticalScrollBar()->value() == value) return;

                    m_isSyncingScroll = true;
                    m_tracklist->verticalScrollBar()->setValue(value);
                    m_isSyncingScroll = false;
                });
    }

    BaseTimeLineModel* m_model {nullptr};
    BaseTimelineView* m_view {nullptr};
    BaseTracklistView* m_tracklist {nullptr};
    QItemSelectionModel* m_selection {nullptr};
    bool m_isSyncingScroll {false};
};

} // namespace

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
    /**
     * @brief 创建并组装 Timeline 组件
     * @details
     * 本次重构的目标是“降低绑定复杂度、收敛状态来源”，主要做两件事：
     * 1) 选择状态：TrackListView 与 TimelineView 共享同一个 QItemSelectionModel，避免两套选择状态互相打架。
     * 2) 滚动同步：用 scrollBar 的 value 同步代替自定义 dx/dy 信号同步（trackScrolled/onScroll），减少误差与耦合。
     */

    // 1) 创建视图组件（仅负责渲染与抛出交互，不再各自维护一套选择状态）
    view = new BaseTimelineView(model, this);
    tracklist = new BaseTracklistView(model, this);
    view->setProperty("qtimeline_managed", true);
    tracklist->setProperty("qtimeline_managed", true);

    // 2) 创建共享选择模型：两个视图必须共享同一个 selectionModel
    //    - Timeline 选中 clip 时，TrackList 需要能感知当前所在 track
    //    - TrackList 选中 track 时，Timeline 也需要获得一致的 currentIndex
    m_sharedSelectionModel = new QItemSelectionModel(model, this);
    tracklist->setSelectionModel(m_sharedSelectionModel);
    view->setSelectionModel(m_sharedSelectionModel);

    if (m_bindings) {
        m_bindings->deleteLater();
        m_bindings = nullptr;
    }

    // 3) 工具栏
    toolbar = new DefaultTimeLineToolBar(view);
    view->initToolBar(toolbar);

    // 4) 创建水平分割器
    splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(tracklist);
    splitter->addWidget(view);
    splitter->setHandleWidth(2);
    splitter->setStyleSheet("QSplitter::handle { background-color: #4c5560; border-left: 1px solid #2a2f34; border-right: 1px solid #2a2f34; } QSplitter::handle:hover { background-color: #5a6470; } QSplitter::handle:pressed { background-color: #6b7684; }");
    splitter->setMouseTracking(true);
    splitter->setSizes({200, 900});

    // 5) 主布局（保持 0 margin/spacing，避免额外间隙导致绘制不对齐）
    mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);

    auto* mainWidget = new QWidget(this);
    auto* mainWidgetLayout = new QVBoxLayout(mainWidget);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);
    mainWidgetLayout->setSpacing(0);
    mainWidgetLayout->addWidget(splitter);
    mainlayout->addWidget(mainWidget);

    // 6) 绑定控制器：集中管理 connect，降低 Widget 内绑定复杂度
    m_bindings = new TimelineBindingsController(model, view, tracklist, m_sharedSelectionModel, this);

    // 7) 说明：此处不再连接 trackScrolled/viewUpdate 等“视图到视图”的信号。
    //    视图之间的联动被收敛为：共享 selectionModel + 同步 scrollBar value。
}

