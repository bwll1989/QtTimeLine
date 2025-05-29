#include "BaseTimeLineModel.h"


BaseTimeLineModel::BaseTimeLineModel(QObject* parent) : QAbstractItemModel(parent), m_pluginLoader(nullptr)
{

}
// 获取播放头位置
qint64 BaseTimeLineModel::getPlayheadPos() const{
    return m_currentFrame;
}
// 设置播放头位置
void BaseTimeLineModel::onSetPlayheadPos(int newPlayheadPos)
{
    m_currentFrame = newPlayheadPos;
    emit S_playheadMoved(newPlayheadPos);

}

QModelIndex BaseTimeLineModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        // Track level
        if (row < m_tracks.size()) {
            return createIndex(row, column, m_tracks[row]);
        }
    } else {
        // Clip level
        TrackData* track = static_cast<TrackData*>(parent.internalPointer());
        if (track && row < track->clips.size()) {
            return createIndex(row, column, track->clips[row]);
        }
    }
    return QModelIndex();
}

QModelIndex BaseTimeLineModel::parent(const QModelIndex &child) const
{
    // 如果子索引无效，返回空索引
    if (!child.isValid()) {
        return QModelIndex();
    }

    // Check if the child is a ClipModel
    AbstractClipModel* clip = static_cast<AbstractClipModel*>(child.internalPointer());
    if (clip) {
        // 获取剪辑所在轨道
        TrackData* track = findParentTrackOfClip(clip);
        if (track) {
            // 获取轨道行
            int trackRow = findTrackRow(track);
            // 创建索引
            return createIndex(trackRow, 0, track);
        }
    }

    return QModelIndex();
}

int BaseTimeLineModel::rowCount(const QModelIndex &parent) const
{
    // 如果父索引无效，返回轨道数量
    if (!parent.isValid()) {
        // Root level: number of tracks
        return m_tracks.size();
    }
    // 获取轨道
    TrackData* track = static_cast<TrackData*>(parent.internalPointer());
    if (track) {
        // 返回剪辑数量
        return track->clips.size();
    }

    return 0;
}

int BaseTimeLineModel::columnCount(const QModelIndex &parent) const
{
    // 如果父索引无效，返回1
    Q_UNUSED(parent);
    return 1; // Only one column for both tracks and clips
}

QVariant BaseTimeLineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
            case TimelineLengthRole:
                return QVariant::fromValue(m_lengthFrame);
            default:
                return QVariant();
        }
    }

    if (index.parent().isValid()) {
        // Clip level
        AbstractClipModel* clip = static_cast<AbstractClipModel*>(index.internalPointer());
        if (!clip) {
            qDebug() << "Clip pointer is null in data()";
            return QVariant();
        }
        switch (role) {
           
            //获取剪辑模型
            case TimelineRoles::ClipModelRole:
                return QVariant::fromValue(clip);
            //获取剪辑开始时间
            case TimelineRoles::ClipInRole:
                return QVariant::fromValue(clip->start());
            //获取剪辑结束时间
            case TimelineRoles::ClipOutRole:
                return QVariant::fromValue(clip->end());
            //获取剪辑长度
            case TimelineRoles::ClipLengthRole:
                return QVariant::fromValue(clip->length());
            //获取剪辑类型
            case TimelineRoles::ClipTypeRole:
                return QVariant::fromValue(clip->type());
            //获取剪辑是否显示小部件
            case TimelineRoles::ClipShowWidgetRole:
                return QVariant::fromValue(clip->isEmbedWidget());
            //获取剪辑是否可调整大小
            case TimelineRoles::ClipResizableRole:
                return QVariant::fromValue(clip->isResizable());
            //获取剪辑是否显示边框
            case TimelineRoles::ClipShowBorderRole:
                return QVariant::fromValue(clip->isShowBorder());
            //获取剪辑ID
            case TimelineRoles::ClipIdRole:
                return QVariant::fromValue(clip->id());
            default:
                return clip->data(role);
        }
    } else {
        // Track level
        TrackData* track = static_cast<TrackData*>(index.internalPointer());
            switch (role){
            //获取轨道工具提示
            case Qt::ToolTipRole:
                return QVariant::fromValue("track " + QString::number(index.row()));
            //获取轨道类型
            case TrackTypeRole:
                return QVariant::fromValue(TrackTypeString(track->type));
            //获取轨道名称
            case TrackNameRole:
                return QVariant::fromValue(track->name);
            //获取轨道模型
            case TrackModelRole:
                return QVariant::fromValue(track);
            //获取轨道剪辑数量
            case TrackClipsCountRole:
                return QVariant::fromValue(track->clips.size());
            //获取轨道剪辑列表
            case TrackClipsRole:
                return QVariant::fromValue(track->clips);
            default:
                return QVariant();
        }
    }
    return QVariant();
}

void BaseTimeLineModel::onStartPlay(){
   qDebug()<<"onStartPlay";
}

void BaseTimeLineModel::onPausePlay(){
    qDebug()<<"onPausePlay";
}

void BaseTimeLineModel::onStopPlay(){
   qDebug()<<"onStopPlay";
}

// 设置数据
bool BaseTimeLineModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //如果索引是时间轴本身
    if (!index.isValid()) {
        return false;
    }
    if (index.parent().isValid()){
        //如果索引是剪辑
        AbstractClipModel* clip = static_cast<AbstractClipModel*>(index.internalPointer());

        if (!clip)
            return false;

        switch (role) {
            // 设置剪辑开始时间
            case TimelineRoles::ClipInRole: {
                int newStart = value.toInt();
                clip->setStart(newStart);
                return true;
            }
            case TimelineRoles::ClipPosRole:{
                int newPos=value.toInt();
                int clipLength = clip->length();
                clip->setStart(newPos);
                clip->setEnd(newPos+clipLength);
                return true;
            }
            case TimelineRoles::ClipIdRole:{
                clip->setId(value.toInt());
                return true; 
            }
                // 设置剪辑是否显示小部件
            case TimelineRoles::ClipShowWidgetRole: {
                bool newShowWidget = value.toBool();
                clip->setEmbedWidget(newShowWidget);
                return true;
            }
                // 设置剪辑是否可调整大小
            case TimelineRoles::ClipResizableRole: {
                bool newResizable = value.toBool();
                clip->setResizable(newResizable);
                return true;
            }
                // 设置剪辑结束时间
            case TimelineRoles::ClipOutRole: {
                int newEnd = value.toInt();
                clip->setEnd(newEnd);
                return true;
            }
                // 设置剪辑长度
            case TimelineRoles::ClipLengthRole: {
                int newLength = value.toInt();
                clip->setEnd(clip->start()+newLength);
                //            emit dataChanged(index, index);
                //            onTimelineLengthChanged();
                return true;
            }
                // 设置剪辑是否显示边框
            case TimelineRoles::ClipShowBorderRole: {
                bool newShowBorder = value.toBool();
                clip->setShowBorder(newShowBorder);
                //            emit dataChanged(index, index);
                return true;
            }
                // 可以添加其他角色的处理
            default:
                return false;
        }

    }else{
        TrackData* track = static_cast<TrackData*>(index.internalPointer());
        if (!track)
            return false;
        switch (role){
            //设置轨道名称
            case TimelineRoles::TrackNameRole:
                track->name=value.toString();
                return true;
            default:
                return false;
        }

    }
}

// 获取支持的拖放操作
Qt::DropActions BaseTimeLineModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

// 获取项目标志
Qt::ItemFlags BaseTimeLineModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled; 
}


void BaseTimeLineModel::clear(){
    while(rowCount()>0){
        onDeleteTrack(0);
    }
    
}

// 保存模型
/**
 * @brief 保存时间线模型的状态到 JSON 对象
 * @return 包含模型状态的 JSON 对象
 */
QJsonObject BaseTimeLineModel::save() const {
    QJsonObject modelJson;
    QJsonArray trackArray;
    modelJson["displayFormat"] = static_cast<int>(m_timeDisplayFormat);
    
    for (const auto& track : m_tracks) {
        QJsonObject trackJson = track->save();
        QJsonArray clipArray;
        // 修正: 使用 const_reference 遍历避免拷贝，直接访问智能指针
        for (const AbstractClipModel* clip : track->clips) {
            clipArray.append(clip->save());
        }
        trackJson["clips"] = clipArray;
        trackArray.append(trackJson);
    }
    
    modelJson["tracks"] = trackArray;
    return modelJson;
}

void BaseTimeLineModel::load(const QJsonObject &modelJson) {
    clear();
    // 设置时间显示格式
    m_timeDisplayFormat = static_cast<TimedisplayFormat>(modelJson["displayFormat"].toInt());

    // 加载轨道
    QJsonArray trackArray = modelJson["tracks"].toArray();
    for (const QJsonValue &trackValue : trackArray) {
        QJsonObject trackJson = trackValue.toObject();
        QString type = trackJson["type"].toString();
        QString name = trackJson["trackName"].toString();

        // 添加新轨道
        onAddTrack(type);
        
        // 获取当前轨道并更新属性

            TrackData* track = m_tracks[m_tracks.size()-1];
            setData(index(m_tracks.size()-1, 0, QModelIndex()), name, TrackNameRole);
            // 加载轨道中的片段
            QJsonArray clipArray = trackJson["clips"].toArray();
            for(const QJsonValue &clipValue : clipArray) {
                QJsonObject clipJson = clipValue.toObject();
                
                // 创建新片段
                AbstractClipModel* clip = getPluginLoader()->createModelForType(
                    clipJson["type"].toString(),
                    clipJson["start"].toInt()
                );
                
                if(clip) {
                    // 加载片段数据
                    m_clipNextId = qMax(m_clipNextId,clipJson["ID"].toInt());
                    clip->load(clipJson);
                    
                    // 连接信号
                    connect(clip, &AbstractClipModel::lengthChanged, [this]() {
                        emit S_clipGeometryChanged();
                    });
                    connect(clip, &AbstractClipModel::timelinePositionChanged, [this]() {
                        emit S_clipGeometryChanged();
                    });
                    
                    // 添加到轨道
                    track->clips.push_back(clip);
                    emit S_addClip();

            }
        }
    }

    // 更新UI
    emit S_trackAdd();
    onUpdateTimeLineLength();

}
// 计算时间线长度
void BaseTimeLineModel::onUpdateTimeLineLength()
{
    // qDebug()<<"onTimelineLengthChanged";
    int max = 0;
    for(const auto& track : m_tracks){
        for(const auto& clip : track->clips){
            max = qMax(max, clip->end());
        }
    }
    if(max!=m_lengthFrame){
        m_lengthFrame = max;
        emit S_LengthChanged(m_lengthFrame);
    }

}

void BaseTimeLineModel::onDeleteTrack(int trackIndex) {

    if (trackIndex < 0 || trackIndex >= m_tracks.size()) {
        qDebug() << "Invalid track index";
        return;
    }

    beginRemoveRows(QModelIndex(), trackIndex, trackIndex); // 开始移除行
    // 删除轨道上的所有片段
    for (const auto& clip : m_tracks[trackIndex]->clips) {
        onDeleteClip(createIndex(0, 0, clip));
    }
    m_tracks.erase(m_tracks.begin() + trackIndex);


    endRemoveRows(); // 结束移除行
    emit S_trackDelete();

} 
// 通过类型名创建轨道
void BaseTimeLineModel::onAddTrack(const QString& type) {
    if (!m_pluginLoader) return;
        
    beginInsertRows(QModelIndex(), m_tracks.size(), m_tracks.size());
    auto track = new TrackData();
    track->type = type;
    track->name = "Track " + QString::number(m_tracks.size());
    m_tracks.push_back(std::move(track));
    endInsertRows();
    emit S_trackAdd();
}

void BaseTimeLineModel::onMoveTrack(int sourceRow, int targetRow) {
    m_tracks.move(sourceRow, targetRow);
    // 更新所有轨道的索引
    emit S_trackMoved(sourceRow,targetRow);
}

void BaseTimeLineModel::onAddClip(int trackIndex, int startFrame) {
    // 检查轨道索引是否有效
    if(trackIndex < 0 || trackIndex >= m_tracks.size()) {
        qDebug() << "Invalid track index:" << trackIndex;
        return;
    }

    // 检查插件加载器是否存在
    if(!getPluginLoader()) {
        qDebug() << "Plugin loader is not initialized";
        return;
    }

    AbstractClipModel* newClip = getPluginLoader()->createModelForType(m_tracks[trackIndex]->type, startFrame);
    if(!newClip) {
        qDebug() << "Failed to create clip model";
        return;
    }
    // 连接信号,片段长度和位置变化时，模型向视图发送_clipGeometryChanged信号，通知视图更新
    connect(newClip, &AbstractClipModel::lengthChanged, [this]() {
        onUpdateTimeLineLength();
        emit S_clipGeometryChanged();
    });
    connect(newClip, &AbstractClipModel::timelinePositionChanged, [this]() {
        emit S_clipGeometryChanged();
    });
    m_tracks[trackIndex]->clips.push_back(newClip);
    setData(createIndex(trackIndex, 0, newClip), QVariant::fromValue(m_clipNextId++), TimelineRoles::ClipIdRole);
    emit S_addClip();

}


void BaseTimeLineModel::onDeleteClip(QModelIndex clipIndex){
    if (!clipIndex.isValid() || !clipIndex.parent().isValid())
        return;

    // 获取轨道索引和轨道
    QModelIndex trackIndex = clipIndex.parent();
    TrackData* track = m_tracks[trackIndex.row()];
    if (!track)
        return;
        
    // 开始删除行
    beginRemoveRows(trackIndex, clipIndex.row(), clipIndex.row());

    // 获取要删除的片段
    if(clipIndex.row() < track->clips.size()) {
        // 从轨道中移除片段
        track->clips.erase(track->clips.begin() + clipIndex.row());
    }

    // 结束删除行
    endRemoveRows();
    //更新时间线长度
    onUpdateTimeLineLength();
    emit S_deleteClip();
}

/**
 * @brief 查找片段所在的轨道
 * @param clip 要查找的片段指针
 * @return 返回包含该片段的轨道的智能指针，如果未找到则返回nullptr
 */
TrackData* BaseTimeLineModel::findParentTrackOfClip(AbstractClipModel* clip) const {
    for (auto track : m_tracks) {
        const auto& clips = track->clips;
        auto it = std::find_if(clips.begin(), clips.end(),
            [clip](AbstractClipModel* storedClip) { // 修改 lambda 参数类型
                return storedClip == clip;
            });

        if (it != clips.end()) {
            return track; // 直接返回原始指针
        }
    }
    return nullptr;
}
// 查找轨道行
int BaseTimeLineModel::findTrackRow(TrackData* track) const {
    // 错误: 直接比较智能指针是不正确的
    // 修正: 需要比较指针值
    for (size_t i = 0; i < m_tracks.size(); ++i) {
        if (m_tracks[i] == track) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void BaseTimeLineModel::setPluginLoader(BasePluginLoader* loader) {
    if (m_pluginLoader) {
        delete m_pluginLoader;
    }
    m_pluginLoader = loader;
    m_pluginLoader->loadPlugins();
}

BasePluginLoader* BaseTimeLineModel::getPluginLoader() const {
    return m_pluginLoader;
}

// 在析构函数中清理
BaseTimeLineModel::~BaseTimeLineModel()
{
    m_tracks.clear(); // 智能指针会自动清理内存
    delete m_pluginLoader;
}

int BaseTimeLineModel::getTrackCount() const {
    return m_tracks.size();
}


