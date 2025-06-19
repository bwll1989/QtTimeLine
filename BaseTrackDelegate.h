#ifndef BASETRACKDELEGATE_H
#define BASETRACKDELEGATE_H

#include <QAbstractItemDelegate>
#include <QObject>

#include<QPainter>
#include <QPushButton>
#include <QLayout>
#include <QSpacerItem>
#include "QLabel"
#include "BaseTimeLineModel.h"
#include "TimeLineStyle.h"
#include "TimelineExports.hpp"
class NODE_TIMELINE_PUBLIC BaseTrackDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    /**
     * 构造函数
     * @param QObject *parent 父窗口
     */
    explicit BaseTrackDelegate(QObject *parent = nullptr);
    /**
     * 绘制
     * @param QPainter *painter 绘图器
     * @param const QStyleOptionViewItem &option 选项
     * @param const QModelIndex &index 索引
     */
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    /**
     * 大小提示
     * @param const QStyleOptionViewItem &option 选项
     * @param const QModelIndex &index 索引
     * @return QSize 大小
     */
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    /**
     * 创建编辑器
     * @param QWidget *parent 父窗口
     * @param const QStyleOptionViewItem &option 选项
     * @param const QModelIndex &index 索引
     * @return QWidget * 编辑器
     */
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // QAbstractItemDelegate interface
public:
    /**
     * 更新编辑器几何形状
     * @param QWidget *editor 编辑器
     * @param const QStyleOptionViewItem &option 选项
     * @param const QModelIndex &index 索引
     */
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * 设置编辑器数据
     * @param QWidget *editor 编辑器
     * @param const QModelIndex &index 索引
     */
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override ;

};

#endif // TRACKDELEGATE_H
