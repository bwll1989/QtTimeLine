#ifndef VIDEOCLIPMODEL_H
#define VIDEOCLIPMODEL_H
#include <QPushButton>
#include "../../AbstractClipModel.hpp"
#include <QJsonArray>
class VideoClipModel : public AbstractClipModel {
    Q_OBJECT
public:
VideoClipModel(int start): AbstractClipModel(start, "Video"),
    m_editor(nullptr)
{
    RESIZEABLE = true;
    EMBEDWIDGET = true;
    SHOWBORDER = true;
   
    // 片段正常颜色
    ClipColor=QColor("#CC0033");

}
virtual ~VideoClipModel() override =default;
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
            return QVariant::fromValue(static_cast<AbstractClipModel*>(const_cast<VideoClipModel*>(this)));
        default:
            return AbstractClipModel::data(role);
    }
}


QVariantMap currentData(int currentFrame) const override {
    // if(currentFrame >= m_start && currentFrame <= m_end){
    //     const_cast<VideoClipModel*>(this)->Video();
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
    auto* sendButton = new QPushButton("测试发送", m_editor);

    playLayout->addWidget(sendButton);
    mainLayout->addWidget(playGroup);
    return m_editor;
}

private:
    QWidget* m_editor;

};

#endif // TRIGGERCLIPMODEL_H 