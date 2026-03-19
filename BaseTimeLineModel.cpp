#include "BaseTimeLineModel.h"


BaseTimeLineModel::BaseTimeLineModel(QObject* parent) : QAbstractItemModel(parent), m_pluginLoader(nullptr)
{
      // 创建并初始化插件加载器
    m_pluginLoader = new BasePluginLoader(this);
    m_pluginLoader->loadPlugins();
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
            case Qt::ToolTipRole:
                return QVariant::fromValue(QString("Length:%1").arg(m_lengthFrame));
            case TimelineLengthRole:
                return QVariant::fromValue(m_lengthFrame);
            case TimeCodeTypeRole:
                return QVariant::fromValue(getTimeCodeType());
            case TimelineModelAliasRole:
                return QVariant::fromValue(m_modelAlias);
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
            case Qt::ToolTipRole:
                return QVariant::fromValue(QString(clip->type()+ ":%1 to %2").arg(clip->start()).arg(clip->end()));
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
            case TimelineRoles::TimeCodeTypeRole:
                return QVariant::fromValue(clip->getTimeCodeType());
            default:
                return clip->data(role);
        }
    } else {
        // Track level
        TrackData* track = static_cast<TrackData*>(index.internalPointer());
            switch (role){
            //获取轨道工具提示
            case Qt::ToolTipRole:
                return QVariant::fromValue(track->name);
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

QVariant BaseTimeLineModel::clipData(ClipId clipID, TimelineRoles role) const
{
    QVariant result;
    // 遍历所有轨道查找指定的片段
    for (const auto& track : m_tracks) {
        for (const auto& clip : track->clips) {
            if (clip->id() == clipID) {
                switch (role) {
                    case ClipIdRole:
                        result=clip->id();
                        break;
                    case ClipInRole:
                        result=clip->start();
                        break;
                    case ClipOutRole:
                        result=clip->end();
                        break;
                    case ClipLengthRole:
                        result=clip->end() - clip->start();
                        break;
                    case ClipTypeRole:
                        result=clip->type();
                        break;
                    case ClipShowWidgetRole:
                        result=clip->isEmbedWidget();
                        break;
                    // case ClipResizableRole:
                    //     return QVariant::fromValue(clip->isResizeable());
                    case ClipShowBorderRole:
                        result=clip->isShowBorder();
                        break;
                    case ClipModelRole:
                        result= QVariant::fromValue(clip);
                        break;
                    case ClipOscWidgetsRole:
                        result=QVariant::fromValue(clip->getExternalControlAddressMapping());
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return result;
}

bool BaseTimeLineModel::setClipData(ClipId clipID, TimelineRoles role, QVariant value)
{
    // 遍历所有轨道查找指定的片段
    for (auto& track : m_tracks) {
        for (auto& clip : track->clips) {
            if (clip->id() == clipID) {
                bool success = false;
                switch (role) {
                    case ClipInRole:
                        clip->setStart(value.toInt());
                        success = true;
                        break;
                    case ClipOutRole:
                        clip->setEnd(value.toInt());
                        success = true;
                        break;
                    case ClipShowWidgetRole:
                        clip->setEmbedWidget(value.toBool());
                        success = true;
                        break;
                    // case ClipResizableRole:
                    //     clip->setResizeable(value.toBool());
                    //     success = true;
                    //     break;
                    case ClipShowBorderRole:
                        clip->setShowBorder(value.toBool());
                        success = true;
                        break;
                }
                if (success) {
                    // 发出数据改变信号
                    emit dataChanged(createIndex(0, 0), createIndex(rowCount(), 0));
                    return true;
                }
            }
        }
    }
    return false;
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
        if (role == TimelineRoles::TimelineModelAliasRole) {
            setModelAlias(value.toString());
            return true;
        }
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
                emit dataChanged(index, index, {TimelineRoles::ClipIdRole});
                return true;
            }
            // 设置剪辑是否显示小部件（AbstractClipModel::setEmbedWidget 不发信号，需要 Model 主动通知）
            case TimelineRoles::ClipShowWidgetRole: {
                const bool newShowWidget = value.toBool();
                clip->setEmbedWidget(newShowWidget);
                emit dataChanged(index, index, {TimelineRoles::ClipShowWidgetRole});
                return true;
            }
            // 设置剪辑是否可调整大小（AbstractClipModel::setResizable 不发信号，需要 Model 主动通知）
            case TimelineRoles::ClipResizableRole: {
                const bool newResizable = value.toBool();
                clip->setResizable(newResizable);
                emit dataChanged(index, index, {TimelineRoles::ClipResizableRole});
                return true;
            }
            // 设置剪辑结束时间（clip->setEnd 会触发 lengthChanged，Model 已监听并转为 dataChanged）
            case TimelineRoles::ClipOutRole: {
                const int newEnd = value.toInt();
                clip->setEnd(newEnd);
                return true;
            }
            // 设置剪辑长度（clip->setEnd 会触发 lengthChanged，Model 已监听并转为 dataChanged）
            case TimelineRoles::ClipLengthRole: {
                const int newLength = value.toInt();
                clip->setEnd(clip->start()+newLength);
                return true;
            }
            // 设置剪辑是否显示边框（AbstractClipModel::setShowBorder 不发信号，需要 Model 主动通知）
            case TimelineRoles::ClipShowBorderRole: {
                const bool newShowBorder = value.toBool();
                clip->setShowBorder(newShowBorder);
                emit dataChanged(index, index, {TimelineRoles::ClipShowBorderRole});
                return true;
            }
            // 设置时间码类型（使用 TimeCodeTypeRole 表达，避免误用 ClipTypeRole）
            case TimelineRoles::TimeCodeTypeRole: {
                const TimeCodeType type = static_cast<TimeCodeType>(value.toInt());
                clip->setTimeCodeType(type);
                emit dataChanged(index, index, {TimelineRoles::TimeCodeTypeRole});
                return true;
            }
            default:
                return false;
        }

    }else{
        TrackData* track = static_cast<TrackData*>(index.internalPointer());
        if (!track)
            return false;
        switch (role){
            //设置轨道名称
            case TimelineRoles::TrackNameRole: {
                /**
                 * @brief 更新轨道名称
                 * @details
                 * TrackListView 使用 openPersistentEditor 创建持久编辑器。
                 * 若这里不发 dataChanged，编辑器可能一直显示创建时的旧文本，
                 * 导致“重新导入后顺序对但 TrackName 显示错误”。
                 */
                track->name = value.toString();
                emit dataChanged(index, index, {role});
                return true;
            }
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
    /**
     * @brief 从 JSON 加载时间线模型
     * @details
     * 重新导入后“顺序正确但 TrackName 显示错误”的根因之一是：导入过程触发了大量插入/删除信号，
     * 视图在中间态创建了 persistent editor，随后 trackName 再被 setData 修改，编辑器未及时同步。
     *
     * 这里改为标准的 beginResetModel/endResetModel：
     * - 导入期间视图不处理中间态，避免错位/错名。
     * - 导入完成后一次性刷新视图。
     */

    beginResetModel();

    // 1) 释放旧数据（不走 onDeleteTrack，避免导入过程中发射大量自定义信号）
    for (TrackData* track : m_tracks) {
        if (!track) continue;
        for (AbstractClipModel* clip : track->clips) {
            delete clip;
        }
        track->clips.clear();
        delete track;
    }
    m_tracks.clear();

    // 2) 重置导入相关状态
    m_clipNextId = 0;
    m_timeDisplayFormat = static_cast<TimedisplayFormat>(modelJson["displayFormat"].toInt());

    // 3) 构建新数据
    if (!getPluginLoader()) {
        endResetModel();
        return;
    }

    const QJsonArray trackArray = modelJson["tracks"].toArray();
    for (const QJsonValue &trackValue : trackArray) {
        const QJsonObject trackJson = trackValue.toObject();
        const QString type = trackJson["type"].toString();
        const QString name = trackJson["trackName"].toString();

        auto* track = new TrackData();
        track->type = type;
        track->name = name;

        const QJsonArray clipArray = trackJson["clips"].toArray();
        for (const QJsonValue &clipValue : clipArray) {
            const QJsonObject clipJson = clipValue.toObject();

            AbstractClipModel* clip = getPluginLoader()->createModelForType(
                clipJson["type"].toString(),
                clipJson["start"].toInt()
            );
            if (!clip) {
                continue;
            }

            clip->setTimelineModel(this);
            clip->setTimeCodeType(getTimeCodeType());
            clip->load(clipJson);

            // 更新下一个可用 ID（以 JSON 中的 Id 为准）
            m_clipNextId = qMax(static_cast<qint64>(m_clipNextId), static_cast<qint64>(clipJson["Id"].toInt()) + 1);

            // 连接信号：ClipTimePanel 可能会直接调用 clip->setStart/setEnd
            // 这里将 clip 的信号转换为 Qt 标准的 dataChanged，让 View 走标准刷新路径。
            connect(clip, &AbstractClipModel::lengthChanged, this, [this, clip](int) {
                onUpdateTimeLineLength();
                const QModelIndex idx = indexForClip(clip);
                if (idx.isValid()) {
                    emit dataChanged(idx, idx, {TimelineRoles::ClipOutRole, TimelineRoles::ClipLengthRole});
                }
            });
            connect(clip, &AbstractClipModel::timelinePositionChanged, this, [this, clip](int) {
                onUpdateTimeLineLength();
                const QModelIndex idx = indexForClip(clip);
                if (idx.isValid()) {
                    emit dataChanged(idx, idx, {TimelineRoles::ClipInRole, TimelineRoles::ClipPosRole});
                }
            });

            track->clips.push_back(clip);
        }

        m_tracks.push_back(track);
    }

    endResetModel();

    // 4) 更新时间线长度（会在变化时发出 S_LengthChanged）
    onUpdateTimeLineLength();
}
// 计算时间线长度
qint64 BaseTimeLineModel::onUpdateTimeLineLength()
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
    return m_lengthFrame;
}

void BaseTimeLineModel::onDeleteTrack(int trackIndex) {
    /**
     * @brief 删除轨道
     * @details
     * 旧实现的问题：
     * - 通过 createIndex(0,0,clip) 传给 onDeleteClip，parent 无效，导致 onDeleteClip 直接 return，片段不会被释放。
     * - 结果是删除轨道时 clip 内存泄漏，并且视图/选择状态也可能出现悬挂指针。
     *
     * 新实现策略：
     * - 删除轨道行时（beginRemoveRows/endRemoveRows）由父行移除负责通知视图；
     * - 在移除轨道前，直接释放该轨道拥有的所有 clip 对象，确保内存与指针安全。
     */

    if (trackIndex < 0 || trackIndex >= m_tracks.size()) {
        qDebug() << "Invalid track index";
        return;
    }

    beginRemoveRows(QModelIndex(), trackIndex, trackIndex);

    TrackData* track = m_tracks[trackIndex];
    if (track) {
        // 释放该轨道上的所有片段
        for (AbstractClipModel* clip : track->clips) {
            delete clip;
        }
        track->clips.clear();
    }

    // 从容器中移除轨道并释放轨道对象
    m_tracks.removeAt(trackIndex);
    delete track;

    endRemoveRows();
}  
// 通过类型名创建轨道
void BaseTimeLineModel::onAddTrack(const QString& type) {
    if (!m_pluginLoader) return;
        
    beginInsertRows(QModelIndex(), m_tracks.size(), m_tracks.size());
    auto track = new TrackData();
    track->type = type;
    track->name = type+" " + QString::number(m_tracks.size());
    m_tracks.push_back(std::move(track));
    endInsertRows();
}

void BaseTimeLineModel::onMoveTrack(int sourceRow, int targetRow) {
    /**
     * @brief 移动轨道
     * @details
     * 使用 Qt 标准的 beginMoveRows/endMoveRows 通知视图更新，
     * 避免仅依赖自定义信号导致视图状态（选择、持久编辑器、滚动位置）不同步。
     */
    if (sourceRow < 0 || sourceRow >= m_tracks.size()) {
        return;
    }
    if (targetRow < 0 || targetRow >= m_tracks.size()) {
        return;
    }
    if (sourceRow == targetRow) {
        return;
    }

    // Qt 约定：当在同一 parent 内向下移动时，destinationRow 需要 +1（因为 source 行先被移除）
    const int destinationRow = (sourceRow < targetRow) ? (targetRow + 1) : targetRow;

    beginMoveRows(QModelIndex(), sourceRow, sourceRow, QModelIndex(), destinationRow);
    m_tracks.move(sourceRow, targetRow);
    endMoveRows();


}

void BaseTimeLineModel::onAddClip(int trackIndex, int startFrame) {
    /**
     * @brief 在指定轨道添加片段
     * @details
     * 本次重构的核心目标是“让 Qt Model/View 的绑定回到标准路径”：
     * - 插入子项必须使用 beginInsertRows/endInsertRows 通知视图，而不是仅依赖自定义信号全局刷新。
     * - index/createIndex 的 row 必须是“片段行号”，旧代码用 trackIndex 作为 row 创建 clip 索引，会造成索引语义错误。
     */

    if (trackIndex < 0 || trackIndex >= m_tracks.size()) {
        qDebug() << "Invalid track index:" << trackIndex;
        return;
    }
    if (!getPluginLoader()) {
        qDebug() << "Plugin loader is not initialized";
        return;
    }

    TrackData* track = m_tracks[trackIndex];
    if (!track) {
        qDebug() << "Track pointer is null";
        return;
    }

    AbstractClipModel* newClip = getPluginLoader()->createModelForType(track->type, startFrame);
    if (!newClip) {
        qDebug() << "Failed to create clip model";
        return;
    }

    newClip->setTimelineModel(this);
    newClip->setTimeCodeType(getTimeCodeType());

    // 连接信号：ClipTimePanel 可能会直接调用 clip->setStart/setEnd
    // 这里将 clip 的信号转换为 Qt 标准的 dataChanged，让 View 走标准刷新路径。
    connect(newClip, &AbstractClipModel::lengthChanged, this, [this, newClip](int) {
        onUpdateTimeLineLength();
        const QModelIndex idx = indexForClip(newClip);
        if (idx.isValid()) {
            emit dataChanged(idx, idx, {TimelineRoles::ClipOutRole, TimelineRoles::ClipLengthRole});
        }
    });
    connect(newClip, &AbstractClipModel::timelinePositionChanged, this, [this, newClip](int) {
        onUpdateTimeLineLength();
        const QModelIndex idx = indexForClip(newClip);
        if (idx.isValid()) {
            emit dataChanged(idx, idx, {TimelineRoles::ClipInRole, TimelineRoles::ClipPosRole});
        }
    });

    // 插入到 track->clips 的尾部
    const int insertRow = track->clips.size();
    const QModelIndex trackModelIndex = index(trackIndex, 0, QModelIndex());

    beginInsertRows(trackModelIndex, insertRow, insertRow);
    track->clips.push_back(newClip);
    endInsertRows();

    // 为新片段设置唯一 ID（使用标准 index(row,col,parent) 获取正确索引）
    const QModelIndex clipIndex = index(insertRow, 0, trackModelIndex);
    setData(clipIndex, QVariant::fromValue(m_clipNextId++), TimelineRoles::ClipIdRole);

    onUpdateTimeLineLength();
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
        AbstractClipModel* clip = track->clips[clipIndex.row()];
        // 从轨道中移除片段
        track->clips.erase(track->clips.begin() + clipIndex.row());
        // 删除片段对象，释放内存
        delete clip;

    }

    // 结束删除行
    endRemoveRows();
    //更新时间线长度
    onUpdateTimeLineLength();
}

/**
 * @brief 通过 clip 指针反查其在模型中的索引
 * @param clip 片段指针
 * @return clip 对应的 QModelIndex；若未找到返回无效索引
 */
QModelIndex BaseTimeLineModel::indexForClip(AbstractClipModel* clip) const
{
    if (!clip) {
        return QModelIndex();
    }

    TrackData* track = findParentTrackOfClip(clip);
    if (!track) {
        return QModelIndex();
    }

    const int trackRow = findTrackRow(track);
    if (trackRow < 0) {
        return QModelIndex();
    }

    int clipRow = -1;
    for (int i = 0; i < track->clips.size(); ++i) {
        if (track->clips[i] == clip) {
            clipRow = i;
            break;
        }
    }
    if (clipRow < 0) {
        return QModelIndex();
    }

    const QModelIndex trackIndex = index(trackRow, 0, QModelIndex());
    return index(clipRow, 0, trackIndex);
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
    /**
     * @brief 析构函数
     * @details
     * 当前数据结构使用裸指针（TrackData* / AbstractClipModel*），因此必须显式释放。
     * 旧代码仅 m_tracks.clear() 会导致轨道与片段全部泄漏。
     */
    for (TrackData* track : m_tracks) {
        if (!track) continue;
        for (AbstractClipModel* clip : track->clips) {
            delete clip;
        }
        track->clips.clear();
        delete track;
    }
    m_tracks.clear();

    delete m_pluginLoader;
    m_pluginLoader = nullptr;
}

int BaseTimeLineModel::getTrackCount() const {
    return m_tracks.size();
}


