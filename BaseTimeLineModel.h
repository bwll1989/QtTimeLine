#ifndef BASETIMELINEMODEL_H
#define BASETIMELINEMODEL_H

#include <QAbstractItemModel>
#include <unordered_set>
#include <unordered_map>
#include "AbstractClipModel.hpp"
#include <string>
#include <QMimeData>
#include <QIODevice>
#include <algorithm>
#include <QList>
#include <QJsonArray>
#include "TimeLineDefines.h"
#include "TimeLineStyle.h"
#include <QFile>
#include <QJsonDocument>
#include "BasePluginLoader.h"
#include "Export.hpp"
struct TrackData {
    QString type;
    QString name;
    // 修改前: QVector<std::unique_ptr<AbstractClipModel>> clips;
    // 修改后: 使用 QList 替代 QVector
    QList<AbstractClipModel*> clips;
    QJsonObject save() const
{
    QJsonObject trackJson;
    trackJson["type"] = type;
    trackJson["trackName"] = name;
    return trackJson;
}
};
// TimelineModel类继承自QAbstractItemModel
class NODE_TIMELINE_PUBLIC BaseTimeLineModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    // 构造函数
    BaseTimeLineModel(QObject* parent = nullptr);

    ~BaseTimeLineModel();
    //获取当前播放位置
    virtual qint64 getPlayheadPos() const;
    /**
     * 获取索引
     * @param int row 行
     * @param int column 列
     * @param QModelIndex parent 父索引
     * @return QModelIndex 索引
     */
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取父索引
     * @param const QModelIndex &child 子索引
     * @return QModelIndex 父索引
     */
    virtual QModelIndex parent(const QModelIndex &child) const override;
    /**
     * 获取行数
     * @param const QModelIndex &parent 父索引
     * @return int 行数 
     */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取列数
     * @param const QModelIndex &parent 父索引
     * @return int 列数
     */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * 获取数据
     * @param const QModelIndex &index 索引
     * @param int role 角色
     * @return QVariant 数据
     */
    virtual QVariant data(const QModelIndex &index, int role) const override;
    /**
     * 设置数据
     * @param const QModelIndex &index 索引
     * @param const QVariant &value 值
     * @param int role 角色
     * @return bool 是否设置成功
     */
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
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
    virtual void clear();
    /**
     * 保存模型
     * @return QJsonObject 保存的模型
     */
    virtual QJsonObject save() const;
    /**
     * 加载模型
     * @param const QJsonObject &modelJson 加载的模型
     */
    virtual void load(const QJsonObject &modelJson) ;
    /**
     * 获取轨道数量
     * @return int 轨道数量
     */
    virtual int getTrackCount() const ;
    /**
     * 获取轨道
     * @return QVector<TrackData> 轨道
     */
    QList<TrackData*> getTracks() const { return m_tracks; }
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
    // 设置插件加载器
    void setPluginLoader(BasePluginLoader* loader) ;
signals:
    //轨道移动信号
    void S_trackMoved(int oldIndex, int newIndex);
    //添加轨道信号
    void S_trackAdd();
    //删除轨道信号
    void S_trackDelete();
    //移动播放头信号
    void S_playheadMoved(int frame);
    //时间线长度变化信号
    void S_LengthChanged(qint64 length);
    //添加片段信号
    void S_addClip();
    //删除片段信号
    void S_deleteClip();
    //片段位置外形变化信号
    void S_clipGeometryChanged();
public slots:
    //开始播放槽函数
    virtual void onStartPlay();
    //暂停播放槽函数
    virtual void onPausePlay();
    //停止播放槽函数
    virtual void onStopPlay();
    //时间轴长度变化槽函数
    virtual void onUpdateTimeLineLength();
    //设置播放头位置槽函数
    virtual void onSetPlayheadPos(int newPlayheadPos);
    //通过轨道索引和开始帧添加片段
    virtual void onAddClip(int trackIndex,int startFrame);
    //删除片段槽函数
    virtual void onDeleteClip(QModelIndex clipIndex);
    //通过类型添加轨道槽函数
    virtual void onAddTrack(const QString& type);
    //删除轨道槽函数
    virtual void onDeleteTrack(int trackIndex);
    //移动轨道槽函数
    virtual void onMoveTrack(int sourceRow, int targetRow);

private:
    // 查找片段所在轨道
    TrackData* findParentTrackOfClip(AbstractClipModel* clip) const ;
    // 查找轨道行
    int findTrackRow(TrackData* track) const ;

    // 时间显示格式，默认显示时间码
    TimedisplayFormat m_timeDisplayFormat = TimedisplayFormat::AbsoluteTimeFormat;
    //插件加载器
    BasePluginLoader* m_pluginLoader; // 插件加载器
    /**
     * 获取轨道
     * @param int index 轨道索引
     * @return TrackData 轨道
     */
    // 修改前: QVector<std::unique_ptr<TrackData>> m_tracks;
    // 修改后: 使用 QList 替代 QVector
    QList<TrackData*> m_tracks;
    
    qint64 m_currentFrame = 0; // 当前帧

    qint64 m_lengthFrame = 0; // 总帧数

    qint64 m_clipNextId=0;
};
#endif // TIMELINEMODEL_H

