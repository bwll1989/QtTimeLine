#ifndef TIMECODE_FRAME_H
#define TIMECODE_FRAME_H

#include <QMetaType>


// Values match Art-Net format
enum class TimeCodeType {
  Film    = 0,       // 24fps
  Film_DF = 1,    // 23.976fps drop frame
  NTSC    = 2,       // 30fps
  NTSC_DF = 3,    // 29.97fps drop frame
  PAL     = 4,   // 25fps
};

// 时间码帧结构体
struct TimeCodeFrame {
  int hours {0};
  int minutes {0};
  int seconds {0};
  int frames {0};
  TimeCodeType type;
};

Q_DECLARE_METATYPE(TimeCodeFrame)

// 重置时间码帧
inline void timecode_frame_reset(TimeCodeFrame &frame)
{
  frame.hours = 0;
  frame.minutes = 0;
  frame.seconds = 0;
  frame.frames = 0;
}

// 获取帧率
inline int timecode_frames_per_sec(TimeCodeType type)
{
  switch(type)
  {
    case TimeCodeType::Film:
      return 24;
    case TimeCodeType::Film_DF:
      return 23.976;
    case TimeCodeType::NTSC:
      return 30;
    case TimeCodeType::NTSC_DF:
      return 29.97;
    case TimeCodeType::PAL:
      return 25;
    default:
      return 25;
  }
}

// 时间码偏移设置
inline TimeCodeFrame timecode_frame_add(TimeCodeFrame frame, int frames)
{
  frame.frames += frames;

  int fps = timecode_frames_per_sec(frame.type);

  if(frame.frames >= fps)
  {
    frame.seconds += static_cast<int>(frame.frames / fps);
    frame.frames = frame.frames % static_cast<int>(fps);
  }

  return frame;
}
// 时间码帧转换为帧数
inline qint64 timecode_frame_to_frames(TimeCodeFrame frame,TimeCodeType type)
{
  int fps = timecode_frames_per_sec(type);
  return frame.hours * 3600 * fps + frame.minutes * 60 * fps + frame.seconds * fps + frame.frames;
}

// 帧数转换为时间码帧
inline TimeCodeFrame frames_to_timecode_frame(int frames, TimeCodeType type)
{
  int fps = timecode_frames_per_sec(type);
  int hours = static_cast<int>(frames / (3600 * fps));
  int remaining_after_hours = static_cast<int>(frames - hours * 3600 * fps);
  int minutes = static_cast<int>(remaining_after_hours / (60 * fps));
  int remaining_after_minutes = static_cast<int>(remaining_after_hours - minutes * 60 * fps);
  int seconds = static_cast<int>(remaining_after_minutes / fps);
  int frame_count = static_cast<int>(remaining_after_minutes - seconds * fps);
  return TimeCodeFrame{hours, minutes, seconds, frame_count, type};
}

// 将时间转为时间码
inline TimeCodeFrame time_to_timecode_frame(double time, TimeCodeType type)
{
    // 获取对应类型的帧率
    int fps = timecode_frames_per_sec(type);
    
    // 计算总帧数(向上取整)
    qint64 total_frames = static_cast<qint64>(std::ceil(time * fps));
    
    // 使用已有的帧数转时间码函数进行转换
    return frames_to_timecode_frame(total_frames, type);
}
// 将时间码转为时间
inline double timecode_frame_to_time(TimeCodeFrame frame, TimeCodeType type)
{
    // 获取对应类型的帧率
    int fps = timecode_frames_per_sec(type);

    // 计算总帧数
    qint64 total_frames = timecode_frame_to_frames(frame, type);
    // 计算总时间
    return static_cast<double>(total_frames) / fps;
}
#endif // TIMECODE_FRAME_H
