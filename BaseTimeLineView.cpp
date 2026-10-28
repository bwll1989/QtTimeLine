#include "BaseTimeLineView.h"
#include <QMenu>
#include <QAction>
#include <QtMath>
#include <QTimer>
#include <algorithm>
#include "DefaultTimeLineToolBar.h"

namespace {
void clipRowsInFrameRange(const TrackData* track, int frameLo, int frameHi, QList<int>& rows)
{
    rows.clear();
    if (!track)
        return;

    const int clipCount = track->clips.size();
    if (clipCount <= 0)
        return;

    auto overlaps = [&](int row) {
        AbstractClipModel* clip = track->clips[row];
        return clip && clip->end() >= frameLo && clip->start() <= frameHi;
    };

    if (track->clipsByStart.size() != clipCount) {
        for (int row = 0; row < clipCount; ++row) {
            if (overlaps(row))
                rows.push_back(row);
        }
        return;
    }

    const int minStart = frameLo - qMax(0, track->maxClipSpan);
    const QList<int>& order = track->clipsByStart;
    const auto first = std::lower_bound(order.begin(), order.end(), minStart, [track](int row, int value) {
        const AbstractClipModel* clip = track->clips[row];
        return (clip ? clip->start() : 0) < value;
    });
    const auto last = std::upper_bound(order.begin(), order.end(), frameHi, [track](int value, int row) {
        const AbstractClipModel* clip = track->clips[row];
        return value < (clip ? clip->start() : 0);
    });
    for (auto it = first; it != last; ++it) {
        if (overlaps(*it))
            rows.push_back(*it);
    }
    std::sort(rows.begin(), rows.end());
}
}
BaseTimelineView::BaseTimelineView(BaseTimeLineModel *viewModel, QWidget *parent)
        : Model(viewModel), QAbstractItemView{parent}
    {
        setObjectName("TimelineView");
        setFrameShape(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt:: ScrollBarAlwaysOff);
        setModel(Model);
        horizontalScrollBar()->setSingleStep(10);
        horizontalScrollBar()->setPageStep(100);
        verticalScrollBar()->setSingleStep(trackHeight);
        verticalScrollBar()->setPageStep(trackHeight * 5);
        viewport()->setMinimumHeight(trackHeight + rulerHeight + toolbarHeight);
        // setMinimumHeight(trackHeight + rulerHeight + toolbarHeight);

        /**
         * @brief 选择模型说明
         * @details
         * 本视图不再在构造函数里创建 selectionModel。
         * - 由 BaseTimelineWidget 统一创建共享 QItemSelectionModel，并同时注入到 TrackListView 与 TimelineView。
         * - 这样可以保证“选择状态”只有一份，避免绑定链路复杂、状态不同步。
         */
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectItems);
        setMouseTracking(true);
        // 确保有选择模型
        setEditTriggers(QAbstractItemView::AllEditTriggers);
        setAutoScroll(true);
        setAutoScrollMargin(5);
        setAcceptDrops(true);
        // setDragEnabled(true);

    // 连接循环播放信号

        installEventFilter(this);

        // ========== 新增：初始化重绘合并定时器 ==========
        m_redrawCoalesceTimer = new QTimer(this);
        m_redrawCoalesceTimer->setSingleShot(true);
        connect(m_redrawCoalesceTimer, &QTimer::timeout, this, [this]() {
            performRedraw();
        });
        // ========== 新增结束 ==========

        // 从当前属性应用颜色，支持 QSS 的 qproperty- 语法
        applyStyleFromProperties();
    }

BaseTimelineView::~BaseTimelineView()
{
    finishClipDrag();
    delete m_toolbar;
    // delete timer;

}

void BaseTimelineView::initToolBar(BaseTimelineToolbar* new_toolbar)
{
    m_toolbar=new_toolbar;
    m_toolbar->setFixedHeight(toolbarHeight-6);
    // 设置工具栏位置
    m_toolbar->move(4, 4);

}

QRect BaseTimelineView::visualRect(const QModelIndex &index) const
{
    /**
     * @brief 获取索引在 viewport 内的可视矩形
     * @details
     * 与 TrackListView 保持一致：以滚动条 value 为唯一真值来源计算偏移，
     * 避免 m_scrollOffset 缓存状态与滚动条不同步导致命中测试/绘制错位。
     */
    const QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
    return itemRect(index).translated(-offset);
}

 QModelIndex BaseTimelineView::indexAt(const QPoint &point) const
    {
        /**
         * 函数说明：根据鼠标点返回对应的模型索引
         * - 原逻辑：优先命中轨道，再命中该轨道下的剪辑矩形（visualRect）
         * - 修改点：为避免在极小缩放下片段过窄导致无法选中，
         *   在剪辑的命中测试时应用“最小命中宽度”保护：
         *   如果 clipRect.width() < minHitWidth，则围绕其中心点临时扩展命中矩形到 minHitWidth，
         *   仅用于contains判断，不影响绘制或布局的真实宽度。
         */
        if (point.y() < contentTop() || trackHeight <= 0 || !model())
            return {};

        const int contentY = point.y() + verticalScrollBar()->value() - contentTop();
        if (contentY < 0)
            return {};
        const int row = contentY / trackHeight;
        if (row < 0 || row >= model()->rowCount())
            return {};

        const QModelIndex trackIndex = model()->index(row, 0);
        const QRect trackRect = visualRect(trackIndex);
        if (!trackRect.contains(point))
            return {};

        auto* track = static_cast<TrackData*>(trackIndex.internalPointer());
        const int minHitWidth = 15;
        const int contentX = point.x() + horizontalScrollBar()->value();
        const int frameLo = pointToFrame(qMax(0, contentX - minHitWidth)) - 1;
        const int frameHi = pointToFrame(contentX) + 1;
        QList<int> clipRows;
        clipRowsInFrameRange(track, frameLo, frameHi, clipRows);

        for (int clipRow : clipRows) {
            const QModelIndex clipIndex = model()->index(clipRow, 0, trackIndex);
            QRect clipRect = visualRect(clipIndex);
            if (clipRect.width() < minHitWidth) {
                const int pad = minHitWidth - clipRect.width();
                clipRect.adjust(0, 0, pad, 0);
                clipRect = clipRect.intersected(trackRect);
            }
            if (clipRect.contains(point))
                return clipIndex;
        }
        return trackIndex;
    }

QRect BaseTimelineView::itemRect(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QRect();
    }
    // 获取轨道宽度
    int trackwidth = getTrackWdith()+ viewport()->width();
    if(index.parent()==QModelIndex())
        // 如果轨道索引
    {
        // 返回轨道矩形
        return QRect(0, (index.row() * trackHeight) + contentTop(), trackwidth, trackHeight);
    }
     // 如果剪辑索引
    else{
        // 获取剪辑
        AbstractClipModel* clip = static_cast<AbstractClipModel*>(index.internalPointer());
        if (clip) {
            // 获取剪辑开始帧
            int startFrame = clip->start();
            // 获取剪辑结束帧
            int endFrame = clip->end()+1;
            // 获取剪辑所在轨道行
            int trackRow = index.parent().row();
            // 获取剪辑开始点x
            int clipStartX = frameToPoint(startFrame);
            // 获取剪辑开始点y
            int clipStartY = (trackRow*trackHeight + contentTop());
            // 获取剪辑宽度
            int clipWidth = frameToPoint(endFrame - startFrame);
            // 获取剪辑所在轨道行开始点
            QPoint topLeft(clipStartX,clipStartY);
            // 返回剪辑矩形
            // qDebug()<<"rect"<<QRect(topLeft,QSize(clipWidth,trackHeight));
            return QRect(topLeft,QSize(clipWidth,trackHeight));
        }

    }
    return QRect();
}

int BaseTimelineView::getTrackWdith() const
{
    int timelineLength = model()->data(QModelIndex(), TimelineRoles::TimelineLengthRole).toInt();
    if (timelineLength <= 0) {
        return viewport()->width();  // 如果没有片段，返回视口宽度
    }
    return frameToPoint(timelineLength + 1);  // 否则返回基于时间轴长度的宽度
}

// 移动选定的剪辑
void BaseTimelineView::moveSelectedClip(int dx, int dy, bool isMouse)
{
    QModelIndexList list = selectionModel()->selectedIndexes();
    if(list.isEmpty())
        return;
    int newPos;
    if(isMouse){
        newPos = dx;
    }else{
        newPos = model()->data(list[0],TimelineRoles::ClipInRole).toInt() + dx;
    }
    // 确保不会移动到负值位置
    newPos = std::max(0, newPos);
    // 更新模型数据
    // 非拖拽时 dataChanged 会局部刷新。拖拽期间模型先不发通知，由鼠标移动自己脏矩形。
    getModel()->setData(list.first(), newPos, TimelineRoles::ClipPosRole);
}

void BaseTimelineView::movePlayheadToFrame(int frame)
{
    // 直接设置时间码生成器的帧位置。S_playheadMoved 会走局部刷新。
    getModel()->onSetPlayheadPos(frame);
}

void BaseTimelineView::onUpdateViewport(){
    scheduleRedraw();
}

// 滚动视图
void BaseTimelineView::scheduleRedraw(int delayMs) {
    m_staticCacheDirty = true;
    if (!m_redrawCoalesceTimer) return;
    // 如果定时器已在等待中，无需重复启动；让多次请求自然合并
    if (!m_redrawCoalesceTimer->isActive()) {
        m_redrawCoalesceTimer->start(delayMs);
    }
}

void BaseTimelineView::performRedraw() {
    if (m_isRedrawing) return;
    m_isRedrawing = true;

    // 内容变化才更新滚动条范围。时间轴没有持久编辑器。
    updateScrollBars();
    viewport()->update();

    m_isRedrawing = false;
}

void BaseTimelineView::onPlayheadMoved(int frame)
{
    const int previous = m_paintedPlayheadFrame;
    m_paintedPlayheadFrame = frame;
    if (!staticCacheReady()) {
        viewport()->update();
        return;
    }
    if (previous >= 0 && previous != frame)
        viewport()->update(playheadDirtyRect(previous));
    viewport()->update(playheadDirtyRect(frame));
}

bool BaseTimelineView::staticCacheReady() const
{
    if (m_staticCacheDirty || m_staticCache.isNull() || !viewport())
        return false;
    const qreal dpr = viewport()->devicePixelRatio();
    const QSize px(qRound(viewport()->width() * dpr), qRound(viewport()->height() * dpr));
    return m_staticCache.size() == px;
}

QRect BaseTimelineView::playheadDirtyRect(int frame) const
{
    if (frame < 0 || !viewport())
        return {};
    const int x = frameToPoint(frame) - horizontalScrollBar()->value();
    const int pad = playheadwidth + 2;
    const int top = qMax(0, contentTop() - playheadheight);
    return QRect(x - pad, top, pad * 2 + 1, viewport()->height() - top).intersected(viewport()->rect());
}

void BaseTimelineView::ensureStaticCache()
{
    QWidget* vp = viewport();
    if (!vp || vp->width() <= 0 || vp->height() <= 0)
        return;
    if (staticCacheReady())
        return;
    if (m_interactiveDrag && Model)
        Model->syncInteractiveClipOrder();

    const qreal dpr = vp->devicePixelRatio();
    m_staticCache = QPixmap(qRound(vp->width() * dpr), qRound(vp->height() * dpr));
    m_staticCache.setDevicePixelRatio(dpr);
    m_staticCache.fill(m_timelineBgColor);

    QPainter cachePainter(&m_staticCache);
    const QRect rect(0, 0, vp->width(), vp->height());
    drawBackground(&cachePainter, rect);
    drawVerticalTimeLines(&cachePainter, rect);
    drawTracks(&cachePainter);
    drawTimeRuler(&cachePainter, rect);
    m_staticCacheDirty = false;
}

QRect BaseTimelineView::clipContentBounds(AbstractClipModel* clip, int trackRow) const
{
    if (!clip)
        return {};
    const int x = frameToPoint(clip->start());
    const int width = qMax(1, frameToPoint(clip->end() - clip->start() + 1));
    const int y = trackRow * trackHeight + contentTop();
    // 右侧多留一截，盖住画在片段矩形外面的标题。
    return QRect(x, y, width, trackHeight).adjusted(-4, -4, 44, 4);
}

void BaseTimelineView::updateClipGeometry(const QModelIndex& index)
{
    if (!index.isValid() || !index.parent().isValid() || !viewport())
        return;
    auto* clip = static_cast<AbstractClipModel*>(index.internalPointer());
    if (!clip)
        return;
    if (!staticCacheReady()) {
        scheduleRedraw();
        return;
    }

    const QRect now = clipContentBounds(clip, index.parent().row());
    const auto found = m_clipContentRects.find(clip);
    const QRect old = found == m_clipContentRects.end() ? now : found->second;
    m_clipContentRects[clip] = now;

    const QPoint scroll(horizontalScrollBar()->value(), verticalScrollBar()->value());
    QRegion dirty(old.translated(-scroll));
    dirty += now.translated(-scroll);
    dirty &= viewport()->rect();
    if (dirty.isEmpty())
        return;
    repaintStaticCache(dirty);
    viewport()->update(dirty.boundingRect());
}

void BaseTimelineView::syncScrollOffsetFromScrollBars()
{
    /**
     * @brief 从滚动条同步 m_scrollOffset
     * @details
     * 滚动条（horizontalScrollBar/verticalScrollBar）是 QAbstractItemView 的内建滚动状态。
     * 当前项目历史代码同时维护了 m_scrollOffset，因此必须在滚动发生后同步缓存，保证绘制/命中测试一致。
     */
    m_scrollOffset = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

void BaseTimelineView::setScrollOffsetClamped(const QPoint& offset)
{
    /**
     * @brief 设置滚动偏移并同步滚动条（自动夹紧）
     * @details
     * 禁止直接修改 m_scrollOffset；正确做法是设置滚动条 value，让 QAbstractItemView 走标准滚动路径。
     * 这里做夹紧，避免出现负值或超过 maximum 的情况。
     */
    const int x = qBound(horizontalScrollBar()->minimum(), offset.x(), horizontalScrollBar()->maximum());
    const int y = qBound(verticalScrollBar()->minimum(),   offset.y(), verticalScrollBar()->maximum());

    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);

    syncScrollOffsetFromScrollBars();
}

void BaseTimelineView::onScroll(int dx, int dy){
    /**
     * @brief 外部滚动同步入口（兼容旧接口）
     * @details
     * 旧实现直接调用 QAbstractItemView::scrollContentsBy 并手动维护 m_scrollOffset，容易导致状态错乱。
     * 新实现以滚动条为真值来源：将 dx/dy 转换为目标 offset，再通过 setScrollOffsetClamped 同步。
     */
    // 滚动条变化会进入 scrollContentsBy，在那里平移缓存。
    setScrollOffsetClamped(m_scrollOffset - QPoint(dx, dy));
}
void BaseTimelineView::horizontalScroll(double position)
{
    // position 是 0-1 之间的值，表示滚动条的相对位置
    int maxScroll = horizontalScrollBar()->maximum();
    int newScrollValue = static_cast<int>(maxScroll * position);

    // 滚动条变化会进入 scrollContentsBy，在那里平移缓存。
    horizontalScrollBar()->setValue(newScrollValue);
}
// 更新滚动条
void BaseTimelineView::updateScrollBars()
{
    /**
     * @brief 更新滚动条范围（与 TrackListView 使用相同几何口径）
     * @details
     * 以滚动条为唯一真值来源：
     * - 垂直内容总高度 = contentTop(标尺+工具栏) + trackCount * trackHeight
     * - 两个视图必须使用同一公式，否则会出现滚动不同步、行对不齐
     */
    if (!model())
        return;

    const int maxX = qMax(0, getTrackWdith() - viewport()->width());
    horizontalScrollBar()->setRange(0, maxX);

    const int contentTop = rulerHeight + toolbarHeight;
    const int totalHeight = model()->rowCount() * trackHeight + contentTop;
    const int maxY = qMax(0, totalHeight - viewport()->height());
    verticalScrollBar()->setRange(0, maxY);

    // range 变化可能导致 value 被 Qt 自动夹紧，这里同步一次缓存
    syncScrollOffsetFromScrollBars();
}

bool BaseTimelineView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    // 轨道名在左侧列表里双击编辑。时间轴上选中轨道时不要弹出输入框。
    if (!index.parent().isValid())
        return false;
    return QAbstractItemView::edit(index, trigger, event);
}

void BaseTimelineView::scrollContentsBy(int dx, int dy)
{
    /**
     * @brief 滚动条变化后的视口滚动
     * @details
     * dx/dy 是内容相对视口的位移：滚动条增大时为负，已有像素向左/上移动。
     * 缓存能平移时只重画新露出的条带；缩放、改内容或跨度超过视口时整页重建。
     */
    QAbstractItemView::scrollContentsBy(dx, dy);
    syncScrollOffsetFromScrollBars();
    if (dx == 0 && dy == 0)
        return;
    if (!shiftStaticCache(dx, dy))
        m_staticCacheDirty = true;
    if (viewport())
        viewport()->update();
}

bool BaseTimelineView::shiftStaticCache(int dx, int dy)
{
    QWidget* vp = viewport();
    if (!vp || !staticCacheReady())
        return false;

    const int w = vp->width();
    const int h = vp->height();
    if (w <= 0 || h <= 0)
        return false;
    // 一次跨过整个视口时，平移省不下绘制。
    if (qAbs(dx) >= w || qAbs(dy) >= h)
        return false;

    const int header = qBound(0, contentTop(), h);
    // 刻度横线画在 contentTop() 这一像素上。裁切高度若正好是 contentTop()，
    // 这一行会落进轨道区域，上下滚动时被轨道背景盖住。
    const int rulerBottom = qMin(h, header + 1);
    const qreal dpr = m_staticCache.devicePixelRatio();
    QPixmap shifted(m_staticCache.size());
    shifted.setDevicePixelRatio(dpr);
    shifted.fill(m_timelineBgColor);
    {
        QPainter blit(&shifted);
        // 标尺固定在顶部，只跟横向滚动走。底边横线留在标尺里。
        if (rulerBottom > 0) {
            blit.setClipRect(0, 0, w, rulerBottom);
            blit.drawPixmap(dx, 0, m_staticCache);
        }
        if (rulerBottom < h) {
            blit.setClipRect(0, rulerBottom, w, h - rulerBottom);
            blit.drawPixmap(dx, dy, m_staticCache);
        }
    }
    m_staticCache = shifted;

    // 多画 1 像素，盖住条带接缝上的刻度线。
    QRegion exposed;
    if (dx > 0)
        exposed += QRect(0, 0, qMin(w, dx + 1), h);
    else if (dx < 0)
        exposed += QRect(qMax(0, w + dx - 1), 0, qMin(w, -dx + 1), h);
    if (dy != 0 && rulerBottom < h) {
        const int contentH = h - rulerBottom;
        const int band = qMin(contentH, qAbs(dy) + 1);
        if (dy > 0)
            exposed += QRect(0, rulerBottom, w, band);
        else
            exposed += QRect(0, h - band, w, band);
        // 轨道条带的重绘会铺背景。标尺整段再画一次，底边横线才不会被盖住。
        exposed += QRect(0, 0, w, rulerBottom);
    }
    repaintStaticCache(exposed);
    return true;
}

void BaseTimelineView::repaintStaticCache(const QRegion& region)
{
    if (region.isEmpty() || m_staticCache.isNull() || !viewport())
        return;
    QPainter painter(&m_staticCache);
    painter.setClipRegion(region);
    const QRect rect(0, 0, viewport()->width(), viewport()->height());
    drawBackground(&painter, rect);
    drawVerticalTimeLines(&painter, rect);
    drawTracks(&painter);
    drawTimeRuler(&painter, rect);
}

int BaseTimelineView::pointToFrame(int point) const
{
    /**
     * 将像素位置转换为帧数（使用实例级 m_timescale，避免跨窗口干扰）
     * - 当 m_timescale 越小，表示更"缩小"，则每个像素代表更多帧
     */
    return qRound(static_cast<double>(point) / m_timescale);
}

int BaseTimelineView::frameToPoint(int frame) const
{
    /**
     * 将帧数转换为像素坐标（使用实例级 m_timescale，避免跨窗口干扰）
     * - 当 m_timescale 越小，表示更"缩小"，则每帧占据更少的像素
     */
    return qRound(static_cast<double>(frame) * m_timescale);
}

void BaseTimelineView::contextMenuEvent(QContextMenuEvent* event) {
    // 构建并弹出右键菜单（统一一个 QMenu 实例）
    QModelIndex index = indexAt(event->pos());
    QMenu contextMenu(this);

    // Focus cursor：滚动视图到播放头位置（尽量居中）
    QAction* focusCursorAction = new QAction("Focus cursor", this);
    focusCursorAction->setIcon(QIcon(":/icons/icons/location.png"));
    connect(focusCursorAction, &QAction::triggered, [this]() {
        const int playheadX = frameToPoint(getModel()->getPlayheadPos());
        const int viewWidth = viewport()->width();
        const int maxScroll = horizontalScrollBar()->maximum();
        int targetScroll = playheadX - viewWidth / 2;
        targetScroll = qBound(0, targetScroll, maxScroll);
        // 滚动条变化会进入 scrollContentsBy，在那里平移缓存。
        horizontalScrollBar()->setValue(targetScroll);
    });
    contextMenu.addAction(focusCursorAction);

    // 如果点击在轨道（无父）上，添加“Add Item”
    if (index.isValid() && !index.parent().isValid()) {
        QAction* addClipAction = new QAction("Add Item", this);
        addClipAction->setIcon(QIcon(":/icons/icons/add.png"));
        connect(addClipAction, &QAction::triggered, [this, index, event]() {
            addClipAtPosition(index, event->pos());
        });
        contextMenu.addAction(addClipAction);
    }

    // 如果点击在已选中的剪辑上，添加“Delete Clip”
    if (index.isValid() && selectionModel()->isSelected(index)) {
        QAction* deleteClipAction = new QAction("Delete Clip", this);
        deleteClipAction->setIcon(QIcon(":/icons/icons/trash.png"));
        connect(deleteClipAction, &QAction::triggered, [this, index]() {
            getModel()->onDeleteClip(index);
            selectionModel()->clearSelection();
            viewport()->update();
        });
        contextMenu.addAction(deleteClipAction);
    }

    // 统一显示菜单
    contextMenu.exec(event->globalPos());

    // 调用基类处理（保持与原逻辑一致）
    QAbstractItemView::contextMenuEvent(event);
}
//void BaseTimelineView::keyPressEvent(QKeyEvent *event)
//{
//    if(selectionModel()->selectedIndexes().isEmpty())
//        return;
//    if (event->key() == Qt::Key_Delete) {
//        // 处理删除键
//
//        QModelIndex currentIndex = selectionModel()->currentIndex();
//        getModel()->onDeleteClip(currentIndex);
//
//        // 清除选择并发送 nullptr
//        selectionModel()->clearSelection();
//        emit currentClipChanged(nullptr);
//        viewport()->update();
//    }
//
//}

void BaseTimelineView::addClipAtPosition(const QModelIndex& index, const QPoint& pos) {
    if (!index.isValid() || index.parent().isValid()) {
        qDebug() << "Invalid index: Click was not on a valid track";
        return;
    }

    int trackIndex = index.row();
    if (trackIndex < 0 || trackIndex >= getModel()->getTrackCount()) {
        qDebug() << "Invalid track index: " << trackIndex;
        return;
    }
    // Calculate the start frame based on the mouse position
    int startFrame = pointToFrame(pos.x() + m_scrollOffset.x());

    getModel()->onAddClip(trackIndex,startFrame);

    // 更新视图（优化为合并重绘）
    scheduleRedraw();
}

void BaseTimelineView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // 处理左键按下的情况
        QAbstractItemView::mouseReleaseEvent(event);
        return;
    }

    finishClipDrag();

    // 结束画布平移模式
    

        m_isPanning = false;
        // m_panMoved = false;
  

    mouseHeld = false;
    m_playheadSelected = false;
    m_mouseEnd = event->pos();
    updateCursorShape();
    //pressed outside of selection
    QAbstractItemView::mouseReleaseEvent(event);
}

void BaseTimelineView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    // 创建并显示属性对话框
    showClipProperty(index);
    QAbstractItemView::mouseDoubleClickEvent(event);
}

void BaseTimelineView::leaveEvent(QEvent *event)
{
    finishClipDrag();
    mouseHeld = false;
    // selectionModel()->clear();
    m_hoverIndex = QModelIndex();
    QAbstractItemView::leaveEvent(event);
}

// 设置缩放比例
void BaseTimelineView::setScale(double value)
{
    /**
     * 根据归一化比例更新本视图实例的时间刻度（像素/帧）
     * - 不再修改全局 timescale，避免不同窗口互相影响
     * - 保持播放头焦点在缩放后视觉位置不变
     */
    // 1. 先保存旧的 timescale 用于计算旧焦点像素位置
    double oldTimescale = m_timescale;

    // 2. 将输入值钳制到 [0.0, 1.0]
    double ratio = qBound(0.0, value, 1.0);

    // 3. 将 ratio 线性映射到 [m_minTimescale, m_baseTimeScale]
    //    ratio=0 -> m_timescale=m_minTimescale；ratio=1 -> m_timescale=m_baseTimeScale
    m_timescale = m_minTimescale + ratio * (static_cast<double>(m_baseTimeScale) - m_minTimescale);

    // 4. 保持播放头焦点位置不变
    int focusFrame = getModel()->getPlayheadPos();
    int oldPointFocus = qRound(static_cast<double>(focusFrame) * oldTimescale);
    int newPointFocus = qRound(static_cast<double>(focusFrame) * m_timescale);
    int diff = newPointFocus - oldPointFocus;

    // 5. 刻度已经变了，旧缓存不能按像素平移。先标脏，滚动条夹紧时不要拿旧图去补条带。
    m_staticCacheDirty = true;
    setScrollOffsetClamped(QPoint(m_scrollOffset.x() + diff, m_scrollOffset.y()));

    // 6. 更新界面
    currentScale = ratio;
    // qDebug()<<"currentScale:"<<currentScale;
    scheduleRedraw();
}

void BaseTimelineView::resizeEvent(QResizeEvent *event)
{
    scheduleRedraw();
    QAbstractItemView::resizeEvent(event);

    // 更新ZoomController
    // emit timelineInfoChanged(getTrackWdith(), viewport()->width(), m_scrollOffset.x());
}

void BaseTimelineView::showEvent(QShowEvent *event)
{
    /**
     * @brief 显示事件
     * @details
     * 兜底逻辑：
     * - 如果外部没有注入共享 selectionModel，则创建一个本地 selectionModel。
     * - 如果本视图未被 Controller 托管（独立使用），则在此处按需绑定“模型变化 -> 合并重绘”。
     */
    if (!selectionModel() && model()) {
        setSelectionModel(new QItemSelectionModel(model(), this));
    }

    // 独立使用场景：没有 Controller 时，视图需要自己监听模型变化来刷新
    if (!property("qtimeline_managed").toBool() && !property("qtimeline_model_bound").toBool() && Model) {
        connect(Model, &QAbstractItemModel::modelReset,   this, [this]() { scheduleRedraw(); });
        connect(Model, &QAbstractItemModel::rowsInserted, this, [this]() { scheduleRedraw(); });
        connect(Model, &QAbstractItemModel::rowsRemoved,  this, [this]() { scheduleRedraw(); });
        connect(Model, &QAbstractItemModel::rowsMoved,    this, [this]() { scheduleRedraw(); });
        connect(Model, &QAbstractItemModel::dataChanged,  this, [this]() { scheduleRedraw(); });

        connect(Model, &BaseTimeLineModel::S_playheadMoved, this, [this](int frame) { onPlayheadMoved(frame); });
        connect(Model, &BaseTimeLineModel::S_LengthChanged,       this, [this]() { scheduleRedraw(); });

        setProperty("qtimeline_model_bound", true);
    }

    updateScrollBars();
    QAbstractItemView::showEvent(event);
}

void BaseTimelineView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QAbstractItemView::mousePressEvent(event);
        return;
    }

    m_mouseStart = event->pos();
    m_mouseEnd = event->pos();
    mouseHeld = true;

    // 清除当前选择（selectionModel 可能由外部注入；兜底情况也可能尚未创建）
    if (selectionModel()) {
        selectionModel()->clearSelection();
    }
    // // 获取播放头位置
    // int playheadPos = frameToPoint(((BaseTimeLineModel*)model())->getPlayheadPos());
    // // 获取播放头矩形
    // QRect playheadHitBox(QPoint(playheadPos-3,rulerHeight),QPoint(playheadPos+2,viewport()->height()));
    // // 获取播放头矩形2
    // QRect playheadHitBox2(QPoint(playheadPos-playheadwidth,-playheadheight + rulerHeight),QPoint(playheadPos+playheadwidth,rulerHeight));
    // // 如果点击在播放头上
    // if(playheadHitBox.contains(m_mouseStart) || playheadHitBox2.contains(m_mouseStart)) 

    //鼠标点击位置在标尺上就移动播放头
    if (m_mouseStart.y() <= rulerHeight + toolbarHeight) 
    {
        m_playheadSelected = true;
        return;
    }
    QModelIndex item = indexAt(event->pos());
    //如果点击在轨道上的的空白处
    if(item.isValid() && !item.parent().isValid()) {
        m_playheadSelected = true;
        return;
    }
    m_playheadSelected = false;

    // 如果点击到了片段
    if(item.isValid() && item.parent().isValid()) {
        // 设置当前项和选择
        selectionModel()->setCurrentIndex(item, QItemSelectionModel::ClearAndSelect);
        setCursor(Qt::ClosedHandCursor);
        m_mouseOffset.setX(frameToPoint(item.data(TimelineRoles::ClipInRole).toInt()) - m_mouseStart.x());
        viewport()->update();
        return;
    }
    if(!item.isValid() && !item.parent().isValid()) {
        // 点击在空白处，不在轨道、片段、播放头上
        m_isPanning = true;
        m_panStart = event->pos();
        // 开始平移前同步一次滚动偏移缓存，避免历史状态不一致
        syncScrollOffsetFromScrollBars();
        m_panInitialOffset = m_scrollOffset;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    
}

/**
 * 鼠标移动事件处理函数
 * 处理鼠标移动时的各种交互行为
 * @param event 鼠标事件对象
 */
void BaseTimelineView::mouseMoveEvent(QMouseEvent *event)
{
    // 处理右键事件
    if (event->button() == Qt::RightButton) {
        QAbstractItemView::mouseMoveEvent(event);
        return;
    }
    // 处理鼠标按住拖动的情况
    if (mouseHeld) {
        handleMouseDrag(event);
        return;
    }

    // 处理鼠标悬停状态
    updateMouseHoverState(event);
    updateCursorShape();
    
    QAbstractItemView::mouseMoveEvent(event);
}

/**
 * 处理鼠标拖动操作
 * @param event 鼠标事件对象
 */
void BaseTimelineView::handleMouseDrag(QMouseEvent *event)
{
    m_mouseEnd = event->pos();
    
    // 处理播放头拖动
    if (m_playheadSelected) {
        movePlayheadToFrame(pointToFrame(std::max(0, m_mouseEnd.x() + m_scrollOffset.x())));
        return;
    }
    // 处理画布平移
    if (m_isPanning) {
        const int dx = event->pos().x() - m_panStart.x();
        const int dy = event->pos().y() - m_panStart.y();
        // 仅水平平移
        QPoint newOffset = m_panInitialOffset - QPoint(dx, 0);

        // 约束滚动范围：水平范围由滚动条/视口宽度决定
        const int minX = 0;
        const int maxX = qMax(minX, getTrackWdith() - viewport()->width());
        newOffset.setX(qBound(minX, newOffset.x(), maxX));

        // 通过滚动条驱动平移，避免直接写 m_scrollOffset 导致状态不同步
        if (newOffset != m_scrollOffset) {
            // 滚动条变化会进入 scrollContentsBy，在那里平移缓存。
            setScrollOffsetClamped(newOffset);
        }
        event->accept();
        return;
    }
    if (!selectionModel())
        return;
    const QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    const QModelIndex clipIndex = selectedIndexes.first();
    auto* clip = static_cast<AbstractClipModel*>(clipIndex.internalPointer());
    if (!clip)
        return;

    const bool moving = m_mouseUnderClipEdge == hoverState::CENTER;
    const bool resizing = m_mouseUnderClipEdge == hoverState::LEFT || m_mouseUnderClipEdge == hoverState::RIGHT;
    if (!moving && !resizing)
        return;

    if (!m_interactiveDrag)
        beginClipDrag(clip, clipIndex);

    // Trigger 等固定长度片段会把标题画到几何矩形右侧外（向右约 40px）。
    // 只刷新 visualRect 时，这段旧文字清不掉，拖动就会叠成一串。
    const QRect oldRect = visualRect(clipIndex).adjusted(-4, -4, 44, 4);
    switch (m_mouseUnderClipEdge) {
        case hoverState::CENTER:
            moveSelectedClip(pointToFrame(m_mouseEnd.x() + m_mouseOffset.x()),
                             m_mouseEnd.y() + m_mouseOffset.y());
            break;
        case hoverState::LEFT:
            getModel()->setData(clipIndex, pointToFrame(m_mouseEnd.x() + m_scrollOffset.x()), TimelineRoles::ClipInRole);
            break;
        case hoverState::RIGHT:
            getModel()->setData(clipIndex, pointToFrame(m_mouseEnd.x() + m_scrollOffset.x()), TimelineRoles::ClipOutRole);
            break;
        default:
            break;
    }
    const QRect nextRect = visualRect(clipIndex).adjusted(-4, -4, 44, 4);
    viewport()->update(oldRect.united(nextRect));
}

void BaseTimelineView::beginClipDrag(AbstractClipModel* clip, const QModelIndex& index)
{
    if (!clip || !index.isValid() || !Model || m_interactiveDrag)
        return;
    m_interactiveDrag = true;
    m_dragClip = clip;
    m_dragIndex = index;
    Model->beginInteractiveGeometryChange(clip);
    // 不在 QAbstractScrollArea 上 grabMouse。按下时鼠标已经由视口捕获，
    // 再抢到外层视图会让松手事件到不了 mouseReleaseEvent，拖拽无法结束。
    m_staticCacheDirty = true;
}

void BaseTimelineView::finishClipDrag()
{
    if (!m_interactiveDrag)
        return;
    m_interactiveDrag = false;
    m_dragClip = nullptr;
    m_dragIndex = QPersistentModelIndex();
    // dataChanged 会按旧位置和新位置补进缓存，避免松手后再整页重建。
    if (Model)
        Model->endInteractiveGeometryChange();
}

/**
 * 更新鼠标悬停状态
 * @param event 鼠标事件对象
 */
void BaseTimelineView::updateMouseHoverState(QMouseEvent *event)
{
    QPoint pos = event->pos();
    m_hoverIndex = indexAt(pos);
    QRect rect = visualRect(m_hoverIndex);
    
    // 默认情况下无悬浮状态
    m_mouseUnderClipEdge = hoverState::NONE;
    
    // 检查是否悬停在片段上
    // bool isValidClip = m_hoverIndex.isValid() && m_hoverIndex.parent().isValid();
    if (m_hoverIndex.isValid() && m_hoverIndex.parent().isValid())
     {
        // 获取片段对象
        AbstractClipModel* clip = static_cast<AbstractClipModel*>(m_hoverIndex.internalPointer());
        if (!clip) {
            return;
        }
        // 设置悬停状态为中心
        m_mouseUnderClipEdge = hoverState::CENTER;
        // 检查是否可调整大小的片段
        if (clip->isResizable()) {
            // 检查是否在左边缘
            if (abs(pos.x() - rect.left()) <= 5) {
                m_mouseUnderClipEdge = hoverState::LEFT;
            }
            // 检查是否在右边缘
            else if (abs(pos.x() - rect.right()) <= 5) {
                m_mouseUnderClipEdge = hoverState::RIGHT;
            }
        }
        return;
    }
    if (m_hoverIndex.isValid() && !m_hoverIndex.parent().isValid())
    {
        //悬浮在轨道上时
        return;
    }
    if (!m_hoverIndex.isValid() && !m_hoverIndex.parent().isValid())
    {
        return;
    }
}

/**
 * 根据鼠标悬停状态更新鼠标指针形状
 */
void BaseTimelineView::updateCursorShape()
{
    switch (m_mouseUnderClipEdge) {
        case hoverState::LEFT:
            setCursor(Qt::SizeHorCursor);
            break;
        case hoverState::RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;
        case hoverState::CENTER:
            setCursor(Qt::OpenHandCursor);
            break;
        case hoverState::NONE:
        default:
            unsetCursor();
            break;
    }
}

void BaseTimelineView::dragEnterEvent(QDragEnterEvent *event)
{

        event->acceptProposedAction();
        m_isSupportMedia = true;
        // QAbstractItemView::dragEnterEvent(event);

}

void BaseTimelineView::dragMoveEvent(QDragMoveEvent *event)
{

    m_lastDragPos = event->position().toPoint();

    // 检查当前位置是否在轨道上
    QModelIndex trackIndex;


    // 排除工具栏和标尺区域
    if (m_lastDragPos.y() < rulerHeight + toolbarHeight) {
        event->ignore();
        m_isSupportMedia = false;
        return;
    }

    // 查找轨道
    bool onTrack = false;
    for (int i = 0; i < getModel()->rowCount(); i++) {
        if (visualRect(getModel()->index(i, 0)).contains(m_lastDragPos)) {
            trackIndex = getModel()->index(i, 0);
            onTrack = true;
            break;
        }
    }
    if (onTrack) {

        // 获取文件类型
        QString fileType = isMimeAcceptable(event->mimeData());
        // 获取轨道类型
        QString trackType = getModel()->data(trackIndex, TimelineRoles::TrackTypeRole).toString();

        // 判断文件类型和轨道类型是否匹配
        if (!fileType.isEmpty() && fileType == trackType) {
            // 类型匹配时接受拖动
            m_isSupportMedia = true;
            event->acceptProposedAction();

        }
        else {
            // 类型不匹配时拒绝拖动
            m_isSupportMedia = false;
            event->ignore();
            unsetCursor();
        }

    } else {
        QAbstractItemView::dragMoveEvent(event);
    }
}

void BaseTimelineView::dropEvent(QDropEvent *event)
{
    if(m_isSupportMedia){
        m_lastDragPos = event->position().toPoint();
        QModelIndex trackIndex;
        QRect rullerRect(-m_scrollOffset.x(),0,viewport()->width() + m_scrollOffset.x(),rulerHeight);
        /* If above or on the ruler drop on the first track*/
        if(m_lastDragPos.y()<0 || rullerRect.contains(m_lastDragPos)){
            if(getModel()->rowCount()>0)
                trackIndex = model()->index(0, 0);
        }else{
            /* Find track at drop point */
            for(int i = 0; i < getModel()->rowCount(); i++){
                if (visualRect(getModel()->index(i, 0)).contains(m_lastDragPos)){
                    trackIndex = getModel()->index(i,0);
                }
            }
        }
        /* If dropped out side of tracks */
        if(!trackIndex.isValid()){
            return;
        }
        int pos = pointToFrame(m_lastDragPos.x());

        getModel()->onAddClip(trackIndex.row(),pos);
        viewport()->update();

    }else{
        QAbstractItemView::dropEvent(event);
    }
}

 bool BaseTimelineView::eventFilter(QObject *watched, QEvent *event)
 {
      if (event->type() == QEvent::KeyPress) {
          QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
          QModelIndexList list = selectionModel()->selectedIndexes();
          BaseTimeLineModel* timelinemodel = (BaseTimeLineModel*)model();
          if (watched == this) {
              switch (keyEvent->key()){
                  case Qt::Key_Right :
                      moveSelectedClip(1,0,false);
                      return true;  // 统一返回 true
                  case Qt::Key_Left:
                      moveSelectedClip(-1,0,false);
                      return true;
                  case Qt::Key_Delete: {
                      QModelIndex currentIndex = selectionModel()->currentIndex();
                      getModel()->onDeleteClip(currentIndex);
                      // 清除选择并发送 nullptr
                      selectionModel()->clearSelection();
                      emit currentClipChanged(nullptr);
                      viewport()->update();
                      return true;
                  }
                  default:
                      break;
              }
          }
      }
      return QObject::eventFilter(watched, event);
 }

void BaseTimelineView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    syncScrollOffsetFromScrollBars();
    ensureStaticCache();

    QPainter painter(viewport());
    if (!m_staticCache.isNull())
        painter.drawPixmap(QPoint(0, 0), m_staticCache);
    if (m_interactiveDrag && m_dragClip && m_dragIndex.isValid()) {
        const QRect clipRect = visualRect(m_dragIndex).adjusted(0, clipoffset, 0, -clipoffset);
        const bool selected = selectionModel() && selectionModel()->isSelected(m_dragIndex);
        m_dragClip->paint(&painter, clipRect, selected);
    }
    drawPlayhead(&painter);

    if (Model)
        m_paintedPlayheadFrame = static_cast<int>(Model->getPlayheadPos());
}

void BaseTimelineView::drawBackground(QPainter* painter, const QRect& rect)
{
    // 绘制背景
    painter->fillRect(rect, m_timelineBgColor);

    // 设置分割线颜色并绘制轨道分隔线
    painter->setPen(m_timelineSeparatorColor);
    int firstRow = 0;
    int lastRow = -1;
    visibleTrackRange(firstRow, lastRow);
    if (lastRow < firstRow)
        return;
    for (int i = qMax(0, firstRow - 1); i <= lastRow; ++i) {
        const int trackSplitter = (i + 1) * trackHeight + contentTop() - m_scrollOffset.y();
        if (trackSplitter < contentTop() || trackSplitter > rect.bottom())
            continue;
        painter->drawLine(0, trackSplitter, rect.width(), trackSplitter);
    }
}

void BaseTimelineView::drawVerticalTimeLines(QPainter* painter, const QRect& rect)
{
    int lineheight = model()->rowCount() * trackHeight + contentTop();

    // 计算时间线间隔
    double frameRate = timecode_frames_per_sec(Model->getTimeCodeType());
    int frameStep = calculateFrameStep(frameRate);

    // 计算起始和结束位置
    int startMarker = static_cast<int>(pointToFrame(m_scrollOffset.x())) * m_timescale + 1;
    int endMarker = rect.width() + m_scrollOffset.x();
    startMarker = pointToFrame(startMarker);
    startMarker -= (startMarker % frameStep);
    startMarker = frameToPoint(startMarker);

    // 绘制垂直时间线
    for (int i = startMarker; i < endMarker; i += m_timescale * frameStep) {
        painter->drawLine(i - m_scrollOffset.x(),
                         std::max(rulerHeight, rect.top()),
                         i - m_scrollOffset.x(),
                         lineheight);
    }
}

int BaseTimelineView::calculateFrameStep(double frameRate) const
{
    /**
     * 函数说明：
     * - 根据当前 m_timescale（像素/帧）和字体大小，计算一个不会导致标签重叠的帧步长。
     * - 再从一组"漂亮步长"（1、2、5、10、25、50、100、帧率）中选取第一个不小于该步长的值。
     */

    /**
     * 按当前实例的 m_timescale 计算合适的刻度步长，避免跨窗口共享状态
     */
    // 估算标签的最小像素间距：与字体大小相关（也可改为用 QFontMetrics 的度量）
    const int minPixelSpacing = qMax(30, static_cast<int>(fontSize * 1.8));

    // 像素间距 => 帧间距（ceil 保证足够间距）
    int framesByPixels = qMax(1, static_cast<int>(std::ceil(static_cast<double>(minPixelSpacing) / m_timescale)));

    // "漂亮"的步长集合（可按需扩展）
    const int fpsRounded = static_cast<int>(std::round(frameRate));
    const QVector<int> niceSteps = { fpsRounded/2, fpsRounded, fpsRounded * 5,fpsRounded * 10,fpsRounded * 30, fpsRounded * 60};

    // 选择第一个 >= framesByPixels 的漂亮步长；若都小于，则用最后一个
    int step = niceSteps.last();
    for (int s : niceSteps) {
        if (s >= framesByPixels) { step = s; break; }
    }

    return step;
}

void BaseTimelineView::drawTimeRuler(QPainter* painter, const QRect& rect)
{
    /**
     * 函数说明：
     * - 绘制标尺背景，计算起止范围与帧步长。
     * - 使用当前 timescale 与帧步长确定像素间距，避免文本重叠。
     */

    // 绘制标尺背景
    painter->setPen(m_timelineRulerColor);
    painter->setBrush(QBrush(m_timelineBgColor));
    painter->drawRect(-m_scrollOffset.x(), 0,
                     rect.width() + m_scrollOffset.x(),
                     contentTop());

    // 计算时间线间隔（帧步长）
    const double frameRate = timecode_frames_per_sec(Model->getTimeCodeType());
    const int frameStep = calculateFrameStep(frameRate);

    // 计算起始/结束位置（以帧为单位，规整到步长边界）
    int startFrame = pointToFrame(m_scrollOffset.x());
    startFrame -= (startFrame % frameStep);

    const int startX = frameToPoint(startFrame);
    const int endX   = rect.width() + m_scrollOffset.x();

    // 绘制时间标记和文本（根据像素间距决定是否绘制文本）
    drawTimeMarkers(painter, startX, endX, frameStep);
}

void BaseTimelineView::drawTimeMarkers(QPainter* painter, int startMarker, int endMarker, int frameStep)
{
    /**
     * 函数说明：
     * - 按帧步长绘制刻度线。
     * - 当像素间距不足最小间距时，仅绘制刻度线不绘制文本，避免重叠。
     */

    // 计算每个刻度的像素间距
    /**
     * 使用实例级 m_timescale 计算刻度像素间距，避免跨窗口共享状态导致的干扰
     */
    const int tickPixelStep = static_cast<int>(std::round(m_timescale * frameStep));

    // 依据字体大小设定最小文本间距
    const int minLabelPixelSpacing = qMax(30, static_cast<int>(fontSize * 1.8));
    const bool canDrawText = tickPixelStep >= minLabelPixelSpacing;

    for (int x = startMarker; x < endMarker; x += tickPixelStep) {
        const int frameNumber = pointToFrame(x);

        // 画刻度线
        painter->drawLine(x - m_scrollOffset.x(),
                          contentTop() - textoffset,
                          x - m_scrollOffset.x(),
                          contentTop());

        if (canDrawText) {
           // 文本格式化为 mm:ss.ff（分钟:秒.帧）
            const double frameRate = timecode_frames_per_sec(Model->getTimeCodeType());
            const int fps = static_cast<int>(std::round(frameRate)); // 将非整数帧率四舍五入用于显示
            const qint64 totalFrames  = static_cast<qint64>(frameNumber);
            const qint64 totalSeconds = (fps > 0) ? (totalFrames / fps) : 0;
            const int ff = (fps > 0) ? static_cast<int>(totalFrames % fps) : 0;
            const int mm = static_cast<int>(totalSeconds / 60);
            const int ss = static_cast<int>(totalSeconds % 60);

            const QString text = QString("%1:%2.%3")
                                    .arg(mm, 2, 10, QChar('0'))
                                    .arg(ss, 2, 10, QChar('0'))
                                    .arg(ff, 2, 10, QChar('0'));

            QRect textRect = painter->fontMetrics().boundingRect(text);

            // 将文本居中到刻度线
            textRect.translate(-m_scrollOffset.x(), 0);
            textRect.translate(x - textRect.width() / 2, contentTop() - textoffset);

            painter->drawText(textRect, text);
        }
    }
}

void BaseTimelineView::drawPlayhead(QPainter* painter)
{
    // 绘制播放头三角形
    QPoint kite[5] = {
        QPoint(0,0),
        QPoint(-playheadwidth, -playheadCornerHeight),
        QPoint(-playheadwidth, -playheadheight),
        QPoint(playheadwidth, -playheadheight),
        QPoint(playheadwidth, -playheadCornerHeight)
    };

    int playheadPos = frameToPoint(getModel()->getPlayheadPos()) - m_scrollOffset.x();
    
    // 移动播放头到正确位置
    for (QPoint &p : kite) {
        p.setX(p.x() + playheadPos);
        p.setY(p.y() + contentTop());
    }

    // 设置画笔和画刷颜色并绘制
    painter->setPen(m_timelinePlayheadColor);
    painter->setBrush(m_timelinePlayheadColor);
    painter->drawConvexPolygon(kite, 5);
    painter->drawLine(QPoint(playheadPos, contentTop()),
                     QPoint(playheadPos, viewport()->height()));
}

/**
 * 设置时间线背景色
 * @param QColor color 背景色
 */
void BaseTimelineView::setTimelineBgColor(const QColor& color) {
    m_timelineBgColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置时间线分隔线颜色
 * @param QColor color 分隔线颜色
 */
void BaseTimelineView::setTimelineSeparatorColor(const QColor& color) {
    m_timelineSeparatorColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置标尺颜色
 * @param QColor color 标尺颜色
 */
void BaseTimelineView::setTimelineRulerColor(const QColor& color) {
    m_timelineRulerColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置播放头颜色
 * @param QColor color 播放头颜色
 */
void BaseTimelineView::setTimelinePlayheadColor(const QColor& color) {
    m_timelinePlayheadColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置片段填充颜色
 * @param QColor color 片段填充颜色
 */
void BaseTimelineView::setClipFillColor(const QColor& color) {
    m_clipFillColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置片段边框颜色
 * @param QColor color 片段边框颜色
 */
void BaseTimelineView::setClipBorderColor(const QColor& color) {
    m_clipBorderColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 设置轨道选中颜色
 * @param QColor color 轨道选中颜色
 */
void BaseTimelineView::setTrackSelectedColor(const QColor& color) {
    m_trackSelectedColor = color;
    applyStyleFromProperties();
    viewport()->update();
}
/**
 * 样式变化事件处理，确保 QSS 生效后刷新颜色
 * @param QEvent* e 事件
 */
void BaseTimelineView::changeEvent(QEvent* e) {
    QAbstractItemView::changeEvent(e);
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::PaletteChange) {
        applyStyleFromProperties();
        viewport()->update();
    }
}
/**
 * 从属性应用颜色到全局样式（兼容旧绘制逻辑和其他文件使用的全局色）
 */
void BaseTimelineView::applyStyleFromProperties() {
    // 更新旧代码使用的全局颜色（TimeLineStyle.h）
    bgColour            = m_timelineBgColor;
    seperatorColour     = m_timelineSeparatorColor;
    rulerColour         = m_timelineRulerColor;
    playheadColour      = m_timelinePlayheadColor;
    ClipColor           = m_clipFillColor;
    ClipBorderColour    = m_clipBorderColor;
    trackSelectedColour = m_trackSelectedColor;
    m_staticCacheDirty = true;
}

void BaseTimelineView::visibleTrackRange(int& firstRow, int& lastRow) const
{
    firstRow = 0;
    lastRow = -1;
    if (!model() || trackHeight <= 0)
        return;
    const int count = model()->rowCount();
    if (count <= 0)
        return;

    const int scrollY = verticalScrollBar()->value();
    const int top = contentTop();
    const int bottomContent = scrollY + viewport()->height() - top - 1;
    if (bottomContent < 0)
        return;

    lastRow = qMin(count - 1, bottomContent / trackHeight);
    firstRow = scrollY <= top ? 0 : qMin(count - 1, (scrollY - top) / trackHeight);
    if (firstRow > lastRow)
        lastRow = -1;
}

void BaseTimelineView::drawTracks(QPainter* painter)
{
    if (!model() || !painter)
        return;

    int firstRow = 0;
    int lastRow = -1;
    visibleTrackRange(firstRow, lastRow);
    if (lastRow < firstRow)
        return;

    const int scrollX = horizontalScrollBar()->value();
    const int scrollY = verticalScrollBar()->value();
    const int viewRight = scrollX + viewport()->width();
    const int padFrames = qMax(1, pointToFrame(4));
    const int frameLo = pointToFrame(qMax(0, scrollX)) - padFrames;
    const int frameHi = pointToFrame(viewRight) + padFrames;
    const bool clipped = painter->hasClipping();
    const QRect clipBounds = clipped ? painter->clipBoundingRect().toAlignedRect() : QRect();
    const int top = contentTop();
    const AbstractClipModel* selectedClip = nullptr;
    if (selectionModel()) {
        const QModelIndex current = selectionModel()->currentIndex();
        if (current.parent().isValid())
            selectedClip = static_cast<AbstractClipModel*>(current.internalPointer());
    }
    QList<int> clipRows;

    for (int i = firstRow; i <= lastRow; ++i) {
        const int trackTop = i * trackHeight + top - scrollY;
        if (clipped && (trackTop + trackHeight <= clipBounds.top() || trackTop >= clipBounds.bottom()))
            continue;

        const QModelIndex trackIndex = model()->index(i, 0);
        auto* track = static_cast<TrackData*>(trackIndex.internalPointer());
        if (!track)
            continue;
        clipRowsInFrameRange(track, frameLo, frameHi, clipRows);
        for (int clipRow : clipRows) {
            if (clipRow < 0 || clipRow >= track->clips.size())
                continue;
            AbstractClipModel* clip = track->clips.at(clipRow);
            if (!clip || (m_interactiveDrag && clip == m_dragClip))
                continue;

            const int startX = frameToPoint(clip->start());
            const int clipWidth = frameToPoint(clip->end() - clip->start() + 1);
            if (startX + clipWidth <= scrollX || startX >= viewRight)
                continue;
            const int paintWidth = qMax(clipWidth, 1);
            if (clipped && !QRect(startX - scrollX, trackTop, paintWidth, trackHeight).intersects(clipBounds))
                continue;

            // 与 clipContentBounds 同一套几何，避免再走 visualRect / parent()。
            m_clipContentRects[clip] = QRect(startX, trackTop + scrollY, paintWidth, trackHeight).adjusted(-4, -4, 44, 4);
            const QRect clipRect(startX - scrollX, trackTop + clipoffset, clipWidth, trackHeight - clipoffset * 2);
            clip->paint(painter, clipRect, clip == selectedClip);
        }
    }
}

void BaseTimelineView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QAbstractItemView::selectionChanged(selected, deselected);
    
    // 获取当前选中的索引
    QModelIndex currentIndex = selectionModel()->currentIndex();
    
    // 发送选中片段变化信号
    if (currentIndex.isValid() && currentIndex.parent().isValid()) {
        auto* clip = static_cast<AbstractClipModel*>(currentIndex.internalPointer());
        emit currentClipChanged(clip);
    } else {
        emit currentClipChanged(nullptr);
    }

    m_staticCacheDirty = true;
    viewport()->update();
}

void BaseTimelineView::wheelEvent(QWheelEvent *event){
    /**
     * 函数说明：支持 Ctrl+滚轮进行缩放；否则进行水平滚动
     * - 缩放使用乘法步进，并钳制到 [minNormalized, 1.0]，避免无限缩小
     * - 保持与 setScale 的最小阈值一致
     */
    if (event->modifiers() & Qt::ControlModifier) {
        // 缩放操作
        QPoint pos = event->position().toPoint();
        double delta = event->angleDelta().y() / 120.0; // 标准滚轮步长
        double scaleChange = (delta > 0 ? 1.1 : 0.9);    // 每次滚动的缩放倍率

        // 允许达到指定下限，从而触达 setScale 的最小可见缩放
        const double minNormalized = minTimescale; // 与 setScale 保持一致
        currentScale = qBound(minNormalized, currentScale * scaleChange, 1.0);

        setScale(currentScale);
    } else {
        /**
         * @brief 非 Ctrl 情况下滚轮用于水平滚动
         * @details
         * 以滚动条为真值来源：通过设置 horizontalScrollBar()->setValue 驱动滚动，
         * 避免只改 m_scrollOffset 造成“视觉滚动了但滚动条没动/命中测试不一致”。
         */
        const int deltaX = -event->angleDelta().y() / 2; // 减小滚动速度

        const int oldX = horizontalScrollBar()->value();
        const int newX = qBound(horizontalScrollBar()->minimum(), oldX + deltaX, horizontalScrollBar()->maximum());

        if (newX != oldX) {
            // 滚动条变化会进入 scrollContentsBy，在那里平移缓存。
            horizontalScrollBar()->setValue(newX);
            emit scrolled(newX - oldX, 0);
        }
    }
    event->accept();
}

void BaseTimelineView::onFrameChanged(qint64 frame)
{
    onPlayheadMoved(static_cast<int>(frame));
}

void BaseTimelineView::onPlaybackStateChanged(bool isPlaying)
{
    dynamic_cast<DefaultTimeLineToolBar*>(m_toolbar)->setPlaybackState(isPlaying);
    
}

void BaseTimelineView::showClipProperty(const QModelIndex& index)
{
    //判断索引不为为片段时
    if(!index.isValid() || !index.parent().isValid()){
        return;
    }
    //获取片段
    AbstractClipModel* clip = static_cast<AbstractClipModel*>(index.internalPointer());
    if (!clip) return;
        //获取属性窗口
        clip->showPropertyWidget();
}

BaseTimeLineModel* BaseTimelineView::getModel() const
{
    return Model;
}

/**
 * @brief 检查文件是否可接受
 * @param QString filePath 文件路径
 * @return bool 是否可接受
 */
QString BaseTimelineView::isMimeAcceptable(const QMimeData *Mime) const
{
    // 获取拖拽的文件路径

    if (Mime->hasUrls())
    {
        QString filePath = Mime->urls().first().toLocalFile();
        QFileInfo fileInfo(filePath);
        QString suffix = fileInfo.suffix().toLower();
        // 根据后缀判断文件类型
        if(VideoTypes.contains(suffix, Qt::CaseInsensitive)) {
            return "Video";
        }
        else if(AudioTypes.contains(suffix, Qt::CaseInsensitive)) {
            return "Audio";
        }
        else if(ImageTypes.contains(suffix, Qt::CaseInsensitive)) {
            return "Image";
        }
        else if(ControlTypes.contains(suffix, Qt::CaseInsensitive)) {
            return "Control";
        }
    }else if (Mime->hasFormat("application/x-osc-address"))
        {
            return "Control";
        }
    return "";
}
