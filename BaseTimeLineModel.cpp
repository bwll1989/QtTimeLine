#include "BaseTimeLineModel.h"


BaseTimelineModel::BaseTimelineModel(QObject* parent) : QAbstractItemModel(parent), m_pluginLoader(nullptr)
{
    m_pluginLoader = new BasePluginLoader();
    m_pluginLoader->loadPlugins();
    // qRegisterMetaType<AbstractClipModel*>();

    //轨道移动后更新当前帧数据
    // connect(this, &BaseTimelineModel::S_trackMoved, this, [this](int oldindex,int newindex){
    //     onCreateCurrentVideoData(this->getPlayheadPos());
    // });
    // //删除轨道后更新当前帧数据
    // connect(this, &BaseTimelineModel::S_trackDelete, this, [this](){
    //     onCreateCurrentVideoData(this->getPlayheadPos());
    // });
    // //删除片段后更新当前帧数据
    // connect(this, &BaseTimelineModel::S_deleteClip, this, [this](){
    //     onCreateCurrentVideoData(this->getPlayheadPos());
    // });
}
// 获取播放头位置
qint64 BaseTimelineModel::getPlayheadPos() const{
    return m_currentFrame;
}
// 设置播放头位置
void BaseTimelineModel::onSetPlayheadPos(int newPlayheadPos)
{
    m_currentFrame = newPlayheadPos;
    emit S_playheadMoved(newPlayheadPos);

}

QModelIndex BaseTimelineModel::index(int row, int column, const QModelIndex &parent) const
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
        BaseTrackModel* track = static_cast<BaseTrackModel*>(parent.internalPointer());
        if (track && row < track->getClips().size()) {
            return createIndex(row, column, track->getClips()[row]);
        }
    }
    return QModelIndex();
}

QModelIndex BaseTimelineModel::parent(const QModelIndex &child) const
{
    // 如果子索引无效，返回空索引
    if (!child.isValid()) {
        return QModelIndex();
    }

    // Check if the child is a ClipModel
    AbstractClipModel* clip = static_cast<AbstractClipModel*>(child.internalPointer());
    if (clip) {
        // 获取剪辑所在轨道
        BaseTrackModel* track = findParentTrackOfClip(clip);
        if (track) {
            // 获取轨道行
            int trackRow = findTrackRow(track);
            // 创建索引
            return createIndex(trackRow, 0, track);
        }
    }

    return QModelIndex();
}

int BaseTimelineModel::rowCount(const QModelIndex &parent) const
{
    // 如果父索引无效，返回轨道数量
    if (!parent.isValid()) {
        // Root level: number of tracks
        return m_tracks.size();
    }
    // 获取轨道
    BaseTrackModel* track = static_cast<BaseTrackModel*>(parent.internalPointer());
    if (track) {
        // 返回剪辑数量
        return track->getClips().size();
    }

    return 0;
}

int BaseTimelineModel::columnCount(const QModelIndex &parent) const
{
    // 如果父索引无效，返回1
    Q_UNUSED(parent);
    return 1; // Only one column for both tracks and clips
}

QVariant BaseTimelineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        switch (role) {
            case TimelineLengthRole:
                return QVariant::fromValue(12345);
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
        BaseTrackModel* track = static_cast<BaseTrackModel*>(index.internalPointer());
            switch (role){
            //获取轨道工具提示
            case Qt::ToolTipRole:
                return QVariant::fromValue("track " + QString::number(index.row()));
            //获取轨道编号
            case TrackNumberRole:
                return QVariant::fromValue(index.row());
            //获取轨道类型
            case TrackTypeRole:
                return QVariant::fromValue(TrackTypeString(track->type()));
            //获取轨道名称
            case TrackNameRole:
                return QVariant::fromValue(track->getName());
            //获取轨道模型
            case TrackModelRole:
                return QVariant::fromValue(track);
            //获取轨道剪辑数量
            case TrackClipsCountRole:
                return QVariant::fromValue(track->getClips().size());
            //获取轨道长度
            case TrackLengthRole:
                return QVariant::fromValue(track->getTrackLength());
            //获取轨道剪辑列表
            case TrackClipsRole:
                return QVariant::fromValue(track->getClips());
            default:
                return QVariant();
        }
    }
    return QVariant();
}

void BaseTimelineModel::onStartPlay(){
   qDebug()<<"onStartPlay";
}

void BaseTimelineModel::onPausePlay(){
    qDebug()<<"onPausePlay";
}

void BaseTimelineModel::onStopPlay(){
   qDebug()<<"onStopPlay";
}

// 设置数据
bool BaseTimelineModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
                //            emit dataChanged(index, index);
                return true;
            }
                // 设置剪辑结束时间
            case TimelineRoles::ClipOutRole: {
                int newEnd = value.toInt();
                clip->setEnd(newEnd);
                //            emit dataChanged(index, index);
                //            onTimelineLengthChanged();
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
        BaseTrackModel* track = static_cast<BaseTrackModel*>(index.internalPointer());
        if (!track)
            return false;
        switch (role){
            //设置轨道名称
            case TimelineRoles::TrackNameRole:
                track->setName(value.toString());
                return true;
            default:
                return false;
        }

    }
}

// 获取支持的拖放操作
Qt::DropActions BaseTimelineModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

// 获取项目标志
Qt::ItemFlags BaseTimelineModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled; 
}


void BaseTimelineModel::clear(){
    while(rowCount()>0){
        onDeleteTrack(0);
    }
    
}

// 保存模型
QJsonObject BaseTimelineModel::save() const {
    QJsonObject modelJson;
    QJsonArray trackArray;
    // auto timecodeType = m_timecodeGenerator->getTimecodeType();
    // modelJson["timecodeType"] = static_cast<int>(timecodeType);
    modelJson["displayFormat"] = static_cast<int>(m_timeDisplayFormat);
    // modelJson["clockSource"] = static_cast<int>(m_timecodeGenerator->getClockSource());
    //轮询所有轨道
    for (BaseTrackModel* track : m_tracks) {
        QJsonObject trackJson = track->save();
        QJsonArray clipArray;
        QVector<AbstractClipModel*> clips = track->getClips();
        //轮询轨道下所有片段
        for (AbstractClipModel* clip : clips) {
            clipArray.append(clip->save());
        }
        trackJson["clips"] = clipArray;
        trackArray.append(trackJson);
    }
    
    modelJson["tracks"] = trackArray;


    // 保存舞台信息

    return modelJson;
}

void BaseTimelineModel::load(const QJsonObject &modelJson) {
    clear();
    // 设置时间显示格式
    m_timeDisplayFormat = static_cast<TimedisplayFormat>(modelJson["displayFormat"].toInt());


    // 加载轨道
    QJsonArray trackArray = modelJson["tracks"].toArray();
    for (const QJsonValue &trackValue : trackArray) {
        QJsonObject trackJson = trackValue.toObject();
        QString type = trackJson["type"].toString();
        BaseTrackModel* track = new BaseTrackModel(trackJson["trackIndex"].toInt(), type, this);
        onAddTrack(track);
        
        // 加载轨道中的片段
        QJsonArray clipArray = trackJson["clips"].toArray();
        for(const QJsonValue &clipValue : clipArray) {
            QJsonObject clipJson = clipValue.toObject();
            AbstractClipModel* clip = getPluginLoader()->createModelForType(
                clipJson["type"].toString(), 
                clipJson["start"].toInt()
            );
            clip->setTrackIndex(track->trackIndex());
            clip->load(clipJson);
            onAddClip(clip);
        }
    }
    emit S_trackAdd();



    emit S_timelineUpdated();
}
// 计算时间线长度
void BaseTimelineModel::onTimelineLengthChanged()
{
    // qDebug()<<"onTimelineLengthChanged";
    int max = 0;
    for(BaseTrackModel* track : m_tracks){
        max = qMax(max,track->getTrackLength());
    }

}

void BaseTimelineModel::onDeleteTrack(int trackIndex) {

    if (trackIndex < 0 || trackIndex >= m_tracks.size()) {
        qDebug() << "Invalid track index";
        return;
    }

    beginRemoveRows(QModelIndex(), trackIndex, trackIndex); // 开始移除行
    // 删除轨道上的所有片段
    for (AbstractClipModel* clip : m_tracks[trackIndex]->getClips()) {
        onDeleteClip(createIndex(0, 0, clip));
    }
    delete m_tracks[trackIndex]; // 删除轨道对象
    m_tracks.erase(m_tracks.begin() + trackIndex); // 从列表中移除轨道
    for (int i = trackIndex; i < m_tracks.size(); ++i) {
        m_tracks[i]->onSetTrackIndex(i);
    }

    endRemoveRows(); // 结束移除行
    emit S_trackDelete();

} 
// 通过类型名创建轨道
void BaseTimelineModel::onAddTrack(const QString& type) {
    if (!m_pluginLoader) return;
        
        beginInsertRows(QModelIndex(), m_tracks.size(), m_tracks.size());
        BaseTrackModel* track = new BaseTrackModel(m_tracks.size(), type);
        connect(track, &BaseTrackModel::S_trackLengthChanged, this, &BaseTimelineModel::onTimelineLengthChanged);
        m_tracks.push_back(track);
        endInsertRows();
        emit S_trackAdd();
    }

void BaseTimelineModel::onAddTrack(BaseTrackModel* track) {
    auto trackIndex =track->trackIndex();
    beginInsertRows(QModelIndex(), trackIndex, trackIndex);
    m_tracks.insert(trackIndex, track);
    connect(track, &BaseTrackModel::S_trackLengthChanged, this, &BaseTimelineModel::onTimelineLengthChanged);
    // Update indices of tracks after insertion point
    for (int i = trackIndex + 1; i < m_tracks.size(); ++i) {
        m_tracks[i]->onSetTrackIndex(i);
    }
    endInsertRows();
    emit S_trackAdd();
}

void BaseTimelineModel::onMoveTrack(int sourceRow, int targetRow) {
    m_tracks.move(sourceRow, targetRow);
    // 更新所有轨道的索引
    for (int i = 0; i < m_tracks.size(); ++i) {
        m_tracks[i]->onSetTrackIndex(i);
    }
    emit S_trackMoved(sourceRow,targetRow);
}

void BaseTimelineModel::onAddClip(int trackIndex, int startFrame) {
    AbstractClipModel* newClip = getPluginLoader()->createModelForType(getTracks()[trackIndex]->type(), startFrame);
    connect(newClip, &AbstractClipModel::lengthChanged, [this]() {emit S_addClip();});
    connect(newClip, &AbstractClipModel::timelinePositionChanged, [this]() {emit S_addClip();});
    getTracks()[trackIndex]->onAddClip(newClip);
    emit S_addClip();
}
//通过片段模型添加片段
void BaseTimelineModel::onAddClip(AbstractClipModel* newClip){
    getTracks()[newClip->trackIndex()]->onAddClip(newClip);
    emit S_addClip();
}

void BaseTimelineModel::onDeleteClip(QModelIndex clipIndex){
     if (!clipIndex.isValid() || !clipIndex.parent().isValid())
        return;

    // 获取轨道索引和轨道
    QModelIndex trackIndex = clipIndex.parent();
    BaseTrackModel* track = m_tracks[trackIndex.row()];
    if (!track)
        return;
    // 开始删除行
    beginRemoveRows(trackIndex, clipIndex.row(), clipIndex.row());

    // 获取要删除的片段
    AbstractClipModel* clip = track->getClips()[clipIndex.row()];

    // 从轨道中移除片段
    track->removeClip(clip);
    // 删除片段对象
    // delete clip;

    // 结束删除行
    endRemoveRows();
    emit S_deleteClip();
}

BaseTrackModel* BaseTimelineModel::findParentTrackOfClip(AbstractClipModel* clip) const {
    for (BaseTrackModel* track : m_tracks) {
        QVector<AbstractClipModel*>& clips = track->getClips();
        if (std::find(clips.begin(), clips.end(), clip) != clips.end()) {
            return track;
        }
    }
    return nullptr; // Return nullptr if no parent track is found
}
// 查找轨道行
int BaseTimelineModel::findTrackRow(BaseTrackModel* track) const {
    auto it = std::find(m_tracks.begin(), m_tracks.end(), track);
    if (it != m_tracks.end()) {
        return std::distance(m_tracks.begin(), it);
    }
    return -1; // Return -1 if the track is not found
}

void BaseTimelineModel::setPluginLoader(BasePluginLoader* loader) {
    m_pluginLoader = loader;
}

BasePluginLoader* BaseTimelineModel::getPluginLoader() const {
    return m_pluginLoader;
}

// 在析构函数中清理
BaseTimelineModel::~BaseTimelineModel()
{

    // for (BaseTrackModel* track : m_tracks) {
    //     onDeleteTrack(track->trackIndex());
    // }
    for (int i = 0; i < m_tracks.size(); ++i) {
        delete m_tracks[i];
    }
    m_tracks.clear();

    delete m_pluginLoader;
}

int BaseTimelineModel::getTrackCount() const {
    return m_tracks.size();
}


