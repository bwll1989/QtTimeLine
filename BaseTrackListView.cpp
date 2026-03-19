#include "BaseTrackListView.h"
#include <QDrag>
#include <QMimeData>
#include <QMenu>
BaseTracklistView::BaseTracklistView(BaseTimeLineModel *viemModel, QWidget *parent): Model(viemModel), QAbstractItemView{parent}{
    /**
     * @brief 构造轨道列表视图
     * @details
     * 关键点：本视图不再“自己 new selectionModel”。
     * - 以前 TrackListView 和 TimelineView 各自持有 selectionModel，会产生两套选择状态，导致逻辑越来越乱。
     * - 重构后 selectionModel 由外部容器（BaseTimelineWidget）统一创建并注入，从而保证选择状态唯一。
     */
    setObjectName("TrackListView");
    setModel(Model);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // selectionModel 由外部注入（BaseTimelineWidget 中的共享 selectionModel）
    // 如果有人单独使用 BaseTracklistView（不通过 BaseTimelineWidget），则会在 showEvent 中补一个兜底。

    horizontalScrollBar()->setSingleStep(10);
    horizontalScrollBar()->setPageStep(100);
    verticalScrollBar()->setSingleStep(trackHeight);
    verticalScrollBar()->setPageStep(trackHeight );
    viewport()->setMinimumHeight(trackHeight + rulerHeight+toolbarHeight);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);

    // Create delete track action
    m_deleteTrackAction = new QAction("Delete track", this);
    m_deleteTrackAction->setIcon(QIcon(":/icons/icons/trash.png"));
    m_deleteTrackAction->setShortcut(QKeySequence(Qt::Key_Delete));
    QObject::connect(m_deleteTrackAction, &QAction::triggered, this, &BaseTracklistView::onDeleteTrack);
    setItemDelegate(new BaseTrackDelegate(this));
    setMouseTracking(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded ) ;
    setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded) ;
    installEventFilter(this);
    
    
}

void BaseTracklistView::setTrackColor(const QColor &color)
{
    m_trackColor = color;
    viewport()->update();
}

void BaseTracklistView::setTrackSelectedColor(const QColor &color)
{
    m_trackSelectedColor = color;
    viewport()->update();
}

void BaseTracklistView::setTrackHoverColor(const QColor &color)
{
    m_trackHoverColor = color;
    viewport()->update();
}

/**
 * @brief 删除轨道
 */
void BaseTracklistView::onDeleteTrack() {
    /**
     * @brief 删除当前选中的轨道
     * @details
     * persistent editor 的创建/关闭统一收敛到 onUpdateViewport/ensureVisiblePersistentEditors。
     */
    if (!Model || !selectionModel()) {
        return;
    }

    const QModelIndexList selected = selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return;
    }

    const int deleteRow = selected.first().row();
    if (deleteRow < 0 || deleteRow >= Model->rowCount()) {
        return;
    }

    const QModelIndex deleteIndex = Model->index(deleteRow, 0);
    if (indexWidget(deleteIndex)) {
        closePersistentEditor(deleteIndex);
    }

    // 从缓存中移除该 editor（若启用了可视范围 editor 缓存）
    for (int i = m_openPersistentEditors.size() - 1; i >= 0; --i) {
        if (!m_openPersistentEditors[i].isValid() || m_openPersistentEditors[i] == deleteIndex) {
            m_openPersistentEditors.remove(i);
        }
    }

    Model->onDeleteTrack(deleteRow);
    selectionModel()->clearSelection();
    m_hoverIndex = QModelIndex();
    onUpdateViewport();
}
/**
 * @brief 上下文菜单事件
 */
void BaseTracklistView::contextMenuEvent(QContextMenuEvent* event) {
        QMenu contextMenu(this);
        //qDebug()<<"contextMenuEvent";
        // Add track creation submenu
        QMenu* addTrackMenu = contextMenu.addMenu("Add Track ");
        addTrackMenu->setIcon(QIcon(":/icons/icons/add.png"));
        // Get available track types from BasePluginLoader
        QStringList availableTypes = Model->getPluginLoader()->getAvailableTypes();
        for (const QString& type : availableTypes) {
            QAction* addTrackAction = addTrackMenu->addAction("Add " + type + " Track");
            connect(addTrackAction, &QAction::triggered, [this, type]() {
                Model->onAddTrack(type);
            });
        }

        // Add delete track option if a track is selected
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            contextMenu.addAction(m_deleteTrackAction);
        }

        contextMenu.exec(event->globalPos());
    }
    
/**
 * @brief 鼠标按下事件
 */
void BaseTracklistView::mousePressEvent(QMouseEvent *event) {
    /**
     * @brief 轨道列表鼠标按下
     * @details
     * - 这里只处理“轨道行”的选择。
     * - 选择状态由共享 selectionModel 承载；若 selectionModel 尚未注入，则直接走基类逻辑，避免空指针崩溃。
     */
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        if (selectionModel()) {
            QModelIndex item = indexAt(event->pos());
            if (item.isValid()) {
                selectionModel()->select(item, QItemSelectionModel::ClearAndSelect);
                selectionModel()->setCurrentIndex(item, QItemSelectionModel::Current);
            }
        }
        viewport()->update();
    }

    QAbstractItemView::mousePressEvent(event);
}
/**
 * @brief 获取某一点的轨道索引
 */
QModelIndex BaseTracklistView::indexAt(const QPoint &point) const {
    // Check if the point is above the ruler or in a blank area
    QRect rulerRect(0, 0, viewport()->width(), rulerHeight+toolbarHeight);
    if (point.y() < 0 || rulerRect.contains(point)) {
        return QModelIndex(); // Return an invalid index
    }

    // Iterate over the rows to find the index at the given point
    for (int i = 0; i < model()->rowCount(); ++i) {
        QModelIndex index = model()->index(i, 0);
        if (visualRect(index).contains(point)) {
            // qDebug() << "index"<<index.row()<<index.column()<<index.parent().row();
            return index; // Return the valid index if found
        }
    }

    // If no valid index is found, return an invalid QModelIndex
    return QModelIndex();
}

BaseTimeLineModel* BaseTracklistView::getModel()
{
    return Model;
}
/**
 * @brief 选择改变
 */
void BaseTracklistView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    QAbstractItemView::selectionChanged(selected, deselected);

    viewport()->update();
}

/**
 * @brief 鼠标移动事件
 */
void BaseTracklistView::mouseMoveEvent(QMouseEvent *event) {
    // 如果不是左键拖动，只更新悬停状态
    if (!(event->buttons() & Qt::LeftButton)) {
        const QModelIndex newHover = indexAt(event->pos());
        if (newHover != m_hoverIndex) {
            m_hoverIndex = newHover;
            viewport()->update();
        }
        return;
    }

    // 检查是否达到拖动启动距离，避免轻微抖动就触发拖拽
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // 获取拖动起始位置的轨道索引
    const QModelIndex index = indexAt(m_dragStartPosition);
    if (!index.isValid()) {
        return;
    }

    // 创建拖动对象
    auto* drag = new QDrag(this);
    auto* mimeData = new QMimeData;

    // 存储源轨道索引
    mimeData->setData("application/x-track-index", QByteArray::number(index.row()));
    drag->setMimeData(mimeData);

    // 创建预览图
    const QRect itemRect = visualRect(index);
    const int previewW = qMax(1, viewport()->width());
    const int previewH = qMax(1, itemRect.height());
    QPixmap pixmap(previewW, previewH);

    QColor fillColor = trackSelectedColour;
    fillColor.setAlphaF(0.8);
    pixmap.fill(fillColor);

    // 将热点设置为鼠标相对于轨道的位置
    const QPoint relativePos = m_dragStartPosition - itemRect.topLeft();
    drag->setPixmap(pixmap);
    drag->setHotSpot(relativePos);

    // 开始拖动操作
    drag->exec(Qt::MoveAction);

    // 防止一次按下过程中重复触发拖拽
    m_dragStartPosition = QPoint();
}

/**
 * @brief 离开事件
 */
void BaseTracklistView::leaveEvent(QEvent *event) {
    // selectionModel()->clear();
    if (m_hoverIndex.isValid()) {
        m_hoverIndex = QModelIndex();
        viewport()->update();
    }
    QAbstractItemView::leaveEvent(event);
}
/**
 * @brief 大小改变事件
 */
void BaseTracklistView::resizeEvent(QResizeEvent *event) {
    updateScrollBars();
    QAbstractItemView::resizeEvent(event);
}

/**
 * @brief 显示事件
 */
void BaseTracklistView::showEvent(QShowEvent *event) {
    /**
     * @brief 显示事件
     * @details
     * 兜底逻辑：
     * - 如果外部没有注入共享 selectionModel，则创建一个本地 selectionModel。
     * - 如果本视图未被 Controller 托管（独立使用），则在此处按需绑定“模型变化 -> 刷新”。
     */
    if (!selectionModel() && model()) {
        setSelectionModel(new QItemSelectionModel(model(), this));
    }

    // 独立使用场景：没有 Controller 时，TrackList 需要自己监听模型变化来刷新
    if (!property("qtimeline_managed").toBool() && !property("qtimeline_model_bound").toBool() && Model) {
        connect(Model, &QAbstractItemModel::modelReset,   this, &BaseTracklistView::onUpdateViewport);
        connect(Model, &QAbstractItemModel::rowsInserted, this, &BaseTracklistView::onUpdateViewport);
        connect(Model, &QAbstractItemModel::rowsRemoved,  this, &BaseTracklistView::onUpdateViewport);
        connect(Model, &QAbstractItemModel::rowsMoved,    this, &BaseTracklistView::onUpdateViewport);

        connect(Model, &QAbstractItemModel::dataChanged,
                this, [this](const QModelIndex& topLeft, const QModelIndex&, const QList<int>&) {
                    // Clip 层数据变化不影响 TrackList；Track 层变化轻量重绘即可
                    if (!topLeft.parent().isValid()) {
                        viewport()->update();
                    }
                });


        connect(Model, &BaseTimeLineModel::S_playheadMoved, this, [this](int) {
            viewport()->update();
        });

        setProperty("qtimeline_model_bound", true);
    }

    updateScrollBars();
    QAbstractItemView::showEvent(event);
}

/**
 * @brief 滚动事件
 */
void BaseTracklistView::scrollContentsBy(int dx, int dy) {
    /**
     * @brief QAbstractItemView 内部滚动回调
     * @details
     * 统一以滚动条 value 为唯一真值来源：
     * - 这里不再维护 m_scrollOffset。
     * - 通过 visualRect() 读取 scrollBar value 来计算偏移。
     */
    QAbstractItemView::scrollContentsBy(dx, dy);
    updateEditorGeometries();
    onUpdateViewport();
}

/**
 * @brief 滚动事件
 */


/**
 * @brief 更新视图
 */
void BaseTracklistView::onUpdateViewport(){
    /**
     * @brief 更新视图
     * @details
     * persistent editor（轨道行内的编辑控件）由本函数统一管理：
     * - 仅在可视范围（含少量缓冲行）内按需 openPersistentEditor。
     * - 对离开可视范围的行按需 closePersistentEditor，避免滚动过程中 editor 只增不减。
     */
    updateScrollBars();
    ensureVisiblePersistentEditors();
    viewport()->update();
}

void BaseTracklistView::ensureVisiblePersistentEditors()
{
    /**
     * @brief 确保可视范围内的轨道行存在 persistent editor
     * @details
     * 通过缓存 m_openPersistentEditors，避免每次更新遍历全部轨道。
     */
    if (!model() || !verticalScrollBar()) {
        return;
    }

    const int count = model()->rowCount();
    if (count <= 0) {
        m_openPersistentEditors.clear();
        return;
    }

    // 清理失效缓存
    for (int i = m_openPersistentEditors.size() - 1; i >= 0; --i) {
        if (!m_openPersistentEditors[i].isValid()) {
            m_openPersistentEditors.remove(i);
        }
    }

    const int y0 = verticalScrollBar()->value();
    const int contentTop = rulerHeight + toolbarHeight;
    const int visibleTop = qMax(0, y0 - contentTop);
    const int visibleBottom = qMax(0, y0 + viewport()->height() - contentTop);

    const int firstRow = qBound(0, visibleTop / trackHeight, count - 1);
    const int lastRow  = qBound(0, visibleBottom / trackHeight, count - 1);

    const int bufferRows = 1;
    const int desiredFirst = qMax(0, firstRow - bufferRows);
    const int desiredLast  = qMin(count - 1, lastRow + bufferRows);

    QVector<QPersistentModelIndex> desired;
    desired.reserve(desiredLast - desiredFirst + 1);
    for (int row = desiredFirst; row <= desiredLast; ++row) {
        desired.push_back(QPersistentModelIndex(model()->index(row, 0, QModelIndex())));
    }

    // 打开可视范围内缺失的 editor
    for (const QPersistentModelIndex& p : desired) {
        if (!p.isValid()) {
            continue;
        }

        bool alreadyTracked = false;
        for (const QPersistentModelIndex& opened : m_openPersistentEditors) {
            if (opened == p) {
                alreadyTracked = true;
                break;
            }
        }
        if (alreadyTracked) {
            continue;
        }

        if (!indexWidget(p)) {
            openPersistentEditor(p);
        }
        m_openPersistentEditors.push_back(p);
    }

    // 关闭离开可视范围的 editor
    for (int i = m_openPersistentEditors.size() - 1; i >= 0; --i) {
        const QPersistentModelIndex& p = m_openPersistentEditors[i];
        if (!p.isValid()) {
            m_openPersistentEditors.remove(i);
            continue;
        }

        bool shouldKeep = false;
        for (const QPersistentModelIndex& d : desired) {
            if (d == p) {
                shouldKeep = true;
                break;
            }
        }

        if (!shouldKeep) {
            if (indexWidget(p)) {
                closePersistentEditor(p);
            }
            m_openPersistentEditors.remove(i);
        }
    }
}

/**
 * @brief 更新滚动条
 */
void BaseTracklistView::updateScrollBars(){
    if(!model())
        return;
    int max =0;
    verticalScrollBar()->setRange(0,model()->rowCount() * trackHeight + rulerHeight+toolbarHeight - viewport()->height());
}

/**
 * @brief 获取视图矩形
 */
QRect BaseTracklistView::visualRect(const QModelIndex &index) const{
    /**
     * @brief 计算索引的可视矩形
     * @details
     * 以滚动条 value 为偏移来源，避免 m_scrollOffset 与滚动条不同步。
     */
    const QPoint offset(horizontalScrollBar()->value(), verticalScrollBar()->value());
    return itemRect(index).translated(-offset);
}
/**
 * @brief 获取项目矩形
 */
QRect BaseTracklistView::itemRect(const QModelIndex &index) const{
    if(!index.isValid())
        return QRect();
    return QRect(0, (index.row() * trackHeight) + rulerHeight+toolbarHeight, viewport()->width(), trackHeight);
};
/**
 * @brief 更新编辑几何
 */
void BaseTracklistView::updateEditorGeometries() {
    QAbstractItemView::updateEditorGeometries();

    for (int i = 0; i < model()->rowCount(); ++i){
        QModelIndex trackIndex = model()->index(i,0);

        QWidget *editor = indexWidget(trackIndex);
        if(!editor){
            continue;
        }

        QRect rect  = editor->rect();
        QPoint topInView = editor->mapToParent(rect.topLeft());
        if(topInView.y()<rulerHeight+toolbarHeight){
            int offset = rulerHeight+toolbarHeight-topInView.y();
            editor->setMask(QRegion(0,offset,editor->width(),rulerHeight+toolbarHeight));
        }

    }
}
/**
 * @brief 事件过滤
 */
bool BaseTracklistView::eventFilter(QObject *watched, QEvent *event)
{

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (watched == this) {
            // 处理 view1 的 keyPressEvent
            // 注意：可以添加其他逻辑来处理特定的键盘输入
            
            if(keyEvent->key()==Qt::Key_Delete){
                onDeleteTrack();
            }
            return true;  // 表示事件已处理，不再向其他控件传递
        }
    }
    return QObject::eventFilter(watched, event);
}
/**
 * @brief 设置选择
 */
void BaseTracklistView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {
    if (!model() || !selectionModel())
    return;

    // 将视图坐标转换为模型索引围
    QModelIndex topLeft = indexAt(rect.topLeft());
    QModelIndex bottomRight = indexAt(rect.bottomRight());

    if (!topLeft.isValid() || !bottomRight.isValid())
        return;

    // 创建选择范围
    QItemSelection selection(topLeft, bottomRight);

    // 更新选择模型
    selectionModel()->select(selection, command);
}
/**
 * @brief 绘制事件
 */
void BaseTracklistView::paintEvent(QPaintEvent *event) {
    /**
     * @brief 绘制轨道列表
     * @details
     * 这里做一个关键的“绑定简化”：
     * - 当 TimelineView 选中 clip 时，selectionModel 的 currentIndex 是“clipIndex（parent 有效）”。
     * - TrackListView 的行索引是“trackIndex（parent 无效）”，两者不是同一个 index。
     * - 旧实现只用 isSelected(trackIndex) 判断，导致“选中 clip 时左侧轨道不高亮”。
     * - 新实现：根据 currentIndex 推导出“当前轨道行号”，让视觉反馈一致，但不强行改变 selectionModel 的内容。
     */
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setBrush(QBrush(m_trackBgColor));

    const QItemSelectionModel* sel = selectionModel();
    const QModelIndex current = sel ? sel->currentIndex() : QModelIndex();
    int currentTrackRow = -1;
    if (current.isValid()) {
        currentTrackRow = current.parent().isValid() ? current.parent().row() : current.row();
    }

    for (int i = 0; i < model()->rowCount(); ++i){
        QModelIndex trackIndex = model()->index(i,0);

        const bool isCurrentTrack = (i == currentTrackRow);
        const bool isTrackSelected = sel ? sel->isSelected(trackIndex) : false;

        if (isCurrentTrack || isTrackSelected) {
            painter.setBrush(QBrush(m_trackSelectedColor));
        } else if (m_hoverIndex==trackIndex) {
            painter.setBrush(QBrush(m_trackHoverColor));
        } else {
            painter.setBrush(QBrush(m_trackColor));
        }

        painter.drawRect(visualRect(trackIndex));
    }

    drawTitle(&painter);

    painter.restore();
}

void BaseTracklistView::drawTitle(QPainter *painter)
{
    if (!painter) {
        return;
    }

    painter->save();

    painter->setBrush(QBrush(m_trackBgColor));
    QRect rect(0,  0, viewport()->width(), rulerHeight+toolbarHeight);
    painter->drawRect(rect);

    if (Model) {
        painter->setPen(fontColor);
        QFont font;
        font.setPixelSize(fontSize);
        painter->setFont(font);
        if(Model->getTimeDisplayFormat()==TimedisplayFormat::TimeCodeFormat){
            painter->drawText(rect,Qt::AlignCenter,QString("%1").arg(Model->getPlayheadPos()));
        }else{
            painter->drawText(rect,Qt::AlignCenter,QString("00:%1").arg(Model->getPlayheadPos()));
        }
    }

    painter->restore();
}

void BaseTracklistView::dragEnterEvent(QDragEnterEvent *event) {
    // 检查是否是我们的自定义MIME类型

    if (event->mimeData()->hasFormat("application/x-track-index")) {
        // 设置移动鼠标形状

        event->acceptProposedAction();

        // unsetCursor();
    } else {
        unsetCursor();
    }
}

void BaseTracklistView::dragMoveEvent(QDragMoveEvent *event) {
    if (event->mimeData()->hasFormat("application/x-track-index")) {
        event->acceptProposedAction();
        // 获取当前拖动位置的轨道索引
        QModelIndex dropIndex = indexAt(event->position().toPoint());
        if (dropIndex.isValid()) {
            // 只高亮显示目标位置，不进行选择
            m_hoverIndex = dropIndex;
            viewport()->update();
        } else {
            // 如果不在有效轨道上，设置禁止鼠标形状
            QAbstractItemView::dragMoveEvent(event);

        }
    }
}

void BaseTracklistView::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasFormat("application/x-track-index")) {
        // 获取源轨道和目标轨道的索引
        QByteArray data = event->mimeData()->data("application/x-track-index");
        int sourceRow = data.toInt();
        int targetRow = indexAt(event->position().toPoint()).row();

        // 只在源和目标不同且都有效时进行移动
        if (sourceRow != targetRow && targetRow >= 0 && sourceRow >= 0) {
            if (Model) {
                // 移动前关闭当前已打开的 persistent editor（数量应接近可视行数）
                for (const QPersistentModelIndex& p : m_openPersistentEditors) {
                    if (p.isValid() && indexWidget(p)) {
                        closePersistentEditor(p);
                    }
                }
                m_openPersistentEditors.clear();

                Model->onMoveTrack(sourceRow, targetRow);

                // 清除选择和悬停状态（selectionModel 可能由外部注入；兜底情况下也可能为空）
                if (selectionModel()) {
                    selectionModel()->clearSelection();
                }
                m_hoverIndex = QModelIndex();
                onUpdateViewport();
            }
        }

        event->acceptProposedAction();
    }else
    {
        QAbstractItemView::dropEvent(event);
    }
}
