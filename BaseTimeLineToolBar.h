#ifndef BASETIMELINETOOLBAR_H
#define BASETIMELINETOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QSpinBox>
#include <QLabel>
#include "TimeLineStyle.h"
class BaseTimelineToolbar : public QToolBar {
    Q_OBJECT
public:
    explicit BaseTimelineToolbar(QWidget* parent = nullptr);
    ~BaseTimelineToolbar() override;
    QAction* m_outputAction;

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

private:
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