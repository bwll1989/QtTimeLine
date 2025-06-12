#ifndef BASETIMELINEVIEW_H
#define BASETIMELINEVIEW_H

#include <QAbstractItemView>
#include <unordered_map>
#include "TimeLineDefines.h"
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QFileInfo>
#include <QMimeData>
#include "BaseTimeLineModel.h"
#include "TimeLineStyle.h"
#include "BaseTrackDelegate.h"
#include "BaseTimeLineToolBar.h"
#include "QMenu"
#include "Export.hpp"
// TimelineView类继承自QAbstractItemView
class NODE_TIMELINE_PUBLIC BaseTimelineView : public QAbstractItemView
{
Q_OBJECT
public:
    // 构造函数，初始化视图模型
    explicit BaseTimelineView(BaseTimeLineModel *viewModel, QWidget *parent = nullptr);
    ~BaseTimelineView() override ;
    /**
     * 返回给定索引的可视矩形
     * @param QModelIndex index 索引
     * @return QRect 可视矩形
     */
    virtual QRect visualRect(const QModelIndex &index) const override;
    /**
     * 滚动到指定索引
     * @param QModelIndex index 索引
     * @param ScrollHint hint 滚动提示
     */
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint) override{};

    /**
     * 返回给定点的索引
     * @param QPoint point 点
     * @return QModelIndex 索引
     */
    virtual QModelIndex indexAt(const QPoint &point) const override;
    /**
     * 显示剪辑属性
     * @param QModelIndex index 片段索引
     */
    virtual void showClipProperty(const QModelIndex& index);
    // 时间线工具栏
    BaseTimelineToolbar* toolbar;
    // 时间线定时器
signals:
    /**
     * 滚动信号
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    void scrolled(int dx,int dy);
    /**
     * 当前剪辑改变信号
     * @param AbstractClipModel* clip 当前剪辑
     */
    void currentClipChanged(AbstractClipModel* clip);
public slots:
    /**
     * 更新可视区域
     */
    virtual void onUpdateViewport();
    /**
     * 滚动视图
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    virtual void onScroll(int dx, int dy);
    /**
     * 水平滚动
     * @param double dx 水平滚动
     */
    virtual void horizontalScroll(double dx);
    /**
     * 更新滚动条
     */
    virtual void updateScrollBars();
    /**
     * 滚动内容
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    virtual void scrollContentsBy(int dx, int dy) override;
    /**
     * 设置缩放
     * @param double value 缩放值
     */
    virtual void setScale(double value);

    /**
     * 在指定位置添加剪辑
     * @param QModelIndex index 索引
     * @param QPoint pos 位置
     */
    virtual void addClipAtPosition(const QModelIndex& index, const QPoint& pos);

protected:
    /**
     * 移动光标
     * @param CursorAction cursorAction 光标动作
     * @param Qt::KeyboardModifiers modifiers 键盘修饰符
     * @return QModelIndex 索引
     */
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override {return QModelIndex();}

    /**
     * 水平偏移
     * @return int 水平偏移
     */
    virtual int horizontalOffset() const override{return 0;}

    /**
     * 垂直偏移
     * @return int 垂直偏移
     */
    virtual int verticalOffset() const override{return 0;}

    /**
     * 是否隐藏索引
     * @param QModelIndex index 索引
     * @return bool 是否隐藏
     */
    virtual bool isIndexHidden(const QModelIndex &index) const override{ return false; }

    /**
     * 设置选择
     * @param QRect rect 矩形
     * @param QItemSelectionModel::SelectionFlags command 命令
     */
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override{}

    /**
     * 设置选择区域
     * @param QItemSelection selection 选择区域
     * @return QRegion 区域
     */
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const override {return QRegion();}

    /**
     * 绘制事件
     * @param QPaintEvent *event 绘制事件
     */
    virtual void paintEvent(QPaintEvent *event) override;

    /**
     * 鼠标按下事件
     * @param QMouseEvent *event 鼠标事件
     */
    virtual void mousePressEvent(QMouseEvent *event) override;

    /**
     * 鼠标移动事件
     * @param QMouseEvent *event 鼠标事件
     */
    virtual void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * 鼠标释放事件
     * @param QMouseEvent *event 鼠标事件
     */
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * 鼠标双击事件
     * @param QMouseEvent *event 鼠标事件
     */
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    /**
     * 鼠标离开事件
     * @param QEvent *event 事件
     */
    virtual void leaveEvent(QEvent *event) override;

    // bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * 上下文菜单事件
     * @param QContextMenuEvent* event 上下文菜单事件
     */
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * 调整大小事件
     * @param QResizeEvent *event 调整大小事件
     */
    virtual void resizeEvent(QResizeEvent *event) override;

    /**
     * 显示事件
     * @param QShowEvent *event 显示事件
     */
    virtual void showEvent(QShowEvent *event) override;

    /**
     * 拖入事件
     * @param QDragEnterEvent *event 拖入事件
     */
    virtual void dragEnterEvent(QDragEnterEvent *event) override;

    /**
     * 拖动移动事件
     * @param QDragMoveEvent *event 拖动移动事件
     */
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    /**
     * 拖动释放事件
     * @param QDropEvent *event 拖动释放事件
     */
    virtual void dropEvent(QDropEvent *event) override;

    /**
     * 滚轮事件
     * @param QWheelEvent *event 滚轮事件
     */
    virtual void wheelEvent(QWheelEvent *event) override;
    /**
     * 绘制垂直时间线
     * @param QPainter* painter 绘图设备
     * @param QRect& rect 矩形
     */
    virtual void drawVerticalTimeLines(QPainter* painter, const QRect& rect);
    /**
     * 绘制背景
     * @param QPainter* painter 绘图设备
     * @param QRect& rect 矩形
     */
    virtual void drawBackground(QPainter* painter, const QRect& rect);
    /**
     * 绘制播放头
     * @param QPainter* painter 绘图设备
     */
    virtual void drawPlayhead(QPainter* painter);
    /**
     * 绘制时间刻度
     * @param QPainter* painter 绘图设备
     * @param QRect& rect 矩形
     */
    virtual void drawTimeRuler(QPainter* painter, const QRect& rect);
    /**
     * 绘制轨道
     * @param QPainter* painter 绘图设备
     */
    virtual void drawTracks(QPainter* painter);
    /**
     * 绘制时间标记
     * @param QPainter* painter 绘图设备
     * @param int startMarker 开始标记
     * @param int endMarker 结束标记
     * @param int frameStep 帧步长
     */
    virtual void drawTimeMarkers(QPainter* painter, int startMarker, int endMarker, int frameStep);
    /**
     * 计算帧步长
     * @param double frameRate 帧率
     * @return int 帧步长
     */
    virtual int calculateFrameStep(double frameRate) const;

    virtual BaseTimeLineModel* getModel() const;

    /**
     * 获取轨道宽度
     * @return int 轨道宽度
     */
    virtual int getTrackWdith() const;
    /**
     * 移动选定的剪辑
     * @param int dx 水平移动
     * @param int dy 垂直移动
     * @param bool isMouse 是否鼠标移动
     */
    virtual void moveSelectedClip(int dx, int dy,bool isMouse = true);
    /**
     * 移动播放头到指定帧
     * @param int frame 帧
     */
    virtual void movePlayheadToFrame(int frame);
    // 更新帧位置
    virtual void onFrameChanged(qint64 frame);
    // 播放状态改变
    virtual void onPlaybackStateChanged(bool isPlaying);
    // 判断文件对应的track类型

    virtual QString isMimeAcceptable(const QMimeData *Mime) const;
    // 在 protected 部分添加以下函数声明

/**
 * 处理鼠标拖动操作
 * @param event 鼠标事件对象
 */
virtual void handleMouseDrag(QMouseEvent *event);

/**
 * 更新鼠标悬停状态
 * @param event 鼠标事件对象
 */
virtual void updateMouseHoverState(QMouseEvent *event);

/**
 * 根据鼠标悬停状态更新鼠标指针形状
 */
virtual void updateCursorShape();
protected slots:
    /**
     * 选择更改
     * @param QItemSelection selected 选择
     * @param QItemSelection deselected 取消选择
     */
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    /**
     * 获取索引的代理
     * @param QModelIndex index 索引
     * @return QAbstractItemDelegate* 代理
     */
    // QAbstractItemDelegate* itemDelegateForIndex(const QModelIndex &index) const override ;

protected:
    // 当前缩放
    double currentScale = 1.0;
    // 时间线模型
    BaseTimeLineModel *Model;
    /**
     * 获取索引的矩形
     * @param QModelIndex index 索引
     * @return QRect 矩形
     */
    QRect itemRect(const QModelIndex &index) const;
    // 滚动偏移
    QPoint m_scrollOffset;
    /**
     * 点转换为帧
     * @param int point 点
     * @return int 帧
     */
    int pointToFrame(int point) const;
    /**
     * 帧转换为点
     * @param int frame 帧
     * @return int 点
     */
    int frameToPoint(int frame) const;    
    // 鼠标开始
    QPoint m_mouseStart;
    // 鼠标结束
    QPoint m_mouseEnd;
    // 鼠标偏移
    QPoint m_mouseOffset;
    // 鼠标是否按下
    bool mouseHeld = false;
    // 播放头是否选中
    bool m_playheadSelected = false;
    // 是否正在拖放媒体
    bool m_isSupportMedia = false;
    // 鼠标最后拖动位置
    QPoint m_lastDragPos;
    // 鼠标悬停状态
    hoverState m_mouseUnderClipEdge = NONE;
    // 鼠标悬停索引
    QModelIndex m_hoverIndex = QModelIndex();

};

#endif // TIMELINEVIEW_HPP



