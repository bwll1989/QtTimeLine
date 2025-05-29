#include "AbstractClipModel.h"

AbstractClipModel::AbstractClipModel(int start,const QString& type, QObject* parent)
: QObject(parent),
m_type(type),
m_start(start),
m_end(start+100),
m_clipPropertyWidget(nullptr),
m_standardPropertyWidget(nullptr)
{
    // 是否可调整大小
    RESIZEABLE = true;
    // 是否在片段中嵌入编辑器
    EMBEDWIDGET = false;
    // 是否显示边框
    SHOWBORDER = true;
    
}

// 实现虚析构函数
AbstractClipModel::~AbstractClipModel() 
{
    // 关闭并删除属性窗口
    if (m_standardPropertyWidget && m_standardPropertyWidget->isVisible()) {
        m_standardPropertyWidget->close();
        m_standardPropertyWidget->deleteLater();
    }
    
}
qint64 AbstractClipModel::id() const { return m_id; }
void AbstractClipModel::setId(qint64 id) { m_id = id; }
// Getters
int AbstractClipModel::start() const { return m_start; }
int AbstractClipModel::end() const { return m_end; }
QString AbstractClipModel::type() const { return m_type; }
int AbstractClipModel::length() const { return m_end - m_start; }
// int AbstractClipModel::trackIndex() const { return m_trackIndex; }
bool AbstractClipModel::isResizable() const { return RESIZEABLE; }
bool AbstractClipModel::isEmbedWidget() const { return EMBEDWIDGET; }
bool AbstractClipModel::isShowBorder() const { return SHOWBORDER; }

QVariant AbstractClipModel::data(int role) const {
    switch (role) {
        case TimelineRoles::ClipModelRole:
            return QVariant::fromValue<AbstractClipModel*>(const_cast<AbstractClipModel*>(this));
        default:
            return QVariant();
    }
}
// Setters
void AbstractClipModel::setStart(int start) { 
    if (m_start != start) {
        if(RESIZEABLE){
           
            m_start = start; 

            emit timelinePositionChanged(m_start);
        }else{
            auto length = this->length();
            m_start = start;
            m_end = m_start + length;
            emit timelinePositionChanged(m_start);
            emit lengthChanged(m_end);
        }

    }
}
void AbstractClipModel::setEnd(int end) { 
    if (m_end != end) {
        if(RESIZEABLE){
            m_end = end;

            emit lengthChanged(m_end);
        }else{
            auto length = this->length();
            m_end = end;
            m_start = m_end - length;
            emit lengthChanged(m_end);
            emit timelinePositionChanged(m_start);
        }

    }
}
// void AbstractClipModel::setTrackIndex(int index) { m_trackIndex = index; }
void AbstractClipModel::setResizable(bool resizable) { RESIZEABLE = resizable; }
void AbstractClipModel::setEmbedWidget(bool embedWidget) { 
    if (EMBEDWIDGET != embedWidget) {
        EMBEDWIDGET = embedWidget;
    }
}
void AbstractClipModel::setShowBorder(bool showBorder) { SHOWBORDER = showBorder; }
// Save/Load
QJsonObject AbstractClipModel::save() const {
    QJsonObject clipJson;
    clipJson["start"] = m_start;
    clipJson["end"] = m_end;
    clipJson["type"] = m_type;
    clipJson["ID"]=m_id;
    return clipJson;
}

void AbstractClipModel::load(const QJsonObject& json) {
    m_start = json["start"].toInt();
    m_end = json["end"].toInt();
    m_type = json["type"].toString();
    m_id = json["ID"].toInt();
}

QVariantMap AbstractClipModel::currentData(int currentFrame) const {

    return QVariantMap();
}

void AbstractClipModel::paint(QPainter* painter, const QRect& rect, bool selected) const {

        // 绘制背景

        drawBackground(painter, rect, selected);

        // 绘制标题

        drawTitle(painter, rect);
       

    }

void AbstractClipModel::drawBackground(QPainter* painter, const QRect& rect, bool selected) const {

    painter->save();
    // 设置背景颜色
    QColor bgColor = selected ?  ClipColor.darker(150):ClipColor;

    painter->setBrush(bgColor);
    if(SHOWBORDER){
        painter->setPen(QPen(ClipBorderColour, ClipBorderWidth));
    }else{
        painter->setPen(QPen(bgColor, 0));
    }
    // 绘制圆角矩形
    painter->drawRoundedRect(rect, clipround, clipround);

    painter->restore();

}

void AbstractClipModel::drawTitle(QPainter* painter, const QRect& rect) const {

    painter->save();
    // 设置文字颜色和字体
    painter->setPen(Qt::white);

    QFont font = painter->font();

    font.setPointSize(8);

    painter->setFont(font);
    // 绘制类型和标题
    QString text = type();
    painter->drawText(rect, Qt::AlignCenter, text);
    painter->restore();

}

void AbstractClipModel::initPropertyWidget(){
   
    m_standardPropertyWidget = new QWidget();
    m_layout = new QVBoxLayout(m_standardPropertyWidget);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(4);

    // 创建一个容器 widget 来放置主要内容
    auto contentWidget = new QWidget(m_standardPropertyWidget);
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);

    // 1. 时间属性组
    QGroupBox *timeGroupBox = new QGroupBox(tr("时间属性"), m_standardPropertyWidget);
    QGridLayout *timeLayout = new QGridLayout(timeGroupBox);
    
    // 开始帧显示
    auto startLabel = new QLabel(tr("开始帧:"), m_standardPropertyWidget);
    timeLayout->addWidget(startLabel, 0, 0);
    m_startFrameSpinBox=new QSpinBox(m_standardPropertyWidget);
    m_startFrameSpinBox->setRange(0, 9999999);
    m_startFrameSpinBox->setValue(start());
    connect(m_startFrameSpinBox, &QSpinBox::valueChanged, this, &AbstractClipModel::setStart);
    connect(this, &AbstractClipModel::timelinePositionChanged, this, [this](){
        m_startFrameSpinBox->blockSignals(true);
        m_startFrameSpinBox->setValue(start());
       m_startTimeCodeLineEdit->setText(QString("%1")
                                   .arg(start()));
        m_startFrameSpinBox->blockSignals(false);
    });
    timeLayout->addWidget(m_startFrameSpinBox, 0, 1);
    
    // 开始时间码显示
    auto startTimeCodeLabel = new QLabel(tr("开始时间:"), m_standardPropertyWidget);
    timeLayout->addWidget(startTimeCodeLabel, 1, 0);
    // m_startTimeCodeLineEdit = new QLineEdit(this);  // 确保在头文件中声明
    m_startTimeCodeLineEdit=new QLineEdit(m_standardPropertyWidget);
    m_startTimeCodeLineEdit->setReadOnly(true);  // 设置为只读
   m_startTimeCodeLineEdit->setText(QString("%1")
                                   .arg(end()));
    timeLayout->addWidget(m_startTimeCodeLineEdit, 1, 1);
    
    // 结束帧显示
    auto endLabel = new QLabel(tr("结束帧:"), m_standardPropertyWidget);
    timeLayout->addWidget(endLabel, 2, 0);
    m_endFrameSpinBox=new QSpinBox(m_standardPropertyWidget);
    m_endFrameSpinBox->setRange(0, 9999999);
    m_endFrameSpinBox->setValue(end());
    connect(m_endFrameSpinBox, &QSpinBox::valueChanged, this, &AbstractClipModel::setEnd);
    connect(this, &AbstractClipModel::lengthChanged, this, [this](){
        m_endFrameSpinBox->blockSignals(true);
        m_endFrameSpinBox->setValue(end());
       m_endTimeCodeLineEdit->setText(QString("%1")
                                   .arg(end()));
        m_endFrameSpinBox->blockSignals(false);
    });
    timeLayout->addWidget(m_endFrameSpinBox, 2, 1);
    
    // 结束时间码显示
    auto endTimeCodeLabel = new QLabel(tr("结束时间:"), m_standardPropertyWidget);
    timeLayout->addWidget(endTimeCodeLabel, 3, 0);
    // m_endTimeCodeLineEdit = new QLineEdit(this);  // 确保在头文件中声明
    m_endTimeCodeLineEdit=new QLineEdit(m_standardPropertyWidget);
    m_endTimeCodeLineEdit->setReadOnly(true);  // 设置为只读
   m_endTimeCodeLineEdit->setText(QString("%1")
                                   .arg(end()));
    timeLayout->addWidget(m_endTimeCodeLineEdit, 3, 1);

    m_layout->addWidget(timeGroupBox);
    
    // 添加代理编辑器的占位符
    if (!m_clipPropertyWidget) {
        m_clipPropertyWidget = clipPropertyWidget();
        contentLayout->addWidget(m_clipPropertyWidget);
    }

    // 添加弹簧以确保内容在顶部
    contentLayout->addStretch();

    // 将内容 widget 添加到主布局
    m_layout->addWidget(contentWidget);
    m_standardPropertyWidget->setWindowFlags(Qt::WindowStaysOnTopHint);
    
    
}

void AbstractClipModel::showPropertyWidget(){
    if (m_clipPropertyWidget)
    {
        m_standardPropertyWidget->show();
    }else
    {
        initPropertyWidget();
        m_standardPropertyWidget->show();
    }
    
   
}
// 如果有任何其他非内联成员函数，也可以在这里实现 