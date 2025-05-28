
#include "BaseTrackModel.h"

BaseTrackModel::BaseTrackModel(int number, const QString& type, QObject* parent)
        : QObject(parent),
        m_type(type),
        m_name(type),
        m_trackIndex(number),
		m_trackLength(0) // 初始化轨道长度为0
    {
        connect(this, &BaseTrackModel::S_trackAddClip, this, &BaseTrackModel::onCalculateTrackLength);
        connect(this, &BaseTrackModel::S_trackDeleteClip, this, &BaseTrackModel::onCalculateTrackLength);
    }

BaseTrackModel::~BaseTrackModel() {
    for(AbstractClipModel* clip : m_clips){
        delete clip;
    }
    m_clips.clear();
}

void BaseTrackModel::removeClip(AbstractClipModel* clip) {
    auto it = std::find(m_clips.begin(), m_clips.end(), clip);
    if(it != m_clips.end()) {
        m_clips.erase(it);
        delete clip;
        // 发出信号
        emit S_trackDeleteClip();
    }
}

QVector<AbstractClipModel*>& BaseTrackModel::getClips(){
    return m_clips;
}
 
QString BaseTrackModel::type() const{
    return m_type;
}

int BaseTrackModel::trackIndex() const{
    return m_trackIndex;
}

QJsonObject BaseTrackModel::save() const
{
    QJsonObject trackJson;
    trackJson["type"] = m_type;
    trackJson["trackIndex"] = m_trackIndex;
    trackJson["trackLength"] = m_trackLength;
    trackJson["trackName"] = m_name;
    return trackJson;
}
 
qint64 BaseTrackModel::getTrackLength() const {
    return m_trackLength;
}

void BaseTrackModel::setName(const QString& name){
    m_name = name;
}

QString BaseTrackModel::getName() const{
    return m_name;
}

void BaseTrackModel::load(const QJsonObject &trackJson, BasePluginLoader* pluginLoader) {
    if (!pluginLoader) return;
    
    m_type = trackJson["type"].toString();
    m_trackIndex = trackJson["trackIndex"].toInt();
    m_trackLength = trackJson["trackLength"].toInt();
    m_name = trackJson["trackName"].toString();
    
}


void BaseTrackModel::onCalculateTrackLength(){
    qint64 length = 0;
    for(AbstractClipModel* clip : m_clips){
        length = qMax(length, clip->end()+1);
    }
    if(length != m_trackLength){
        m_trackLength = length;
        emit S_trackLengthChanged(length);
    }
}

void BaseTrackModel::onSetTrackIndex(int index){
    int oldindex=m_trackIndex;
    m_trackIndex = index;
    for(AbstractClipModel* clip : m_clips){
        qDebug() << "设置了新的轨道索引"<<index;
        clip->setTrackIndex(index);
    }
    emit S_trackIndexChanged(oldindex,index);
}
  
void BaseTrackModel::onAddClip(AbstractClipModel* newClip) {

    //片段长度变化时更新轨道长度
    connect(newClip, &AbstractClipModel::lengthChanged, this, &BaseTrackModel::onCalculateTrackLength);
    if (newClip) {
        
        qDebug() << "添加了新的剪辑"<<nextId;
        newClip->setTrackIndex(m_trackIndex);
        newClip->setId(nextId);
        m_clips.push_back(newClip);
        nextId++;
        emit S_trackAddClip();
    }

}

