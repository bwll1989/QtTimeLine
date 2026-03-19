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
#include <QItemSelectionModel>
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
    // 主布局
    QVBoxLayout *mainlayout;
    // 分割器：左侧 TrackList，右侧 Timeline
    QSplitter* splitter ;
    // 数据模型（轨道/片段/播放头等状态的唯一来源）
    BaseTimeLineModel* model;
    // 时间线视图（绘制 clip）
    BaseTimelineView* view;
    // 轨道列表视图（绘制 track 行）
    BaseTracklistView* tracklist;
    // 默认工具栏（控制播放/缩放等）
    DefaultTimeLineToolBar* toolbar;

private:
    /**
     * @brief 两个视图共享的选择模型
     * @details
     * 以往 TrackListView / TimelineView 各自创建 selectionModel，会出现“两套选择状态”，
     * 导致绑定链路复杂、状态不同步、逻辑越来越乱。
     * 这里将选择状态收敛为唯一来源：m_sharedSelectionModel。
     */
    QItemSelectionModel* m_sharedSelectionModel {nullptr};

    QObject* m_bindings {nullptr};
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

