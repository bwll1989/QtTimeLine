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
#include "TimelineExports.hpp"
#include "TimeCodeDefines.h"
using namespace QtTimeline;
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
    virtual QVariant clipData(ClipId clipID, TimelineRoles role) const ;

    virtual bool setClipData(ClipId clipID, TimelineRoles role, QVariant value);

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

    QString getModelAlias() const { return m_modelAlias; }
    // 设置模型别名
    void setModelAlias(const QString& alias) {
        m_modelAlias = alias.startsWith("/") ? alias : ("/" + alias);
    }

    // 设置插件加载器
    void setPluginLoader(BasePluginLoader* loader) ;

    virtual TimeCodeType getTimeCodeType() const
    {
        return TimeCodeType::PAL;
    }

    virtual void setTimeCodeType(TimeCodeType type)
    {
        m_timeCodeType=type;
    }
signals:
    /**
     * @brief 播放头位置变化信号
     * @details
     * 播放头不是 Model/View 树中的某一行数据，因此不适合用 dataChanged 表达。
     * 保留此信号供 Timeline/TrackList 的标题区域轻量刷新使用。
     */
    void S_playheadMoved(int frame);

    /**
     * @brief 时间线总长度变化信号
     * @details
     * 总长度是聚合值（由所有 clip 的 end 取 max 计算），不对应某一行的直接字段。
     */
    void S_LengthChanged(qint64 length);
public slots:
    //开始播放槽函数
    virtual void onStartPlay();
    //暂停播放槽函数
    virtual void onPausePlay();
    //停止播放槽函数
    virtual void onStopPlay();
    //时间轴长度变化槽函数
    virtual qint64 onUpdateTimeLineLength();
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

protected:
    /**
     * @brief 通过 clip 指针反查其在模型中的索引
     * @details
     * ClipTimePanel 会直接调用 AbstractClipModel::setStart/setEnd 等修改片段。
     * 为了让 View 能走 Qt 标准路径刷新，这里提供从 clip 指针构建 QModelIndex 的能力，
     * 以便在监听到 clip 的信号后发出 dataChanged。
     */
    QModelIndex indexForClip(AbstractClipModel* clip) const;

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

    ClipId m_clipNextId=0;

    QString m_modelAlias = "default123";

    TimeCodeType m_timeCodeType=TimeCodeType::PAL;
};
#endif // TIMELINEMODEL_H

