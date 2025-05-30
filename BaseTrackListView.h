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
#include "Export.hpp"
class NODE_TIMELINE_PUBLIC BaseTracklistView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BaseTracklistView(BaseTimeLineModel *viemModel, QWidget *parent = nullptr);

    ~BaseTracklistView() override=default;
    /**
     * 更新滚动条
     */
    virtual void updateScrollBars();
    /**
     * 获取可视区域
     * @param QModelIndex index 索引
     * @return QRect 可视区域
     */
    virtual QRect visualRect(const QModelIndex &index) const override;
    /**
     * 滚动到指定索引
     * @param QModelIndex index 索引
     * @param ScrollHint hint 提示
     */
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint) override{}
    /**
     * 获取指定点的索引
     */
    virtual QModelIndex indexAt(const QPoint &point) const override;


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
    void viewUpdate();

public slots:
    /**
     * 滚动
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    virtual void scroll(int dx, int dy);
   
    /**
     * 更新视图
     */
    virtual void onUpdateViewport();
    
protected:
    /**
     * 移动光标
     * @param CursorAction cursorAction 光标动作
     * @param Qt::KeyboardModifiers modifiers 键盘修饰符
     * @return QModelIndex 索引
     */
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override { return QModelIndex();}
    /**
     * 水平偏移
     * @return int 水平偏移
     */
    virtual int horizontalOffset() const override {return 0;}
    /**
     * 垂直偏移
     * @return int 垂直偏移
     */
    virtual int verticalOffset() const override {return 0;}
    /**
     * 是否隐藏索引
     * @param QModelIndex index 索引
     * @return bool 是否隐藏
     */ 
    virtual bool isIndexHidden(const QModelIndex &index) const override { return false; }
    /**
     * 设置选择
     * @param QRect rect 矩形
     * @param QItemSelectionModel::SelectionFlags command 命令
     */
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override ;
    /**
     * 获取选择区域
     * @param QItemSelection selection 选择
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
     * 离开事件
     * @param QEvent *event 事件
     */
    virtual void leaveEvent(QEvent *event) override;
    /**
     * 调整事件
     * @param QResizeEvent *event 调整事件
     */
    virtual void resizeEvent(QResizeEvent *event) override;
    /**
     * 显示事件
     * @param QShowEvent *event 显示事件
     */
    virtual void showEvent(QShowEvent *event) override;
    /**
     * 滚动内容
     * @param int dx 水平滚动
     * @param int dy 垂直滚动
     */
    virtual void scrollContentsBy(int dx, int dy) override;
    /**
     * 获取项目矩形
     * @param QModelIndex index 索引
     * @return QRect 矩形
     */
    virtual QRect itemRect(const QModelIndex &index) const;
    /**
     * 事件过滤
     * @param QObject *watched 观察对象
     * @param QEvent *event 事件
     * @return bool 是否过滤
     */
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    /**
     * 上下文菜单事件
     * @param QContextMenuEvent* event 上下文菜单事件
     */
    virtual void contextMenuEvent(QContextMenuEvent* event) override ;
protected slots:
    /**
     * 删除轨道
     */
    virtual void onDeleteTrack();
    /**
     * 选择改变
     * @param QItemSelection selected 选择
     * @param QItemSelection deselected 取消选择
     */
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    /**
     * 更新编辑几何
     */
    virtual void updateEditorGeometries() override;
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
    BaseTimeLineModel *Model;
};

#endif // TRACKLISTVIEW_HPP
