// ClipTimePanel.cpp
// 函数说明：实现 ClipTimePanel 构造逻辑，布局“时间属性”控件并保持原有功能：
// - 显示为 00:00:00:000（毫秒格式）
// - 文本框 textChanged 与 spinbox 一样调用 setStart/setEnd
// - 响应 AbstractClipModel 的 timelinePositionChanged/lengthChanged 信号更新 UI

#include "ClipTimePanel.hpp"
#include "AbstractClipModel.hpp"
#include "TimeCodeDefines.h"
#include <QObject>
#include <QString>

ClipTimePanel::ClipTimePanel(AbstractClipModel* owner, QWidget* parent)
    : m_owner(owner),
    QGroupBox(parent)
{
    setTitle(tr("时间属性"));
    m_timeLayout = new QGridLayout(this);

    // 开始帧显示
    auto startLabel = new QLabel(tr("开始帧:"), this);
    m_startFrameSpinBox = new QSpinBox(this);
    m_startFrameSpinBox->setRange(0, 9999999);
    // m_startFrameSpinBox->setValue(m_owner->start());
    m_startTimeCodeLineEdit = new QTimeEdit(this);
    m_startTimeCodeLineEdit->setDisplayFormat("hh:mm:ss:zzz");
    m_timeLayout->addWidget(startLabel, 0, 0,1,1);
    m_timeLayout->addWidget(m_startFrameSpinBox, 0, 1,1,1);
    m_timeLayout->addWidget(m_startTimeCodeLineEdit, 0, 2,1,1);

    // 片段总长显示（绝对时间：毫秒）
    auto lengthLabel = new QLabel(tr("片段位置:"), this);
    m_positionFrameSpinBox = new QSpinBox(this);
    m_positionFrameSpinBox->setRange(0, 9999999);
    m_positionFrameLineEdit = new QTimeEdit(this);
    m_positionFrameLineEdit->setDisplayFormat("hh:mm:ss:zzz");
    m_timeLayout->addWidget(lengthLabel, 1, 0,1,1);
    m_timeLayout->addWidget(m_positionFrameSpinBox, 1 ,1,1,1);
    m_timeLayout->addWidget(m_positionFrameLineEdit, 1, 2,1,1);

    // 结束帧显示（绝对时间：毫秒）
    auto endLabel = new QLabel(tr("结束帧:"), this);
    m_endFrameSpinBox = new QSpinBox(this);
    m_endFrameSpinBox->setRange(0, 9999999);
    m_endTimeCodeLineEdit = new QTimeEdit(this);
    m_endTimeCodeLineEdit->setDisplayFormat("hh:mm:ss:zzz");
    m_timeLayout->addWidget(endLabel, 2, 0,1,1);
    m_timeLayout->addWidget(m_endFrameSpinBox, 2, 1,1,1);
    m_timeLayout->addWidget(m_endTimeCodeLineEdit, 2, 2,1,1);
    updateStartFrame(m_owner->start());
    updateEndFrame(m_owner->end());
    updatePositionFrame(m_owner->start());
    initSignalConnections();
}
void ClipTimePanel::initSignalConnections() {
    connect(m_startFrameSpinBox,&QSpinBox::valueChanged,this,&ClipTimePanel::updateStartFrame);
    connect(m_startTimeCodeLineEdit,&QTimeEdit::timeChanged,this,&ClipTimePanel::updateStartTimeCode);
    connect(m_endFrameSpinBox,&QSpinBox::valueChanged,this,&ClipTimePanel::updateEndFrame);
    connect(m_endTimeCodeLineEdit,&QTimeEdit::timeChanged,this,&ClipTimePanel::updateEndTimeCode);
    connect(m_positionFrameSpinBox,&QSpinBox::valueChanged,this,&ClipTimePanel::updatePositionFrame);
    connect(m_positionFrameLineEdit,&QTimeEdit::timeChanged,this,&ClipTimePanel::updatePositionTimeCode);
    connect(m_owner, &AbstractClipModel::timelinePositionChanged, this, [this](int frames){
        updateStartFrame(frames);
        updatePositionFrame(frames);
    });
    connect(m_owner, &AbstractClipModel::lengthChanged, this, [this](int frames){
       updateEndFrame(frames);
    });

}

void ClipTimePanel::updateStartFrame(int frames) {
    if (!m_owner) {
        qDebug() << "m_owner is nullptr";
        return;
    }
    QSignalBlocker b1(m_startFrameSpinBox);
    QSignalBlocker b2(m_startTimeCodeLineEdit);
    m_startFrameSpinBox->setValue(frames);
    auto tc = frames_to_timecode_frame(frames, m_owner->getTimeCodeType());
    QTime time=timecode_frame_to_qtime(tc, m_owner->getTimeCodeType());
    m_startTimeCodeLineEdit->setTime(time);

    m_owner->setStart(frames);
}

void ClipTimePanel::updateStartTimeCode(const QTime& tc) {
    if (!m_owner) {
        qDebug() << "m_owner is nullptr";
        return;
    }
    QSignalBlocker b1(m_startFrameSpinBox);
    QSignalBlocker b2(m_startTimeCodeLineEdit);
    TimeCodeFrame timecode = qtime_to_timecode_frame(tc, m_owner->getTimeCodeType());
    int newStart = static_cast<int>(timecode_frame_to_frames(timecode, m_owner->getTimeCodeType()));
    m_startFrameSpinBox->setValue(newStart);
    m_owner->setStart(newStart);

}
void ClipTimePanel::updateEndFrame(int frames) {
    if (!m_owner) {
        return;
    }
    QSignalBlocker b1(m_endFrameSpinBox);
    QSignalBlocker b2(m_endTimeCodeLineEdit);
    m_endFrameSpinBox->setValue(frames);
    auto tc = frames_to_timecode_frame(frames, m_owner->getTimeCodeType());
    QTime time=timecode_frame_to_qtime(tc, m_owner->getTimeCodeType());
    m_endTimeCodeLineEdit->setTime(time);
    m_owner->setEnd(frames);
}
void ClipTimePanel::updateEndTimeCode(const QTime& tc) {
    if (!m_owner) {
        qDebug() << "m_owner is nullptr";
        return;
    }
    QSignalBlocker b1(m_endFrameSpinBox);
    QSignalBlocker b2(m_endTimeCodeLineEdit);
    TimeCodeFrame timecode = qtime_to_timecode_frame(tc, m_owner->getTimeCodeType());
    int newEnd = static_cast<int>(timecode_frame_to_frames(timecode, m_owner->getTimeCodeType()));
    m_endFrameSpinBox->setValue(newEnd);
    m_owner->setEnd(newEnd);
}
void ClipTimePanel::updatePositionFrame(int frames) {
    if (!m_owner) {
        qDebug() << "m_owner is nullptr";
        return;
    }
    int length=m_owner->length();
    QSignalBlocker b1(m_positionFrameSpinBox);
    QSignalBlocker b2(m_positionFrameLineEdit);
    m_positionFrameSpinBox->setValue(frames);
    auto tc = frames_to_timecode_frame(frames, m_owner->getTimeCodeType());
    QTime time=timecode_frame_to_qtime(tc, m_owner->getTimeCodeType());
    m_positionFrameLineEdit->setTime(time);
    m_owner->setStart(frames);
    m_owner->setEnd(frames+length);
}
void ClipTimePanel::updatePositionTimeCode(const QTime& tc) {
    if (!m_owner) {
        qDebug() << "m_owner is nullptr";
        return;
    }
    int length=m_owner->length();
    QSignalBlocker b1(m_positionFrameSpinBox);
    QSignalBlocker b2(m_positionFrameLineEdit);
    TimeCodeFrame timecode = qtime_to_timecode_frame(tc, m_owner->getTimeCodeType());
    int newEnd = static_cast<int>(timecode_frame_to_frames(timecode, m_owner->getTimeCodeType()));
    m_positionFrameSpinBox->setValue(newEnd);
    m_owner->setStart(newEnd);
    m_owner->setEnd(newEnd+length);
}