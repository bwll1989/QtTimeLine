#ifndef TIMELINETYPES_H
#define TIMELINETYPES_H
#include "QString"
#include <QMetaType>
#include <QtGlobal>
#include <QTime>
#include <QtCore/QMetaObject>
#include "Export.hpp"
namespace QtTimeline
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
NODE_TIMELINE_PUBLIC Q_NAMESPACE
#else
Q_NAMESPACE_EXPORT(NODE_TIMELINE_PUBLIC)
#endif

    enum  hoverState {LEFT,RIGHT,CENTER,NONE};
    enum class ItemType {
        Track, // 轨道
        Clip, // 剪辑
        Invalid // 无效
    };
    Q_ENUM_NS(ItemType)

    enum TimelineRoles {
        ClipIdRole, // 剪辑ID
        ClipInRole, // 剪辑开始时间
        ClipOutRole, // 剪辑结束时间
        ClipOscWidgetsRole, //剪辑OSC控件地址
        ClipLengthRole, // 剪辑长度
        ClipPosRole,//剪辑位置
        ClipTypeRole, // 剪辑类型
        ClipShowWidgetRole, // 剪辑是否显示小部件
        ClipResizableRole, // 剪辑是否可调整大小
        ClipShowBorderRole, // 剪辑是否显示边框
        ClipModelRole,  // 用于获取完整的模型对象
        TrackTypeRole, // 轨道类型
        TrackNameRole, // 轨道名称
        TrackModelRole, // 轨道模型
        TrackClipsCountRole, // 轨道剪辑数量
        TrackClipsRole, // 轨道剪辑列表
        TimelineLengthRole, // 时间轴长度
        TimeCodeTypeRole, //时间码格式
    };
    Q_ENUM_NS(TimelineRoles)

    using ClipId = unsigned int;
    // namespace TimecodeFormat {
    //     // 标准电影帧率
    //     constexpr int FILM = 24;           // 24 fps - 标准电影
    //     constexpr double FILM_DF = 23.976; // 24 fps - 标准电影 Drop Frame
    //     // 标准电视帧率 constexpr int FILM
    //     constexpr int NTSC = 30;          // 29.97 fps - NTSC
    //     constexpr double NTSC_DF = 29.97;  // 29.97 fps - NTSC Drop Frame
    //     constexpr int PAL = 25;           // 25 fps - PAL/SECAM
    //     constexpr double PAL_DF = 24.975; // 25 fps - PAL/SECAM Drop Frame
    // }



    // 时间显示格式
    enum class TimedisplayFormat {
        TimeCodeFormat, // 时间码
        AbsoluteTimeFormat, // 绝对时间
    };
    Q_ENUM_NS(TimedisplayFormat)

    enum class ClockSource {
        LTC,
        MTC,
        Internal,
    };
    Q_ENUM_NS(ClockSource)
    // 定义支持的文件类型列表
    inline QStringList VideoTypes = {"mp4", "avi", "mov", "wmv", "mkv", "flv"};
    inline QStringList AudioTypes = {"mp3", "wav", "aac", "ogg", "flac"};
    inline QStringList ImageTypes = {"jpg", "jpeg", "png", "bmp", "gif", "tiff"};
    inline QStringList ControlTypes = {"ilk"};
    // 将track类型转换为字符串
    static QString TrackTypeString(const QString& type) {
        return type;
    }
}

#endif // TIMELINETYPES_H
