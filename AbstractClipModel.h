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

class AbstractClipModel : public QObject {
    Q_OBJECT
public:
    explicit AbstractClipModel(int start, const QString& type, QObject* parent = nullptr);
    /**
     * 析构函数
     */
    virtual ~AbstractClipModel();
    
    qint64 id() const;

    void setId(qint64 id);
    // Getters
    /**
     * 开始
     * @return int 开始
     */
    int start() const ;
    /**
     * 结束
     * @return int 结束
     */
    int end() const ;
    /**
     * 类型
     * @return QString 类型
     */
    QString type() const;
    /**
     * 长度
     * @return int 长度
     */
    int length() const ;
    /**
     * 轨道索引
     * @return int 轨道索引
     */
    // int trackIndex() const ;
    /**
     * 是否可调整大小
     * @return bool 是否可调整大小
     */
    bool isResizable() const ;
    /**
     * 是否显示小部件
     * @return bool 是否显示小部件
     */
    bool isEmbedWidget() const ;
        // 标准属性窗口
    QWidget* m_standardPropertyWidget;
    /**
     * 是否显示边框
     * @return bool 是否显示边框
     */
    bool isShowBorder() const ;
    // Setters
    /**
     * 设置开始
     * @param int start 开始
     */
    virtual void setStart(int start) ;
    /**
     * 设置结束
     * @param int end 结束
     */
    virtual void setEnd(int end);
    /**
     * 设置轨道索引
     * @param int index 轨道索引
     */
    // virtual void setTrackIndex(int index);
    /**
     * 设置是否可调整大小
     * @param bool resizable 是否可调整大小
     */
    virtual void setResizable(bool resizable) ;
    /**
     * 设置是否显示小部件
     * @param bool embedWidget 是否显示小部件
     */
    void setEmbedWidget(bool embedWidget) ;
    /** 
     * 设置显示边框
     * @param bool showBorder 显示边框
    */
    void setShowBorder(bool showBorder);
    /**
     * 保存
     * @return QJsonObject 数据
     */
    virtual QJsonObject save() const ;
    /**
     * 加载
     * @param const QJsonObject& json 数据
     */
    virtual void load(const QJsonObject& json);
    /**
     * 获取数据
     * @param int role 角色
     * @return QVariant 数据
     */
    virtual QVariant data(int role) const ;

    /**
     * 获取当前视频数据
     * @param int currentFrame 当前帧
     * @return QVariantMap 当前视频数据
     */
    virtual QVariantMap currentData(int currentFrame) const;
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
    void showPropertyWidget();
    /**
     * 绘制片段
     * @param QPainter* painter 画笔
     * @param const QRect& rect 绘制区域
     * @param bool selected 是否被选中
     */
    void initPropertyWidget();
    virtual void paint(QPainter* painter, const QRect& rect, bool selected) const;
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

    virtual void drawBackground(QPainter* painter, const QRect& rect, bool selected) const ;

    /**

     * 绘制标题

     */

    virtual void drawTitle(QPainter* painter, const QRect& rect) const ;
};

Q_DECLARE_METATYPE(AbstractClipModel*)

#endif // ABSTRACTCLIPMODEL_HPP 