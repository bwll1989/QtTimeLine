#ifndef BASETIMELINEMODEL_H
#define BASETIMELINEMODEL_H

#include <QAbstractItemModel>
#include <unordered_set>
#include <unordered_map>
#include "BaseTrackModel.h"
#include "AbstractClipModel.h"
#include <string>
#include <QMimeData>
#include <QIODevice>
#include <algorithm>
#include <vector>
#include <QJsonArray>
#include "TimeLineDefines.h"
#include "TimeLineStyle.h"
#include <QFile>
#include <QJsonDocument>
#include "BasePluginLoader.h"
// TimelineModel类继承自QAbstractItemModel
class BaseTimelineModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    // 构造函数
    BaseTimelineModel(QObject* parent = nullptr);

    ~BaseTimelineModel();
    //获取当前播放位置
    qint64 getPlayheadPos() const;
    /**
     * 获取索引
     * @param int row 行
     * @param int column 列
     * @param QModelIndex parent 父索引
     * @return QModelIndex 索引
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取父索引
     * @param const QModelIndex &child 子索引
     * @return QModelIndex 父索引
     */
    QModelIndex parent(const QModelIndex &child) const override;
    /**
     * 获取行数
     * @param const QModelIndex &parent 父索引
     * @return int 行数 
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取列数
     * @param const QModelIndex &parent 父索引
     * @return int 列数
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取数据
     * @param const QModelIndex &index 索引
     * @param int role 角色
     * @return QVariant 数据
     */
    QVariant data(const QModelIndex &index, int role) const override;
    /**
     * 设置数据
     * @param const QModelIndex &index 索引
     * @param const QVariant &value 值
     * @param int role 角色
     * @return bool 是否设置成功
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    /**
     * 获取支持的拖放操作
     * @return Qt::DropActions 支持的拖放操作
     */
    Qt::DropActions supportedDropActions() const override;
    /**
     * 获取项目标志
     * @param const QModelIndex &index 索引
     * @return Qt::ItemFlags 项目标志
     */
    // 获取项目标志
    Qt::ItemFlags flags(const QModelIndex &index) const ;
    /**
     * 清除
     */
    void clear();
    /**
     * 保存模型
     * @return QJsonObject 保存的模型
     */
    QJsonObject save() const;
    /**
     * 加载模型
     * @param const QJsonObject &modelJson 加载的模型
     */
    void load(const QJsonObject &modelJson) ;
    /**
     * 获取轨道数量
     * @return int 轨道数量
     */
    int getTrackCount() const ;
    /**
     * 获取轨道
     * @return QVector<BaseTrackModel*> 轨道
     */
    QVector<BaseTrackModel*> getTracks() const { return m_tracks; }
    /**
     * 获取插件加载器
     * @return BasePluginLoader* 插件加载器
     */
    BasePluginLoader* getPluginLoader() const ;

    /**
     * 获取时间码显示格式
     */
    TimedisplayFormat getTimeDisplayFormat() const {
        return  m_timeDisplayFormat;
    }
    /**
     * 设置时间码显示格式
     */
    void setTimeDisplayFormat(TimedisplayFormat val){
        m_timeDisplayFormat=val;
    }
   
signals:
    //时间线更新信号
    void S_timelineUpdated();
    //轨道移动信号
    void S_trackMoved(int oldIndex, int newIndex);
    //添加轨道信号
    void S_trackAdd();
    //删除轨道信号
    void S_trackDelete();
    //移动播放头信号
    void S_playheadMoved(int frame);
    //时间线长度变化信号
    void S_timelineLengthChanged();
    //添加片段信号
    void S_addClip();
    //删除片段信号
    void S_deleteClip();

public slots:
    //开始播放槽函数
    void onStartPlay();
    //暂停播放槽函数
    void onPausePlay();
    //停止播放槽函数
    void onStopPlay();
    //时间轴长度变化槽函数
    void onTimelineLengthChanged();
    //设置播放头位置槽函数
    void onSetPlayheadPos(int newPlayheadPos);
    //通过轨道索引和开始帧添加片段
    void onAddClip(int trackIndex,int startFrame);
    //通过片段模型添加片段
    void onAddClip(AbstractClipModel* clip);
    //删除片段槽函数
    void onDeleteClip(QModelIndex clipIndex);
    //通过类型添加轨道槽函数
    void onAddTrack(const QString& type);
    //通过片段模型添加轨道槽函数
    void onAddTrack(BaseTrackModel* track);
    //删除轨道槽函数
    void onDeleteTrack(int trackIndex);
    //移动轨道槽函数
    void onMoveTrack(int sourceRow, int targetRow);

private:
    // 查找片段所在轨道
    BaseTrackModel* findParentTrackOfClip(AbstractClipModel* clip) const ;
    // 查找轨道行
    int findTrackRow(BaseTrackModel* track) const ;
    // 设置插件加载器
    void setPluginLoader(BasePluginLoader* loader) ;
    // 时间显示格式，默认显示时间码
    TimedisplayFormat m_timeDisplayFormat = TimedisplayFormat::AbsoluteTimeFormat;
    //插件加载器
    BasePluginLoader* m_pluginLoader; // 插件加载器
    /**
     * 获取轨道
     * @param int index 轨道索引
     * @return BaseTrackModel* 轨道
     */
    QVector<BaseTrackModel*> m_tracks; // 轨道
    
    qint64 m_currentFrame = 0; // 当前帧
};

#endif // TIMELINEMODEL_H
