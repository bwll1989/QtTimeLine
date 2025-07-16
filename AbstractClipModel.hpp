#ifndef ABSTRACTCLIPMODEL_H
#define ABSTRACTCLIPMODEL_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include "TimeLineDefines.h"
#include "TimeLineStyle.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QMouseEvent>
#include <QEvent>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>
#include <QMimeData>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QFont>
#include <TimeCodeDefines.h>
#include "TimelineExports.hpp"
#include <qDrag>
#include "OSCMessage.h"
using namespace QtTimeline;
class NODE_TIMELINE_PUBLIC AbstractClipModel : public QObject {
    Q_OBJECT
public:
    explicit AbstractClipModel(int start,const QString& type, QObject* parent=nullptr): QObject(parent),
    m_type(type),
    m_start(start),
    m_end(start+100),
    m_clipPropertyWidget(nullptr),
    m_standardPropertyWidget(nullptr){
        // 是否可调整大小
        RESIZEABLE = true;
        // 是否在片段中嵌入编辑器
        EMBEDWIDGET = false;
        // 是否显示边框
        SHOWBORDER = true;

    }
    /**
     * 析构函数
     */
~AbstractClipModel() override 
{
    // 关闭并删除属性窗口
    if (m_standardPropertyWidget && m_standardPropertyWidget->isVisible()) {
        m_standardPropertyWidget->close();
        m_standardPropertyWidget->deleteLater();
    }
    
}

ClipId id() const { return m_id; }

void setId(ClipId id) { m_id = id; }
    // Getters
    /**
     * 开始
     * @return int 开始
     */
int start() const { return m_start; }
    /**
     * 结束
     * @return int 结束
     */
int end() const { return m_end; }
    /**
     * 类型
     * @return QString 类型
     */
QString type() const { return m_type; }
    /**
     * 长度
     * @return int 长度
     */
int length() const { return m_end - m_start; }
    /**
     * 是否可调整大小
     * @return bool 是否可调整大小
     */
bool isResizable() const { return RESIZEABLE; }
    /**
     * 是否显示小部件
     * @return bool 是否显示小部件
     */
bool isEmbedWidget() const { return EMBEDWIDGET; }
        // 标准属性窗口
QWidget* m_standardPropertyWidget;
    /**
     * 是否显示边框
     * @return bool 是否显示边框
     */
bool isShowBorder() const { return SHOWBORDER; }
    // Setters
    /**
     * 设置开始
     * @param int start 开始
     */
virtual void setStart(int start) { 
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
    /**
     * 设置结束
     * @param int end 结束
     */
virtual void setEnd(int end) { 
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

    /**
     * 设置是否可调整大小
     * @param bool resizable 是否可调整大小
     */
virtual void setResizable(bool resizable) { RESIZEABLE = resizable; }
    /**
     * 设置是否显示小部件
     * @param bool embedWidget 是否显示小部件
     */
void setEmbedWidget(bool embedWidget) { 
    if (EMBEDWIDGET != embedWidget) {
        EMBEDWIDGET = embedWidget;
    }
}
    /** 
     * 设置显示边框
     * @param bool showBorder 显示边框
    */
void setShowBorder(bool showBorder) { SHOWBORDER = showBorder; }
    /**
     * 保存
     * @return QJsonObject 数据
     */

virtual void setTimeCodeType(TimeCodeType timeCodeType){m_timeCodeType=timeCodeType;}

virtual TimeCodeType getTimeCodeType() const {return m_timeCodeType;}

virtual QJsonObject save() const {
    QJsonObject clipJson;
    clipJson["start"] = m_start;
    clipJson["end"] = m_end;
    clipJson["type"] = m_type;
    clipJson["Id"]=static_cast<qint64>(m_id);
    return clipJson;
}
    /**
     * 加载
     * @param const QJsonObject& json 数据
     */
virtual void load(const QJsonObject& json) {
    m_start = json["start"].toInt();
    m_end = json["end"].toInt();
    m_type = json["type"].toString();
    m_id = json["Id"].toInt();
}
    /**
     * 获取数据
     * @param int role 角色
     * @return QVariant 数据
     */
virtual QVariant data(int role) const {
    switch (role) {
        case TimelineRoles::ClipModelRole:
            return QVariant::fromValue<AbstractClipModel*>(const_cast<AbstractClipModel*>(this));
        default:
            return QVariant();
    }
}

    /**
     * 获取当前视频数据
     * @param int currentFrame 当前帧
     * @return QVariantMap 当前视频数据
     */
virtual QVariantMap currentData(int currentFrame) const {

    return QVariantMap();
}
    /**
     * 获取当前控制数据
     * @param int currentFrame 当前帧
     * @return QVariantMap 当前控制数据
     */

virtual QWidget* clipPropertyWidget(){return nullptr;};
    /**
     * 生成标准属性窗口，其中包含自定义的属性
     * @return QWidget* 属性窗口
     */
void showPropertyWidget(){
    if (m_clipPropertyWidget)
    {
        m_standardPropertyWidget->show();
    }else
    {
        initPropertyWidget();
        m_standardPropertyWidget->show();
    }
}
    /**
     * 绘制片段
     * @param QPainter* painter 画笔
     * @param const QRect& rect 绘制区域
     * @param bool selected 是否被选中
     */
virtual void initPropertyWidget(){
   
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
    registerOSCControl("/start",m_startFrameSpinBox);
    connect(m_startFrameSpinBox, &QSpinBox::valueChanged, this, &AbstractClipModel::setStart);
    connect(this, &AbstractClipModel::timelinePositionChanged, this, [this](){
        m_startFrameSpinBox->blockSignals(true);
        m_startFrameSpinBox->setValue(start());
        auto startTimeCode=frames_to_timecode_frame(start(),getTimeCodeType());
        m_startTimeCodeLineEdit->setText(QString("%1:%2:%3.%4").arg(startTimeCode.hours)
                                                                   .arg(startTimeCode.minutes)
                                                                   .arg(startTimeCode.seconds)
                                                                   .arg(startTimeCode.frames));
        m_startFrameSpinBox->blockSignals(false);
    });
    timeLayout->addWidget(m_startFrameSpinBox, 0, 1);
    m_startTimeCodeLineEdit=new QLineEdit(m_standardPropertyWidget);
    m_startTimeCodeLineEdit->setReadOnly(true);  // 设置为只读
    auto startTimeCode=frames_to_timecode_frame(start(),getTimeCodeType());
    m_startTimeCodeLineEdit->setText(QString("%1:%2:%3.%4").arg(startTimeCode.hours)
                                                               .arg(startTimeCode.minutes)
                                                               .arg(startTimeCode.seconds)
                                                               .arg(startTimeCode.frames));
    timeLayout->addWidget(m_startTimeCodeLineEdit, 0, 2);
    
    // 结束帧显示
    auto endLabel = new QLabel(tr("结束帧:"), m_standardPropertyWidget);
    timeLayout->addWidget(endLabel, 1, 0);
    m_endFrameSpinBox=new QSpinBox(m_standardPropertyWidget);
    m_endFrameSpinBox->setRange(0, 9999999);
    m_endFrameSpinBox->setValue(end());
    registerOSCControl("/end",m_endFrameSpinBox);
    connect(m_endFrameSpinBox, &QSpinBox::valueChanged, this, &AbstractClipModel::setEnd);
    connect(this, &AbstractClipModel::lengthChanged, this, [this](){
        m_endFrameSpinBox->blockSignals(true);
        m_endFrameSpinBox->setValue(end());
        auto endTimeCode=frames_to_timecode_frame(end(),getTimeCodeType());
        m_endTimeCodeLineEdit->setText(QString("%1:%2:%3.%4").arg(endTimeCode.hours)
                                                                .arg(endTimeCode.minutes)
                                                                .arg(endTimeCode.seconds)
                                                                .arg(endTimeCode.frames));
        m_endFrameSpinBox->blockSignals(false);
    });
    timeLayout->addWidget(m_endFrameSpinBox, 1, 1);
    
    // 结束时间码显示

    m_endTimeCodeLineEdit=new QLineEdit(m_standardPropertyWidget);
    m_endTimeCodeLineEdit->setReadOnly(true);  // 设置为只读
    auto endtimecode=frames_to_timecode_frame(end(),getTimeCodeType());
    m_endTimeCodeLineEdit->setText(QString("%1:%2:%3.%4").arg(endtimecode.hours).
                                                            arg(endtimecode.minutes).
                                                            arg(endtimecode.seconds).
                                                            arg(endtimecode.frames));
    timeLayout->addWidget(m_endTimeCodeLineEdit, 1, 2);

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

virtual void paint(QPainter* painter, const QRect& rect, bool selected) const {

    // 绘制背景

    drawBackground(painter, rect, selected);

    // 绘制标题

    drawTitle(painter, rect);
    

}
/**
 * 注册控件OSC地址和Widget指针
 */
virtual void registerOSCControl(const QString& oscAddress, QWidget* control)
{
    // 如果oscAddress不以"/"开头，则不注册
    if (!oscAddress.startsWith("/")) return;
    // 构建完整的OSC地址，自动给OSC地址添加前缀，包括节点ID
    if (!control) return;
    // 如果已存在相同地址的映射，先移除旧的
    auto it = _OscMapping.find(oscAddress);
    if (it != _OscMapping.end()) {
        _OscMapping.erase(it);
    }

    // 添加新的映射
    control->installEventFilter(this);
    control->setMouseTracking(true);
    _OscMapping[oscAddress] = control;
}
/**
 * 注销控件OSC地址和Widget指针
 */
virtual void unregisterOSCControl(const QString& oscAddress)
{
    if (!oscAddress.startsWith("/")) return;
    auto it = _OscMapping.find(oscAddress);
    if (it != _OscMapping.end()) {
        _OscMapping.erase(it);
    }
}
/**
 * 获取控件OSC地址和Widget指针
 */
virtual QWidget* getWidgetFromOSCAddress(const QString& oscAddress) const
{
    auto it = _OscMapping.find(oscAddress);
    return it != _OscMapping.end() ? it->second : nullptr;
}
/**
 * 获取OSC地址和控件的映射
 */
virtual std::unordered_map<QString, QWidget*> getOscMapping() const
{
    return _OscMapping;
}
Q_SIGNALS:
    /**
     * 数据变化信号
     */
    void lengthChanged(qint64 length);  // 添加长度变化信号
    void filePathChanged(const QString& filePath);  // 添加文件路径变化信号
    void sizeChanged(QSize size);   //添加尺寸变化信号
    void posChanged(QPoint position); //位置变化信号
    void rotateChanged(int rotete); //旋转变化信号
    void timelinePositionChanged(int frame); //时间轴上位置变化信号，即开始时间
    void videoDataUpdate() const; //视频数据更新
    void audioDataUpdate() const; //音频数据更新
    void controlDataUpdate() const; //控制数据更新

protected:
    // 开始
    int m_start;
    // 结束
    int m_end;
    // 类型
    QString m_type;
    // 轨道索引
    // int m_trackIndex;
    // 是否可调整大小
    bool RESIZEABLE;
    // 是否显示小部件
    bool EMBEDWIDGET;
    // 是否显示边框
    bool SHOWBORDER;
    //控件OSC地址映射
    std::unordered_map<QString, QWidget*> _OscMapping;
    // 片段ID
    ClipId m_id;
    // 拖拽起始位置
    QPoint dragStartPosition;
    // 是否正在拖拽
    bool isDragging = false;
    // 布局
    QVBoxLayout* m_layout;
    //开始帧
    QSpinBox* m_startFrameSpinBox;
    //结束帧
    QSpinBox* m_endFrameSpinBox;
    //开始时间码
    QLineEdit* m_startTimeCodeLineEdit;
    //结束时间码
    QLineEdit* m_endTimeCodeLineEdit;
    // 代理窗口
    QWidget* m_clipPropertyWidget;
    //时间码类型
    TimeCodeType m_timeCodeType;

    void startDrag(QWidget* widget) 
    {
        // 找到对应的OSC地址
        QString oscAddress;
        for (const auto& pair : _OscMapping) {
            if (pair.second == widget) {
                oscAddress = pair.first;
                break;
            }
        }
        
        if (oscAddress.isEmpty()) return;
    
        OSCMessage message;
        message.address = "/timeline/" + QString::number(m_id) + oscAddress;
        message.host = "127.0.0.1";
        message.port = 8991;

        // 获取控件的值
        if (auto* button = qobject_cast<QAbstractButton*>(widget)) {
            message.value = button->isChecked();
            message.type = "Int";
        } else if (auto* slider = qobject_cast<QAbstractSlider*>(widget)) {
            message.value = slider->value();
            message.type = "Int";
        } else if (auto* spinBox = qobject_cast<QSpinBox*>(widget)) {
            message.value = spinBox->value();
            message.type = "Int";
        } else if (auto* lineEdit = qobject_cast<QLineEdit*>(widget)) {
            message.value = lineEdit->text();
            message.type = "String";
        } else if (auto* label = qobject_cast<QLabel*>(widget)) {
            message.value = label->text();
            message.type = "String";
        } else {
            message.value = QVariant();
            message.type = "String";
        }
    
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << message.host << message.port << message.address << message.value<<message.type;
    
        QMimeData* mimeData = new QMimeData;
        mimeData->setData("application/x-osc-address", itemData);
    
        QDrag* drag = new QDrag(widget);
        drag->setMimeData(mimeData);
        QPixmap pixmap(200, 30);
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制背景
        QColor bgColor(40, 40, 40, 200);  // 半透明深灰色
        painter.setBrush(bgColor);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(pixmap.rect(), 5, 5);  // 圆角矩形
        // 绘制文本
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(9);
        painter.setFont(font);
        QRect textRect = pixmap.rect().adjusted(30, 0, -8, 0);  // 图标右侧的文本区域
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, message.address);
    
        // 设置拖拽预览
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));  // 热点在中心
    
        drag->exec(Qt::CopyAction);
    }
    
    bool eventFilter(QObject* watched, QEvent* event) override
{
    // 检查watched是否是_OscMapping中的控件
    auto it = std::find_if(_OscMapping.begin(), _OscMapping.end(),
        [watched](const auto& pair) { return pair.second == watched; });
    
    if (it != _OscMapping.end()) {

        QWidget* widget = it->second;
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::LeftButton && (mouseEvent->modifiers() & Qt::ControlModifier)) {
                    dragStartPosition = mouseEvent->pos();
                    isDragging = true;
                   
                }
                break;
            }
            case QEvent::MouseMove: {
                 
                if (!isDragging) break;
               
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                if ((mouseEvent->pos() - dragStartPosition).manhattanLength() 
                    >= QApplication::startDragDistance()) {
                    
                    startDrag(widget);
                    isDragging = false;
                    return true;
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                isDragging = false;
                break;
            }
            default:
                break;
        }
    }
    return false;
}
    /**

     * 绘制背景

     */

virtual void drawBackground(QPainter* painter, const QRect& rect, bool selected) const {

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

    /**

     * 绘制标题

     */

virtual void drawTitle(QPainter* painter, const QRect& rect) const {

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

};

Q_DECLARE_METATYPE(AbstractClipModel*)

#endif // ABSTRACTCLIPMODEL_HPP 