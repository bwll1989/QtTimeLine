#ifndef CONTROLCLIPMODEL_HPP
#define CONTROLCLIPMODEL_HPP
#include <QPushButton>
#include "../../AbstractClipModel.h"
#include <QJsonArray>
class ControlClipModel : public AbstractClipModel {
    Q_OBJECT
public:


ControlClipModel(int start): AbstractClipModel(start, "Control"),
    m_editor(nullptr)
{
    RESIZEABLE = true;
    EMBEDWIDGET = false;
    SHOWBORDER = true;
   
    // 片段正常颜色
    ClipColor=QColor("#6a2c70");

}
~ControlClipModel() override =default;
// 重写保存和加载函数
QJsonObject save() const override {
    QJsonObject json = AbstractClipModel::save();
    return json;
}

void load(const QJsonObject& json) override {
    AbstractClipModel::load(json);
}

QVariant data(int role) const override {
    switch (role) {
        case TimelineRoles::ClipModelRole:
            return QVariant::fromValue(static_cast<AbstractClipModel*>(const_cast<ControlClipModel*>(this)));
        default:
            return AbstractClipModel::data(role);
    }
}


QVariantMap currentData(int currentFrame) const override {
    // if(currentFrame >= m_start && currentFrame <= m_end){
    //     const_cast<ControlClipModel*>(this)->Control();
    // }
    return QVariantMap();
}

QWidget* clipPropertyWidget() override{
    m_editor = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_editor);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(4);
        // OSC消息设置
    auto* playGroup = new QGroupBox("触发设置", m_editor);
    auto* playLayout = new QVBoxLayout(playGroup);
    playLayout->setContentsMargins(0, 0, 0, 0);
    // 测试按钮
    auto* sendButton = new QPushButton("control发送", m_editor);

    playLayout->addWidget(sendButton);
    mainLayout->addWidget(playGroup);
    return m_editor;
}

private:
    QWidget* m_editor;

};

#endif // TRIGGERCLIPMODEL_H 