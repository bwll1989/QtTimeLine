#ifndef BASETIMELINETOOLBAR_H
#define BASETIMELINETOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QApplication>
#include <QSpinBox>
#include <QLabel>
#include "OSCMessage.h"
#include "TimeLineStyle.h"
#include "TimelineExports.hpp"
#include <QMimeData>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
class NODE_TIMELINE_PUBLIC BaseTimelineToolbar : public QToolBar {
    Q_OBJECT
public:
    explicit BaseTimelineToolbar(QWidget* parent = nullptr);
    ~BaseTimelineToolbar() override;
    QAction* m_outputAction;
    //控件OSC地址映射
    std::shared_ptr<std::unordered_map<QString, QAction*>> _OscMapping;

    /**
     * 获取OSC地址和控件的映射
     */
    std::shared_ptr<std::unordered_map<QString, QAction*>> getOscMapping() const
    {
        return _OscMapping;
    }
signals:
    /**
     * 播放点击
     */ 
    void playClicked();
    /**
     * 停止点击
     */
    void stopClicked();
    /**
     * 暂停点击
     */
    void pauseClicked();
    /**
     * 循环点击
     * @param bool enabled 是否启用
     */
    void loopToggled(bool enabled);
    /**
     * 输出窗口点击
     * @param bool show 是否显示
     */
    void outputWindowToggled(bool show);
    /**
     * 设置点击
     */
    void settingsClicked();
    /**
     * 上一帧点击
     */
    void prevFrameClicked();
    /**
     * 下一帧点击
     */
    void nextFrameClicked();
    /**
     * 移动剪辑点击
     */
    void moveClipClicked(int dx);
    /**
     * 删除剪辑点击
     */
    void deleteClipClicked();
    /**
     * 放大点击
     */
    void zoomInClicked();
    /**
     * 缩小点击
     */
    void zoomOutClicked();
public slots:
    /**
     * 设置播放状态
     * @param bool isPlaying 是否播放
     */
    void setPlaybackState(bool isPlaying);
private:
    /**
     * 创建动作
     */
    void createActions();
    /**
     * 设置UI
     */
    void setupUI();
    /**
 * 注册控件OSC地址和Widget指针
 */
     void registerOSCControl(const QString& oscAddress, QAction* control)
    {
        // 如果oscAddress不以"/"开头，则不注册
        if (!oscAddress.startsWith("/")) return;
        // 构建完整的OSC地址，自动给OSC地址添加前缀，包括节点ID
        if (!control) return;
        // 如果已存在相同地址的映射，先移除旧的
        auto it = _OscMapping.get()->find(oscAddress);
        if (it != _OscMapping.get()->end()) {
            _OscMapping.get()->erase(it);
        }

        // 添加新的映射
         QWidget* widget = widgetForAction(control);
         if (widget) {
             widget->installEventFilter(this);
         }
         (*_OscMapping)[oscAddress] = control;
    }
    /**
     * 注销控件OSC地址和Widget指针
     */
     void unregisterOSCControl(const QString& oscAddress)
    {
        if (!oscAddress.startsWith("/")) return;
        auto it = _OscMapping->find(oscAddress);
        if (it != _OscMapping->end()) {
            _OscMapping->erase(it);
        }
    }
    /**
     * 获取控件OSC地址和Widget指针
     */
     QAction* getWidgetFromOSCAddress(const QString& oscAddress) const
    {
        auto it = _OscMapping->find(oscAddress);
        return it !=_OscMapping->end() ? it->second : nullptr;
    }

protected:
     void startDrag(QAction* widget);

    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    // 拖拽起始位置
    QPoint dragStartPosition;
    // 是否正在拖拽
    bool isDragging = false;
    //播放动作
    QAction* m_playAction;
    //停止动作
    QAction* m_stopAction;
    //暂停动作
    QAction* m_pauseAction;
    //循环动作
    QAction* m_loopAction;
    //下一帧动作
    QAction* m_nextFrameAction;
    //上一帧动作
    QAction* m_previousFrameAction;
    //下一媒体动作
    QAction* m_nextMediaAction;
    //上一媒体动作
    QAction* m_previousMediaAction;
    //全屏动作
    QAction* m_fullscreenAction;
    //设置动作
    QAction* m_settingsAction;
    //移动剪辑左动作
    QAction* m_moveClipLeftAction;
    //移动剪辑右动作
    QAction* m_moveClipRightAction;
    //删除剪辑动作
    QAction* m_deleteClipAction;
    //放大动作
    QAction* m_zoomInAction;
    //缩小动作
    QAction* m_zoomOutAction;
    //是否播放
    bool m_isPlaying = false;
};

#endif // TIMELINETOOLBAR_HPP 