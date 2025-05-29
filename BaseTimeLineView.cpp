#include "BaseTimeLineView.h"
#include <QMenu>
#include <QAction>
#include <QtMath>
BaseTimelineView::BaseTimelineView(BaseTimeLineModel *viewModel, QWidget *parent)
        : Model(viewModel), QAbstractItemView{parent}
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt:: ScrollBarAlwaysOff);
        setModel(Model);
        horizontalScrollBar()->setSingleStep(10);
        horizontalScrollBar()->setPageStep(100);
        verticalScrollBar()->setSingleStep(trackHeight);
        verticalScrollBar()->setPageStep(trackHeight * 5);
        viewport()->setMinimumHeight(trackHeight + rulerHeight + toolbarHeight);
        setMinimumHeight(trackHeight + rulerHeight + toolbarHeight);
        QItemSelectionModel* selModel = new QItemSelectionModel(Model, this);
        setSelectionModel(selModel);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectItems);
        setMouseTracking(true);
        // 确保有选择模型
        setEditTriggers(QAbstractItemView::AllEditTriggers);
        setAutoScroll(true);
        setAutoScrollMargin(5);
        setAcceptDrops(true);

        // 创建工具栏
        toolbar = new BaseTimelineToolbar(this);
        toolbar->setFixedHeight(toolbarHeight);
        // 设置工具栏位置
        toolbar->move(0, 0);
        
        // 连接工具栏播放按钮信号
        connect(toolbar, &BaseTimelineToolbar::playClicked, [this]() {
            Model->onStartPlay();
        });
        // 连接工具栏停止按钮信号
        connect(toolbar, &BaseTimelineToolbar::stopClicked, [this]() {
            Model->onStopPlay();
        });
        // 连接工具栏暂停按钮信号
        connect(toolbar, &BaseTimelineToolbar::pauseClicked, [this]() {
            Model->onPausePlay();
        });
        connect(toolbar,&BaseTimelineToolbar::prevFrameClicked,[this](){
           Model->onSetPlayheadPos(qMax(0,Model->getPlayheadPos()-1));
        });
        connect(toolbar,&BaseTimelineToolbar::nextFrameClicked,[this](){
            Model->onSetPlayheadPos(Model->getPlayheadPos()+1);
        });
        // 当视频窗口关闭时，更新工具栏按钮状态
        connect(Model,&BaseTimeLineModel::S_clipGeometryChanged,this,&BaseTimelineView::onUpdateViewport);
        connect(Model, &BaseTimeLineModel::S_trackAdd, this, &BaseTimelineView::onUpdateViewport);
        connect(Model, &BaseTimeLineModel::S_trackDelete, this, &BaseTimelineView::onUpdateViewport);
        connect(Model, &BaseTimeLineModel::S_trackMoved, this, &BaseTimelineView::onUpdateViewport);
        connect(Model, &BaseTimeLineModel::S_addClip, this, &BaseTimelineView::onUpdateViewport);
        connect(Model, &BaseTimeLineModel::S_playheadMoved, this, &BaseTimelineView::onUpdateViewport);

        installEventFilter(this);



        // 连接移动剪辑按钮信号
        connect(toolbar, &BaseTimelineToolbar::moveClipClicked, [this](int dx) {
            moveSelectedClip(dx,0,false);
        });
        // 连接删除剪辑按钮信号
        connect(toolbar, &BaseTimelineToolbar::deleteClipClicked, [this]() {
            if(selectionModel()->selectedIndexes().isEmpty())
                return;
            
            QModelIndex currentIndex = selectionModel()->currentIndex();
            Model->onDeleteClip(currentIndex);
            
            // 清除选择并发送 nullptr
            selectionModel()->clearSelection();
            emit currentClipChanged(nullptr);
            
            viewport()->update();
        });

        // 连接放大按钮信号
        connect(toolbar, &BaseTimelineToolbar::zoomInClicked, [this]() {
       
           setScale(1.1);
        });
        // 连接缩小按钮信号
        connect(toolbar, &BaseTimelineToolbar::zoomOutClicked, [this]() {
            setScale(0.9);
        });
        // 连接循环播放信号

    }

BaseTimelineView::~BaseTimelineView()
{
    delete toolbar;
    // delete timer;

}
QRect BaseTimelineView::visualRect(const QModelIndex &index) const
{
    
    return itemRect(index).translated(-m_scrollOffset);
}

 QModelIndex BaseTimelineView::indexAt(const QPoint &point) const
    {
        //检查位置是否在工具栏区域
        if (point.y() < rulerHeight+toolbarHeight) {
            return QModelIndex(); // Return an invalid index if the point is in the ruler area
        }

        //遍历每个轨道，匹配
        for (int i = 0; i < model()->rowCount(); ++i) {
            QModelIndex trackIndex = model()->index(i, 0);
            QRect trackRect = visualRect(trackIndex);

            //检查位置是否在轨道矩形内
            if (trackRect.contains(point)) {
                //遍历每个剪辑，匹配位置是否在剪辑矩形内
                for (int j = 0; j < model()->rowCount(trackIndex); ++j) {
                    QModelIndex clipIndex = model()->index(j, 0, trackIndex);
                    QRect clipRect = visualRect(clipIndex);

                    //检查位置是否在剪辑矩形内
                    if (clipRect.contains(point)) {
                        //如果位置在剪辑矩形内，返回剪辑索引
                        return clipIndex;
                    }
                }
                //如果位置在轨道矩形内，但不在剪辑矩形内，返回轨道索引
                return trackIndex; 
            }
        }
        //如果位置不在任何轨道或剪辑矩形内，返回无效索引
        return QModelIndex(); 
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
        return QRect(0, (index.row() * trackHeight) + rulerHeight+toolbarHeight, trackwidth, trackHeight);
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
            int clipStartY = (trackRow*trackHeight + rulerHeight+toolbarHeight);
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
    Model->setData(list.first(), newPos, TimelineRoles::ClipPosRole);
    updateEditorGeometries();
    updateScrollBars();
    viewport()->update();
}

void BaseTimelineView::movePlayheadToFrame(int frame)
{
    //  qDebug() << "movePlayheadToFrame" << frame;
    // 直接设置时间码生成器的帧位置
    Model->onSetPlayheadPos(frame);
    viewport()->update();
}

void BaseTimelineView::onUpdateViewport(){
    updateEditorGeometries();
    updateScrollBars();
    viewport()->update();
}

// 滚动视图
void BaseTimelineView::onScroll(int dx, int dy){
    
    m_scrollOffset -= QPoint(dx, dy);
    QAbstractItemView::scrollContentsBy(dx, dy);
    updateEditorGeometries();
    updateScrollBars();
    viewport()->update();
}
void BaseTimelineView::horizontalScroll(double position)
{
    // position 是 0-1 之间的值，表示滚动条的相对位置
    int maxScroll = horizontalScrollBar()->maximum();
    int newScrollValue = static_cast<int>(maxScroll * position);
    
    // 设置滚动条位置
    horizontalScrollBar()->setValue(newScrollValue);
    
    // 更新编辑器位置
    updateEditorGeometries();
    viewport()->update();
}
// 更新滚动条
void BaseTimelineView::updateScrollBars()
{
    if (!model())
        return;
    int max = 0;
    max = getTrackWdith() -  viewport()->width();
    horizontalScrollBar()->setRange(0, max);
    verticalScrollBar()->setRange(0, model()->rowCount() * trackHeight + rulerHeight - viewport()->height());
}

void BaseTimelineView::scrollContentsBy(int dx, int dy)
{
    m_scrollOffset -= QPoint(dx, dy);
    QAbstractItemView::scrollContentsBy(dx, dy);
    updateEditorGeometries();
}

int BaseTimelineView::pointToFrame(int point) const
{
    return point/timescale ;
}

int BaseTimelineView::frameToPoint(int frame) const
{
    return frame*timescale;
}

void BaseTimelineView::contextMenuEvent(QContextMenuEvent* event) {
    QModelIndex index = indexAt(event->pos());

    // Check if the index is a track (i.e., it has no parent)
    if (index.isValid() && !index.parent().isValid()) {
        QMenu contextMenu(this);

        QAction* addClipAction = new QAction("Add Item", this);
        connect(addClipAction, &QAction::triggered, [this, index, event]() {
            addClipAtPosition(index, event->pos());
        });

        contextMenu.addAction(addClipAction);
        contextMenu.exec(event->globalPos());
    }
    if(index.isValid()&&selectionModel()->isSelected(index)){
   
        QMenu contextMenu(this);

        QAction* deleteClipAction = new QAction("Delete Clip", this);
        connect(deleteClipAction, &QAction::triggered, [this, index]() {
            Model->onDeleteClip(index);
            selectionModel()->clearSelection();
            viewport()->update();
        });

        contextMenu.addAction(deleteClipAction);
        contextMenu.exec(event->globalPos());
        QAbstractItemView::contextMenuEvent(event);
    }
}
//void BaseTimelineView::keyPressEvent(QKeyEvent *event)
//{
//    if(selectionModel()->selectedIndexes().isEmpty())
//        return;
//    if (event->key() == Qt::Key_Delete) {
//        // 处理删除键
//
//        QModelIndex currentIndex = selectionModel()->currentIndex();
//        Model->onDeleteClip(currentIndex);
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
    if (trackIndex < 0 || trackIndex >= Model->getTrackCount()) {
        qDebug() << "Invalid track index: " << trackIndex;
        return;
    }
    // Calculate the start frame based on the mouse position
    int startFrame = pointToFrame(pos.x() + m_scrollOffset.x());
    
    Model->onAddClip(trackIndex,startFrame);

    // 更新视图
    onUpdateViewport();
}

void BaseTimelineView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // 处理左键按下的情况
        QAbstractItemView::mousePressEvent(event);
        return;
    }
    mouseHeld = false;
    m_playheadSelected = false;
    m_mouseEnd = event->pos();

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
    mouseHeld = false;
    // selectionModel()->clear();
    m_hoverIndex = QModelIndex();
    QAbstractItemView::leaveEvent(event);
}

// 设置缩放比例
void BaseTimelineView::setScale(double value)
{

    // 保存当前焦点帧位置
    int focusFrame = Model->getPlayheadPos();
    
    // 计算旧焦点在点坐标系中的位置
    int oldPointFocus = frameToPoint(focusFrame);
    // 设置新的缩放比例 (value 范围是 0-1)
    timescale = (value * 99 + 1) * baseTimeScale / 100;  // 将 0-1 映射到 5%-100% 的缩放范围
//     timescale = value * baseTimeScale;
    timescale=qMax(1,timescale);

    int newPointFocus = frameToPoint(focusFrame);
    
    // 计算位移差异以保持焦点位置不变
    int diff = newPointFocus - oldPointFocus;
    
    // 调整滚动偏移以保持焦点位置
    if (m_scrollOffset.x() + diff >= 0) {
        scrollContentsBy(-diff, 0);
    } else {
        m_scrollOffset.setX(0);
    }
    
    // 更新界面
    updateEditorGeometries();
    updateScrollBars();
    viewport()->update();
}

void BaseTimelineView::resizeEvent(QResizeEvent *event)
{
    updateScrollBars();
    updateEditorGeometries();
    QAbstractItemView::resizeEvent(event);
    
    // 更新ZoomController
    // emit timelineInfoChanged(getTrackWdith(), viewport()->width(), m_scrollOffset.x());
}

void BaseTimelineView::showEvent(QShowEvent *event)
{
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

    // 清除当前选择
    selectionModel()->clearSelection();
    
    // 获取播放头位置   
    int playheadPos = frameToPoint(((BaseTimeLineModel*)model())->getPlayheadPos());
    // 获取播放头矩形
    QRect playheadHitBox(QPoint(playheadPos-3,rulerHeight),QPoint(playheadPos+2,viewport()->height()));
    // 获取播放头矩形2
    QRect playheadHitBox2(QPoint(playheadPos-playheadwidth,-playheadheight + rulerHeight),QPoint(playheadPos+playheadwidth,rulerHeight));
    // 如果点击在播放头上
    if(playheadHitBox.contains(m_mouseStart) || playheadHitBox2.contains(m_mouseStart)) {
        m_playheadSelected = true;
        return;
    }
    
    m_playheadSelected = false;
    
    // 获取点击位置的项
    QModelIndex item = indexAt(event->pos());
    
    // 如果点击到了片段
    if(item.isValid() && item.parent().isValid()) {
        // 设置当前项和选择
        selectionModel()->setCurrentIndex(item, QItemSelectionModel::ClearAndSelect);
        m_mouseOffset.setX(frameToPoint(item.data(TimelineRoles::ClipInRole).toInt()) - m_mouseStart.x());
    } 
    // else {
    //     // 点击到空白区域或轨道
    //     selectionModel()->clearSelection();
    //     movePlayheadToFrame(pointToFrame(std::max(0, m_mouseEnd.x() + m_scrollOffset.x())));
    // }
    
    viewport()->update();
}

void BaseTimelineView::mouseMoveEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::RightButton) {
            // 处理左键按下的情况
            QAbstractItemView::mouseMoveEvent(event);
            return;
        }

        if(mouseHeld){
//            如果鼠标按住拖动
            m_mouseEnd = event->pos();
            if(!selectionModel()->selectedIndexes().isEmpty()&&m_mouseEnd.x()>=0){
                QModelIndex clipIndex = selectionModel()->selectedIndexes().first();
                AbstractClipModel* clip = static_cast<AbstractClipModel*>(clipIndex.internalPointer());
                
                if(m_mouseUnderClipEdge==hoverState::NONE){
                    moveSelectedClip(pointToFrame(m_mouseEnd.x()+m_mouseOffset.x()),m_mouseEnd.y()+m_mouseOffset.y());
                }else if(clip && clip->isResizable()){
                    if(m_mouseUnderClipEdge==hoverState::LEFT){
                        int newFrame = pointToFrame(m_mouseEnd.x() + m_scrollOffset.x());
                        Model->setData(clipIndex, newFrame, TimelineRoles::ClipInRole);
                        // Model->onTimelineLengthChanged();
                        updateEditorGeometries();
                    }else if(m_mouseUnderClipEdge==hoverState::RIGHT){
                        int newFrame = pointToFrame(m_mouseEnd.x() + m_scrollOffset.x());
                        Model->setData(clipIndex, newFrame, TimelineRoles::ClipOutRole);
                        // Model->onTimelineLengthChanged();
                        updateEditorGeometries();
                    }
                }
                viewport()->update();
            }
            else{
                movePlayheadToFrame(pointToFrame(std::max(0,m_mouseEnd.x() + m_scrollOffset.x())));
                viewport()->update();
            }

            if(m_playheadSelected){
                movePlayheadToFrame(pointToFrame(std::max(0,m_mouseEnd.x() + m_scrollOffset.x())));
                viewport()->update();
            }
            return QAbstractItemView::mouseMoveEvent(event);
       }

        QPoint pos = event->pos();
        m_hoverIndex = indexAt(event->pos());
        QRect rect = visualRect(m_hoverIndex);
        m_mouseUnderClipEdge = hoverState::NONE;
       //5 is hitbox size + -5px
       //see if item is a clip
        if((m_hoverIndex.isValid() && m_hoverIndex.parent().isValid())){
            AbstractClipModel* clip = static_cast<AbstractClipModel*>(m_hoverIndex.internalPointer());
            if(clip && clip->isResizable()){
                if(abs(pos.x() - rect.left())<=5){
                    m_mouseUnderClipEdge=hoverState::LEFT;
                }else if(abs(pos.x() - rect.right())<=5){
                    m_mouseUnderClipEdge=hoverState::RIGHT;
                }
            }
       }
        if (m_mouseUnderClipEdge != hoverState::NONE) {
           setCursor(Qt::SizeHorCursor);
        }else {
           unsetCursor();
        }

       QAbstractItemView::mouseMoveEvent(event);
    }

void BaseTimelineView::dragEnterEvent(QDragEnterEvent *event)
{


}

void BaseTimelineView::dragMoveEvent(QDragMoveEvent *event)
{
    if(m_isDroppingMedia){
        m_lastDragPos = event->position().toPoint();
        event->acceptProposedAction();
    }else{
        QAbstractItemView::dragMoveEvent(event);
    }
}

void BaseTimelineView::dropEvent(QDropEvent *event)
{
    if(m_isDroppingMedia){
        m_lastDragPos = event->position().toPoint();
        QModelIndex trackIndex;
        BaseTimeLineModel* timelineModel = ((BaseTimeLineModel*)model());
        QRect rullerRect(-m_scrollOffset.x(),0,viewport()->width() + m_scrollOffset.x(),rulerHeight);
        /* If above or on the ruler drop on the first track*/
        if(m_lastDragPos.y()<0 || rullerRect.contains(m_lastDragPos)){
            if(timelineModel->rowCount()>0)
                trackIndex = model()->index(0, 0);
        }else{
            /* Find track at drop point */
            for(int i = 0; i < timelineModel->rowCount(); i++){
                if (visualRect(timelineModel->index(i, 0)).contains(m_lastDragPos)){
                    trackIndex = timelineModel->index(i,0);
                }
            }
        }
        /* If dropped out side of tracks */
        if(!trackIndex.isValid()){
            trackIndex =QModelIndex();
        }
        int pos = pointToFrame(m_lastDragPos.x());


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
                      Model->onDeleteClip(currentIndex);
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
    QPainter painter(viewport());
    painter.save();

    // 1. 绘制基本背景和轨道分隔线
    drawBackground(&painter, event->rect());
    
    // 2. 绘制垂直时间线
    drawVerticalTimeLines(&painter, event->rect());
    
    // 3. 绘制轨道内容(clips)
    drawTracks(&painter);
    
    // 4. 绘制时间标尺
    drawTimeRuler(&painter, event->rect());
    
    // 5. 绘制播放头
    drawPlayhead(&painter);

    painter.restore();
}

void BaseTimelineView::drawBackground(QPainter* painter, const QRect& rect)
{
    // 绘制背景
    painter->fillRect(rect, bgColour);
    
    // 设置分割线颜色并绘制轨道分隔线
    painter->setPen(seperatorColour);
    for (int i = 0; i < model()->rowCount(); ++i) {
        int trackSplitter = (i+1) * trackHeight + rulerHeight + toolbarHeight - m_scrollOffset.y();
        painter->drawLine(0, trackSplitter, rect.width(), trackSplitter);
    }
}

void BaseTimelineView::drawVerticalTimeLines(QPainter* painter, const QRect& rect)
{
    int lineheight = model()->rowCount() * trackHeight + rulerHeight + toolbarHeight;
    
    // 计算时间线间隔
    double frameRate = 25.0;
    int frameStep = calculateFrameStep(frameRate);

    // 计算起始和结束位置
    int startMarker = static_cast<int>(pointToFrame(m_scrollOffset.x())) * timescale + 1;
    int endMarker = rect.width() + m_scrollOffset.x();
    startMarker = pointToFrame(startMarker);
    startMarker -= (startMarker % frameStep);
    startMarker = frameToPoint(startMarker);

    // 绘制垂直时间线
    for (int i = startMarker; i < endMarker; i += timescale * frameStep) {
        painter->drawLine(i - m_scrollOffset.x(), 
                         std::max(rulerHeight, rect.top()),
                         i - m_scrollOffset.x(), 
                         lineheight);
    }
}

int BaseTimelineView::calculateFrameStep(double frameRate) const
{
    if (timescale >= frameRate) {
        return 10;  // 每帧都显示时间码
    } else if (timescale >= frameRate / 2) {
        return 20;  // 每2帧显示一次
    } else if (timescale >= frameRate / 5) {
        return 50;  // 每5帧显示一次
    } else if (timescale >= frameRate / 10) {
        return 100; // 每10帧显示一次
    } else if (timescale >= frameRate / 25) {
        return 25; // 每25帧显示一次
    } else {
        return qRound(frameRate); // 每秒显示一次
    }
}

void BaseTimelineView::drawTimeRuler(QPainter* painter, const QRect& rect)
{
    // 绘制标尺背景
    painter->setPen(rulerColour);
    painter->setBrush(QBrush(bgColour));
    painter->drawRect(-m_scrollOffset.x(), 0, 
                     rect.width() + m_scrollOffset.x(), 
                     rulerHeight + toolbarHeight);

    // 计算时间线间隔
    double frameRate = 25.0;
    int frameStep = calculateFrameStep(frameRate);

    // 计算起始和结束位置
    int startMarker = static_cast<int>(pointToFrame(m_scrollOffset.x())) * timescale + 1;
    int endMarker = rect.width() + m_scrollOffset.x();
    startMarker = pointToFrame(startMarker);
    startMarker -= (startMarker % frameStep);
    startMarker = frameToPoint(startMarker);

    // 绘制时间标记和文本
    drawTimeMarkers(painter, startMarker, endMarker, frameStep);
}

void BaseTimelineView::drawTimeMarkers(QPainter* painter, int startMarker, int endMarker, int frameStep)
{
    for (int i = startMarker; i < endMarker; i += timescale * frameStep) {
        int number = pointToFrame(i);
        QString text = tr("%1").arg(number);
        QRect textRect = painter->fontMetrics().boundingRect(text);

        textRect.translate(-m_scrollOffset.x(), 0);
        textRect.translate(i - textRect.width() / 2, rulerHeight + toolbarHeight - textoffset);

        painter->drawLine(i - m_scrollOffset.x(), textRect.bottom(),
                         i - m_scrollOffset.x(), rulerHeight + toolbarHeight);

        painter->drawText(textRect, text);
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

    int playheadPos = frameToPoint(Model->getPlayheadPos()) - m_scrollOffset.x();
    
    // 移动播放头到正确位置
    for (QPoint &p : kite) {
        p.setX(p.x() + playheadPos);
        p.setY(p.y() + rulerHeight + toolbarHeight);
    }

    // 设置画笔和画刷颜色并绘制
    painter->setPen(playheadColour);
    painter->setBrush(playheadColour);
    painter->drawConvexPolygon(kite, 5);
    painter->drawLine(QPoint(playheadPos, rulerHeight + toolbarHeight),
                     QPoint(playheadPos, viewport()->height()));
}

void BaseTimelineView::drawTracks(QPainter* painter)
{
    for (int i = 0; i < model()->rowCount(); ++i) {
        QModelIndex trackIndex = model()->index(i, 0);
       
        for (int j = 0; j < model()->rowCount(trackIndex); ++j) {

            QModelIndex clipIndex = model()->index(j, 0, trackIndex);
           // 设置画笔和画刷
           AbstractClipModel* clip = clipIndex.data(TimelineRoles::ClipModelRole).value<AbstractClipModel*>();
           if (clip) {
            // 计算片段的绘制区域
            QRect clipRect = visualRect(clipIndex).adjusted(0, clipoffset, 0, -clipoffset);
            // 判断是否选中
            bool selected = selectionModel()->isSelected(clipIndex);
            // 调用片段的绘制方法
            clip->paint(painter, clipRect, selected);
        }
            // 使用圆角矩形绘制clip
            // if(clipIndex.data(TimelineRoles::ClipShowBorderRole).toBool()){
            //     painter->setPen(QPen(ClipBorderColour, ClipBorderWidth));
            //     QRect clipRect = visualRect(clipIndex).adjusted(0, clipoffset, 0, -clipoffset);
            //     painter->drawRoundedRect(clipRect, clipround, clipround);  // 设置水平和垂直圆角半径为5像素
            // }
            // if (!indexWidget(clipIndex)&&clipIndex.data(TimelineRoles::ClipShowWidgetRole).toBool()) {
                
            //     openPersistentEditor(clipIndex);
            // }
            // else{
            //     // closePersistentEditor(clipIndex);
            //     itemDelegateForIndex(clipIndex)->paint(painter, option, clipIndex);
            // }
           
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
    
    viewport()->update();
}

void BaseTimelineView::wheelEvent(QWheelEvent *event){
        if (event->modifiers() & Qt::ControlModifier) {
            // 缩放操作，以鼠标位置为中心
            QPoint pos = event->position().toPoint();
            double delta = event->angleDelta().y() / 120.0; // 标准滚轮步长
            double scaleChange = (delta > 0 ? 1.1 : 0.9);
            currentScale = currentScale*scaleChange;
            currentScale=qBound(0.1,currentScale,1.0);
            setScale(currentScale);
        } else {
            // 水平滚动
            QPoint numPixels = event->angleDelta();
            int dx = -numPixels.y() / 2; // 减小滚动速度
            
            // 计算新的滚动位置
            int newX = m_scrollOffset.x() + dx;
            
            // 限制滚动范围
            int maxScroll = qMax(0, getTrackWdith() - viewport()->width());
            newX = qBound(0, newX, maxScroll);
            
            // 应用新的滚动位置
            if (newX != m_scrollOffset.x()) {
                m_scrollOffset.setX(newX);
                updateEditorGeometries();
                viewport()->update();
                emit scrolled(dx, 0);
            }
        }
        event->accept();
    }

void BaseTimelineView::onFrameChanged(qint64 frame)
{
    // qDebug() << "Frame changed:" << frame;
    // 不再调用 movePlayheadToFrame，直接更新视图
    viewport()->update();
}

void BaseTimelineView::onPlaybackStateChanged(bool isPlaying)
{
    toolbar->setPlaybackState(isPlaying);
    
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
