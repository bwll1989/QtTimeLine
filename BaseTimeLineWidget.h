//
// Created by bwll1 on 2024/5/27.
//

#pragma once
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
#include "DefaultTimeLineToolBar.h"



class NODE_TIMELINE_PUBLIC BaseTimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit BaseTimelineWidget(BaseTimeLineModel* model=nullptr, QWidget *parent = nullptr);
    ~BaseTimelineWidget() override;

public:
    //主布局
    QVBoxLayout *mainlayout;
    //分割器
    QSplitter* splitter ;
    //模型
    BaseTimeLineModel* model;
    //视图
    BaseTimelineView* view;
    //轨道列表
    BaseTracklistView* tracklist;
    //工具栏
    DefaultTimeLineToolBar* toolbar;
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

