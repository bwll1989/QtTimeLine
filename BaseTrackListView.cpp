#include "BaseTrackListView.h"
#include <QDrag>
#include <QMimeData>
#include <QMenu>
BaseTracklistView::BaseTracklistView(BaseTimeLineModel *viemModel, QWidget *parent): Model(viemModel), QAbstractItemView{parent}{
    setModel(Model);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionModel(new QItemSelectionModel(Model, this));
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
    m_scrollOffset = QPoint(0,0);
    
    // Create delete track action
    m_deleteTrackAction = new QAction("Delete track", this);
    m_deleteTrackAction->setShortcut(QKeySequence(Qt::Key_Delete));
    QObject::connect(m_deleteTrackAction, &QAction::triggered, this, &BaseTracklistView::onDeleteTrack);
    //轨道变化后刷新显示
    QObject::connect(Model, &BaseTimeLineModel::S_trackAdd, this, &BaseTracklistView::onUpdateViewport);
    QObject::connect(Model, &BaseTimeLineModel::S_trackDelete, this, &BaseTracklistView::onUpdateViewport);
    QObject::connect(Model, &BaseTimeLineModel::S_trackMoved, this, &BaseTracklistView::onUpdateViewport);
    QObject::connect(Model, &BaseTimeLineModel::S_playheadMoved, this, &BaseTracklistView::onUpdateViewport);
    setItemDelegate(new BaseTrackDelegate(this));
    setMouseTracking(true);
    m_scrollOffset = QPoint(0,0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded ) ;
    setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded) ;
    installEventFilter(this);
}
/**
 * @brief 删除轨道
 */
void BaseTracklistView::onDeleteTrack() {
    if (Model&&selectionModel()->selectedIndexes().size()>=1) {
        // 获取要删除的行号
        int deleteRow = selectionModel()->selectedIndexes().first().row();
        
        // 关闭要删除的编辑器
        QModelIndex deleteIndex = Model->index(deleteRow, 0);
        closePersistentEditor(deleteIndex);
        Model->onDeleteTrack(deleteRow);

        // 只需要重新打开删除行之后的编辑器
        for (int i = deleteRow; i < Model->rowCount(); ++i) {
            QModelIndex index = Model->index(i, 0);
            openPersistentEditor(index);
        }
    }
}
/**
 * @brief 上下文菜单事件
 */
void BaseTracklistView::contextMenuEvent(QContextMenuEvent* event) {
        QMenu contextMenu(this);
        //qDebug()<<"contextMenuEvent";
        // Add track creation submenu
        QMenu* addTrackMenu = contextMenu.addMenu("Add Track      ");
        
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
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        QModelIndex item = indexAt(event->pos());
        if (item.isValid()) {
            selectionModel()->select(item, QItemSelectionModel::ClearAndSelect);
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
        m_hoverIndex = indexAt(event->pos());
        viewport()->update();
        return;
    }
    // // 检查是否达到拖动启动距离
    // if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
    //     return;
    // }
    // 获取拖动起始位置的轨道索引
    QModelIndex index = indexAt(m_dragStartPosition);
    if (!index.isValid()) {
        return;
    }

    // 创建拖动对象
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    
    // 存储源轨道索引
    mimeData->setData("application/x-track-index", QByteArray::number(index.row()));
    drag->setMimeData(mimeData);
    // 创建预览图
    QRect itemRect = visualRect(index);
    QPixmap pixmap(parentWidget()->width(), itemRect.size().height());
    QColor fillColor = trackSelectedColour;
    fillColor.setAlphaF(0.8); // 设置50%透明度
    pixmap.fill(fillColor);
    // 计算鼠标相对于轨道项的位置
    QPoint relativePos = m_dragStartPosition - itemRect.topLeft();
    // 设置拖动时的预览图
    drag->setPixmap(pixmap);
    // 将热点设置为鼠标相对于轨道的位置
    drag->setHotSpot(relativePos);
    // 开始拖动操作
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

/**
 * @brief 离开事件
 */
void BaseTracklistView::leaveEvent(QEvent *event) {
    // selectionModel()->clear();
    m_hoverIndex = QModelIndex();
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
    updateScrollBars();
    QAbstractItemView::showEvent(event);
}

/**
 * @brief 滚动事件
 */
void BaseTracklistView::scrollContentsBy(int dx, int dy) {
    m_scrollOffset -= QPoint(dx, dy);
    QAbstractItemView::scrollContentsBy(dx, dy);
    updateEditorGeometries();
    onUpdateViewport();
    //qDebug()<<"scrollContentsBy";
    emit trackScrolled(dx, dy);
}

/**
 * @brief 滚动事件
 */
void BaseTracklistView::scroll(int dx, int dy){
    m_scrollOffset -= QPoint(0, dy);
    updateEditorGeometries();
    QAbstractItemView::scrollContentsBy(dx, dy);
}

/**
 * @brief 更新视图
 */
void BaseTracklistView::onUpdateViewport(){
    updateScrollBars();
    viewport()->update();
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
    return itemRect(index).translated(-m_scrollOffset);
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
    QPainter painter(viewport());
    painter.save();  // 保存状态
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setBrush(QBrush(bgColour));

    // Draw tracklist
    int viewportWidth = viewport()->width();
    QStyleOptionViewItem option;
    QAbstractItemView::initViewItemOption(&option);
    
    for (int i = 0; i < model()->rowCount(); ++i){
        QModelIndex trackIndex = model()->index(i,0);
        openPersistentEditor(model()->index(i,0));
        if(selectionModel()->isSelected(trackIndex)){
           painter.setBrush(QBrush(trackSelectedColour));
        }else
        if(m_hoverIndex==trackIndex && !selectionModel()->isSelected(trackIndex)){
            painter.setBrush(QBrush(trackHoverColour));
        }
        else{
            painter.setBrush(QBrush(trackColour));
        }
        // painter.drawLine(visualRect(trackIndex).bottomLeft(),visualRect(trackIndex).bottomRight());
        // itemDelegate(trackIndex)->paint(&painter, option, trackIndex);
       
        painter.drawRect(visualRect(trackIndex));
      
    }

    drawTitle(&painter);
}

void BaseTracklistView::drawTitle(QPainter *painter)
{
    painter->setBrush(QBrush(bgColour));
    QRect rect(0,  0, viewport()->width(), rulerHeight+toolbarHeight);
    painter->drawRect(rect);
    painter->setPen(fontColor);
    QFont font;
    font.setPixelSize(fontSize);
    painter->setFont(font);
    if(Model->getTimeDisplayFormat()==TimedisplayFormat::TimeCodeFormat){
        painter->drawText(rect,Qt::AlignCenter,QString("%1").arg(Model->getPlayheadPos()));
    }else{
        painter->drawText(rect,Qt::AlignCenter,QString("00:%1").arg(Model->getPlayheadPos()));
    }
    painter->restore();  // 恢复状态
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
                for (int i = 0; i < Model->rowCount(); ++i) {
                    // 关闭所有编辑器
                    QModelIndex index = Model->index(i, 0);
                    closePersistentEditor(index);
                }

                Model->onMoveTrack(sourceRow, targetRow);
                // 清除选择和悬停状态
                selectionModel()->clearSelection();
                m_hoverIndex = QModelIndex();
                viewport()->update();
                // 发送信号以通知tracklist和timelineview更新
                emit viewUpdate();  // 发送信号
            }
        }

        event->acceptProposedAction();
    }else
    {
        QAbstractItemView::dropEvent(event);
    }
}