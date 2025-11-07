// ClipTimeGroupBox
// 类说明：封装“时间属性”界面，将原先 AbstractClipModel::initPropertyWidget 中的 timeGroupBox 独立为 QGroupBox 子类。
// 职责：构建开始/结束/总长的控件，统一显示为绝对时间（毫秒）00:00:00:000；
//       管理信号连接：文本框修改触发 setStart/setEnd，与 spinbox 行为一致；响应模型的时间/长度变化信号同步更新 UI。

#pragma once
#include "TimelineExports.hpp"
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include "TimeCodeDefines.h"
#include <QTimeEdit>
class AbstractClipModel;
class NODE_TIMELINE_PUBLIC ClipTimePanel : public QGroupBox {
public:
    // 构造函数：创建并布置 UI，连接信号到 AbstractClipModel，保持原有功能
    explicit ClipTimePanel(AbstractClipModel* owner, QWidget* parent = nullptr);
    void initSignalConnections();
public:
    QSpinBox*   m_startFrameSpinBox {nullptr};
    QTimeEdit*  m_startTimeCodeLineEdit {nullptr};

    QSpinBox*   m_positionFrameSpinBox {nullptr};
    QTimeEdit*  m_positionFrameLineEdit {nullptr};

    QSpinBox*   m_endFrameSpinBox {nullptr};
    QTimeEdit*  m_endTimeCodeLineEdit {nullptr};

Q_SIGNALS:
    void startFrameChanged(int value);
    void positionChanged(int value);
    void endFrameChanged(int value);
public Q_SLOTS:
    void updateStartFrame(int frames);
    void updateStartTimeCode(const QTime& tc);
    void updateEndFrame(int frames);
    void updateEndTimeCode(const QTime& tc);
    void updatePositionFrame(int frames);
    void updatePositionTimeCode(const QTime& tc);
private:
    AbstractClipModel* m_owner {nullptr};
    // 本地控件（不强制写回到 AbstractClipModel 的成员，避免循环依赖）
    QGridLayout* m_timeLayout {nullptr};
    QLabel*      m_startFrameLabel {nullptr};
    QLabel*      m_lengthFrameLabel {nullptr};
    QLabel*      m_endFrameLabel {nullptr};

};