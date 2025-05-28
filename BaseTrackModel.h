#ifndef BASETRACKMODEL_H
#define BASETRACKMODEL_H
#pragma once
#include <QObject>
#include "TimeLineDefines.h"
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include "AbstractClipModel.h"
#include "BasePluginLoader.h"
// 前向声明 BaseTimelineModel 类
class BaseTimelineModel;
// TrackModel类
class BaseTrackModel : public QObject
{
Q_OBJECT
public:
    // 构造函数
    explicit BaseTrackModel(int number, const QString& type, QObject* parent = nullptr);
    BaseTrackModel() = default;
    ~BaseTrackModel() override;
    /**
     * 移除剪辑
     * @param AbstractClipModel* clip 剪辑
     */
    void removeClip(AbstractClipModel* clip) ;

    /**
     * 获取剪辑
     * @return QVector<AbstractClipModel*>& 剪辑列表
     */
    QVector<AbstractClipModel*>& getClips();
    /**
     * 获取轨道类型
     * @return QString 轨道类型
     */
    QString type() const;
    /**
     * 获取轨道索引
     * @return int 轨道索引
     */
    int trackIndex() const;
    /**
     * 保存轨道
     * @return QJsonObject 轨道JSON
     */
    QJsonObject save() const;
    /**
     * 获取轨道长度
     * @return qint64 轨道长度
     */
    qint64 getTrackLength() const ;
    /**
     * 加载轨道
     * @param const QJsonObject &trackJson 轨道JSON
     * @param PluginLoader* pluginLoader 插件加载器
     */
    void setName(const QString& name);

    QString getName() const;

    void load(const QJsonObject &trackJson, BasePluginLoader* pluginLoader) ;

public:
    Q_SIGNALS:
    /**
     * 轨道长度变化信号
     * @param qint64 length 轨道长度
     */
    void S_trackLengthChanged(qint64 length);

    void S_trackAddClip();

    void S_trackDeleteClip();

    void S_trackIndexChanged(int oldIndex, int newIndex );
public slots:
     /**
     * 计算轨道长度
     */
    void onCalculateTrackLength();
    /**
     * 设置轨道索引
     * @param int index 轨道索引
     */
    void onSetTrackIndex(int index);
    /**
     * 添加剪辑
     * @param int start 开始
     * @param PluginLoader* pluginLoader 插件加载器
     */
    void onAddClip(AbstractClipModel* newClip) ;
private:
     //下一个ID
    quint64 nextId = 1;
    //轨道索引
    int m_trackIndex; 
    //轨道类型
    QString m_type; 
    //剪辑列表
    QVector<AbstractClipModel*>  m_clips; 
    //轨道长度
    qint64 m_trackLength;
    QString m_name;
 
};

#endif // TRACKMODEL_HPP
