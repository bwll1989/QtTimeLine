//
// Created by bwll1 on 2024/5/30.
//
#include "QColor"
#ifndef TIMELINESTYLE_H
#define TIMELINESTYLE_H

// inline QColor zoomFillColour("#4F4F4F"); //缩放填充颜色
// inline QColor zoomBackgroundColour("#262626"); //缩放背景颜色
// inline QColor zoomFrameColour("#262626"); //缩放边框颜色
// 工具栏相关设置
inline QColor toolbarSeparatorColor("#333333"); // 工具栏分隔符颜色

inline QColor toolbarButtonHoverColor("#333333"); // 工具栏按钮悬停颜色

inline QColor toolbarButtonPressedColor("#404040"); // 工具栏按钮按下颜色

inline QColor toolbarButtonCheckedColor("#404040"); // 工具栏按钮选中颜色

inline int toolbarButtonWidth = 30; // 工具栏按钮宽度

inline int toolbarHeight = 30; //工具栏高度

inline int toolbarButtonRadius = 2; // 工具栏按钮圆角

inline int toolbarSpacing = 3; // 工具栏间距

inline int toolbarPadding = 3; // 工具栏内边距


//舞台相关设置
inline int screenBorderNormalWidth = 2; //屏幕边框正常宽度

inline int screenBorderHoverWidth = 2; //屏幕边框悬停宽度

inline int screenBorderSelectedWidth = 4; //屏幕边框选中宽度

inline QColor screenNormalColour = QColor(255,255,255); //屏幕正常颜色

inline QColor screenHoverColour = QColor(255, 165, 0); //屏幕悬停颜色

inline QColor screenSelectedColour = QColor(255, 165, 0); //屏幕选中颜色

inline QColor stageBgColour = QColor(53, 53, 53); //舞台背景颜色

inline int stageAxisWidth = 2; //舞台坐标轴显示宽度

inline QColor stageAxisXColour = QColor("#5f832b"); //舞台轴X颜色

inline QColor stageAxisYColour = QColor("#953c49"); //舞台轴Y颜色



// 时间轴相关设置

inline int trackHeight = 40; //轨道高度

inline int rulerHeight = 30; //刻度高度

inline int clipoffset = 2; //clip偏移

inline int clipround = 0; //clip圆角

inline QColor bgColour = QColor(53, 53, 53);  // 深灰色背景

inline QColor fillColour = QColor("#202020"); //填充颜色

inline QColor seperatorColour = QColor("#414141"); //时间轴竖直分割线

inline QColor rulerColour = QColor("#a7a7a7"); //刻度颜色

inline int textoffset = 13; //文本偏移

inline int baseTimeScale = 10; //基础时间刻度，用于计算缩放

inline int fontSize = 25; //显示时间字体大小

inline QColor fontColor = QColor(206, 91, 87); //字体颜色

inline double m_scale = 0.1; //缩放

inline int timescale = 1; //时间刻度,用于计算缩放

inline int playheadwidth = 5; //播放头宽度

inline int playheadheight = 12; //播放头高度

inline int playheadCornerHeight = playheadheight/4; //播放头圆角高度

// inline double fps=25.00; //帧率

inline QColor playheadColour =QColor(206, 91, 87);  // 播放头颜色即竖线

// Clip colors
inline QColor ClipColor = QColor(126, 188, 105);  //clip颜色

inline QColor ClipHoverColor =  QColor(122, 183, 102); //clip悬停颜色

inline QColor ClipSelectedColor =  QColor(53, 79, 44); //clip选中颜色

inline QColor ClipBorderColour = QColor(235, 154, 5); //clip选中边框颜色

inline int ClipBorderWidth = 2; //clip选中边框宽度

inline QColor trackColour = QColor(53, 53, 53); //轨道颜色

inline QColor trackSelectedColour = trackColour.darker(180); //轨道选中颜色

inline QColor trackHoverColour = trackColour.darker(80); //轨道悬停颜色

const QColor propertyBgColour = bgColour;    // 属性面板使用相同的背景色

#endif //TIMELINESTYLE_HPP

