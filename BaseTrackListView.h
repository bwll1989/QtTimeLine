#ifndef BASETRACKLISTVIEW_H
#define BASETRACKLISTVIEW_H

#include <QAbstractItemView>
#include <QMenu>
#include "BaseTrackDelegate.h"
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QDrag>
#include <QApplication>
#include "BaseTimeLineModel.h"
#include "TimeLineStyle.h"
#include <QTimer>
#include <QMessageBox>

class BaseTracklistView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BaseTracklistView(BaseTimelineModel *viemModel, QWidget *parent = nullptr);

    ~BaseTracklistView() override=default;
    /**
     * 更新滚动条
     */
    void updateScrollBars();
    /**
     * 获取可视区域
     * @param QModelIndex index 索引
     * @return QRect 可视区域
     */
    QRect visualRect(const QModelIndex &index) const override;
    /**
     * 滚动到指定索引
     * @param QModelIndex index 索引
     * @param ScrollHint hint 提示
     */
    void scrollTo(const QModelIndex &index, ScrollHint hint) override{}
    /**
     * 获取指定点的索引
     */
    QModelIndex indexAt(const QPoint &point) const override;


signals:
    /**
     * 滚动信号
     * @param int dx 水平滚动   
     * @param int dy 垂直滚动
     */
    void trackScrolled(int dx, int dy);
    /**
     * 视图更新信号
     */
    void viewupdate();

public slots:
    /**
     * 滚动
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    void scroll(int dx, int dy);
   
    /**
     * 更新视图
     */
    void onUpdateViewport();
    
protected:
    /**
     * 移动光标
     * @param CursorAction cursorAction 光标动作
     * @param Qt::KeyboardModifiers modifiers 键盘修饰符
     * @return QModelIndex 索引
     */
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override { return QModelIndex();}
    /**
     * 水平偏移
     * @return int 水平偏移
     */
    int horizontalOffset() const override {return 0;}
    /**
     * 垂直偏移
     * @return int 垂直偏移
     */
    int verticalOffset() const override {return 0;}
    /**
     * 是否隐藏索引
     * @param QModelIndex index 索引
     * @return bool 是否隐藏
     */ 
    bool isIndexHidden(const QModelIndex &index) const override { return false; }
    /**
     * 设置选择
     * @param QRect rect 矩形
     * @param QItemSelectionModel::SelectionFlags command 命令
     */
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override ;
    /**
     * 获取选择区域
     * @param QItemSelection selection 选择
     * @return QRegion 区域
     */
    QRegion visualRegionForSelection(const QItemSelection &selection) const override {return QRegion();}
    /**
     * 绘制事件
     * @param QPaintEvent *event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * 鼠标按下事件
     * @param QMouseEvent *event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * 鼠标移动事件
     * @param QMouseEvent *event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    /**
     * 离开事件
     * @param QEvent *event 事件
     */
    void leaveEvent(QEvent *event) override;
    /**
     * 调整事件
     * @param QResizeEvent *event 调整事件
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * 显示事件
     * @param QShowEvent *event 显示事件
     */
    void showEvent(QShowEvent *event) override;
    /**
     * 滚动内容
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    void scrollContentsBy(int dx, int dy) override;
    /**
     * 获取项目矩形
     * @param QModelIndex index 索引
     * @return QRect 矩形
     */
    QRect itemRect(const QModelIndex &index) const;
    /**
     * 事件过滤
     * @param QObject *watched 观察对象
     * @param QEvent *event 事件
     * @return bool 是否过滤
     */
    bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * 上下文菜单事件
     * @param QContextMenuEvent* event 上下文菜单事件
     */
    void contextMenuEvent(QContextMenuEvent* event) override ;
protected slots:
    /**
     * 删除轨道
     */
    void onDeleteTrack();
    /**
     * 选择改变
     * @param QItemSelection selected 选择
     * @param QItemSelection deselected 取消选择
     */
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    /**
     * 更新编辑几何
     */
    void updateEditorGeometries() override;
    /**
     * 拖入事件
     * @param QDragEnterEvent *event 拖入事件
     */
    void dragEnterEvent(QDragEnterEvent *event) override;
    /**
     * 拖动移动事件
     * @param QDragMoveEvent *event 拖动移动事件
     */
    void dragMoveEvent(QDragMoveEvent *event) override;
    /**
     * 拖动释放事件
     * @param QDropEvent *event 拖动释放事件
     */
    void dropEvent(QDropEvent *event) override;

public:
    /**
     * 删除轨道动作
     */
    QAction* m_deleteTrackAction;
private:
    //滚动偏移  
    QPoint m_scrollOffset;
    //拖动开始位置
    QPoint m_dragStartPosition;
    //悬停索引
    QModelIndex m_hoverIndex = QModelIndex();
    //模型
    BaseTimelineModel *Model;
};

#endif // TRACKLISTVIEW_HPP
