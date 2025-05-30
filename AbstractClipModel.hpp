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
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDialog>
#include <QPainter>
#include <QRect>
#include <QColor>
#include <QFont>
#include "Export.hpp"

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
virtual~AbstractClipModel() 
{
    // 关闭并删除属性窗口
    if (m_standardPropertyWidget && m_standardPropertyWidget->isVisible()) {
        m_standardPropertyWidget->close();
        m_standardPropertyWidget->deleteLater();
    }
    
}
    
qint64 id() const { return m_id; }

void setId(qint64 id) { m_id = id; }
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
virtual QJsonObject save() const {
    QJsonObject clipJson;
    clipJson["start"] = m_start;
    clipJson["end"] = m_end;
    clipJson["type"] = m_type;
    clipJson["ID"]=m_id;
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
    m_id = json["ID"].toInt();
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
void initPropertyWidget(){
   
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

virtual void paint(QPainter* painter, const QRect& rect, bool selected) const {

    // 绘制背景

    drawBackground(painter, rect, selected);

    // 绘制标题

    drawTitle(painter, rect);
    

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
    // 片段ID
    qint64 m_id;
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