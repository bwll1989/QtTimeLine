#pragma once
#include <QPushButton>
#include "../../AbstractClipModel.hpp"
#include <QJsonArray>
class TriggerClipModel : public AbstractClipModel {
    Q_OBJECT
public:
TriggerClipModel(int start): AbstractClipModel(start, "Trigger"),
    m_editor(nullptr)
{
    RESIZEABLE = false;
    EMBEDWIDGET = false;
    SHOWBORDER = true;
   
    // 片段正常颜色。写在片段自己身上，避免改掉全局 ClipColor。
    setColor(QColor("#CC0033"));
    qDebug()<<"trigger clip model created"<<m_id;
}
virtual ~TriggerClipModel() override =default;
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
            return QVariant::fromValue(static_cast<AbstractClipModel*>(const_cast<TriggerClipModel*>(this)));
        default:
            return AbstractClipModel::data(role);
    }
}


QVariantMap currentData(int currentFrame) const override {
    // if(currentFrame >= m_start && currentFrame <= m_end){
    //     const_cast<TriggerClipModel*>(this)->Video();
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
    connect(sendButton, &QPushButton::clicked, [=]() {
        qDebug()<<"test";
    });
    playLayout->addWidget(sendButton);
    mainLayout->addWidget(playGroup);
    return m_editor;
}

void paint(QPainter* painter, const QRect& rect, bool selected) const override{
    painter->save();  // 保存状态
    painter->setRenderHint(QPainter::Antialiasing);
    // 设置画笔和画刷
    const QColor fill = color();
    if (selected) {
        painter->setBrush(fill.darker(150));
    } else {
        painter->setBrush(fill);
    }

    // 设置细边框
    if(SHOWBORDER){
        painter->setPen(QPen(ClipBorderColour, 1));
    }else{
        painter->setPen(QPen(fill, 0));
    }
    // 绘制文本
    auto textRect = rect.adjusted(4,10,40,-10);
    painter->drawRect(textRect);
    painter->setPen(fill.lightness() > 160 ? QColor(30, 30, 30) : Qt::white);
    QFont font = painter->font();
    font.setPointSize(8);  // 设置较小的字体大小
    painter->setFont(font);

    const QString text = painter->fontMetrics().elidedText(displayText(), Qt::ElideRight, qMax(0, textRect.width() - 4));
    painter->drawText(textRect, Qt::AlignCenter, text);
    // 绘制左侧竖线
    painter->setPen(QPen(ClipBorderColour, 4));  // 稍微粗一点的线
    painter->drawLine(rect.left() + 2, rect.top(),
                     rect.left() + 2, rect.bottom());

    // 设置回细边框
    painter->setPen(QPen(ClipBorderColour, 1));
    painter->restore();  // 恢复状态
}
private:
    QWidget* m_editor;

};
