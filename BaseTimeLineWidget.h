//
// Created by bwll1 on 2024/5/27.
//

#ifndef BASETIMELINEWIDGET_H
#define BASETIMELINEWIDGET_H
#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define Q_DECL_EXPORT __declspec(dllexport)
#  define Q_DECL_IMPORT __declspec(dllimport)
#else
#  define Q_DECL_EXPORT     __attribute__((visibility("default")))
#  define Q_DECL_IMPORT     __attribute__((visibility("default")))
#endif

#if defined(TIMELINEWIDGET_LIBRARY)
#  define TIMELINEWIDGET_EXPORT Q_DECL_EXPORT
#else
#  define TIMELINEWIDGET_EXPORT Q_DECL_IMPORT
#endif


#include <QToolBar>
#include<QStyleFactory>
#include <QSplitter>
#include <QVBoxLayout>
#include <QShortcut>
#include <QScreen>
#include "qtreeview.h"
#include "BaseTimeLineModel.h"
#include "BaseTimeLineView.h"
#include "BaseTrackListView.h"
#include "TimeLineDefines.h"
#include <QWidget>
#include <QFileDialog>
#include <QJsonDocument>
#include <QGuiApplication>


class TIMELINEWIDGET_EXPORT BaseTimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseTimelineWidget(BaseTimelineModel* model=nullptr, QWidget *parent = nullptr);
    ~BaseTimelineWidget() override;

public:
    //主布局
    QVBoxLayout *mainlayout;
    //分割器
    QSplitter* splitter = new QSplitter(Qt::Horizontal,this);
    //模型
    BaseTimelineModel* model;
    //视图
    BaseTimelineView* view;
    //轨道列表
    BaseTracklistView* tracklist;
   

Q_SIGNALS:
    //初始化信号
    void initialized();

public slots:
    /**
     * 保存
     * @return QJsonObject 保存的json对象
     */
    virtual QJsonObject save();
    /**
     * 加载
     * @param const QJsonObject& json 加载的json对象
     */
    virtual void load(const QJsonObject& json);
    /**
     * 显示设置对话框
     */
    virtual void showSettingsDialog(); // Add new slot


private:
    /**
     * 创建组件
     */
    virtual void createComponents();

};


#endif //TIMELINE_TIMEWIDGET_H
