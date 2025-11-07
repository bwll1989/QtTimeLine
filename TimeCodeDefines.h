#ifndef TIMECODE_FRAME_H
#define TIMECODE_FRAME_H

#include <QMetaType>
#include <QTime>
/**
 * 时间码类型枚举类
 */
enum class TimeCodeType {
  Film    = 0,       // 24fps
  Film_DF = 1,    // 23.976fps drop frame
  NTSC    = 2,       // 30fps
  NTSC_DF = 3,    // 29.97fps drop frame
  PAL     = 4,   // 25fps
  HD      =5,    //50fps
};

/**
 * 时间码帧结构体
 */
struct TimeCodeFrame {
  int hours {0};
  int minutes {0};
  int seconds {0};
  int frames {0};
  TimeCodeType type;
};

Q_DECLARE_METATYPE(TimeCodeFrame)

/**
 * 将时间码帧重置为0
 * @param frame
 */
inline void timecode_frame_reset(TimeCodeFrame &frame)
{
  frame.hours = 0;
  frame.minutes = 0;
  frame.seconds = 0;
  frame.frames = 0;
}

/**
 * 获取每秒帧数
 * @param type 时间码类型
 * @return 每秒帧数
 */
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
    case TimeCodeType::HD:
      return 50;
    default:
      return 25;
  }
}

/**
 * 时间码加帧
 * @param frame 时间码帧
 * @param frames 帧数
 * @return 加帧后的时间码帧
 */
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

/**
 * 时间码帧转换为帧数
 * @param frame 时间码帧
 * @param type 时间码类型
 * @return 帧数
 */
inline qint64 timecode_frame_to_frames(TimeCodeFrame frame,TimeCodeType type)
{
  int fps = timecode_frames_per_sec(type);
  return frame.hours * 3600 * fps + frame.minutes * 60 * fps + frame.seconds * fps + frame.frames;
}

/**
 * 帧数转换为时间码帧
 * @param frames 帧数
 * @param type 时间码类型
 * @return 时间码帧
 */
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

/**
 * 将时间转为时间码帧
 * @param time 时间（秒）
 * @param type 时间码类型
 * @return 时间码帧
 */
inline TimeCodeFrame time_to_timecode_frame(double time, TimeCodeType type)
{
    // 获取对应类型的帧率
    int fps = timecode_frames_per_sec(type);
    
    // 计算总帧数(向上取整)
    qint64 total_frames = static_cast<qint64>(std::ceil(time * fps));
    
    // 使用已有的帧数转时间码函数进行转换
    return frames_to_timecode_frame(total_frames, type);
}

/**
 * 将时间码帧转为时间（秒）
 * @param frame 时间码帧
 * @param type 时间码类型
 * @return 时间（秒）
 */
inline double timecode_frame_to_time(TimeCodeFrame frame, TimeCodeType type)
{
    // 获取对应类型的帧率
    int fps = timecode_frames_per_sec(type);

    // 计算总帧数
    qint64 total_frames = timecode_frame_to_frames(frame, type);
    // 计算总时间（秒）
    return static_cast<double>(total_frames) / fps;
}

/**
 * 将时间码帧转为字符串，格式：hh:mm:ss:ms
 * @param frame 时间码帧
 * @param type 时间码类型
 * @return 时间码字符串
 */
inline QString timecode_frame_to_string(TimeCodeFrame frame, TimeCodeType type) {
    // 计算毫秒：帧数 * 1000 / 帧率
    int fps = timecode_frames_per_sec(type);
    int ms = static_cast<int>((frame.frames * 1000) / fps);
    return QString("%1:%2:%3:%4")
        .arg(frame.hours,   2, 10, QChar('0'))
        .arg(frame.minutes, 2, 10, QChar('0'))
        .arg(frame.seconds, 2, 10, QChar('0'))
        .arg(ms,            3, 10, QChar('0'));
}

/**
 * 将时间码字符串转为时间码帧
 * @param str 时间码字符串
 * @param type 时间码类型
 * @return 时间码帧
 */
inline TimeCodeFrame string_to_timecode_frame(const QString &str, TimeCodeType type) {
    TimeCodeFrame frame{0, 0, 0, 0, type};
    QStringList parts = str.split(':');
    if (parts.size() != 4) return frame;

    int fps = timecode_frames_per_sec(type);
    frame.hours   = parts[0].toInt();
    frame.minutes = parts[1].toInt();
    frame.seconds = parts[2].toInt();
    int ms        = parts[3].toInt();
    // 毫秒转帧数
    frame.frames  = static_cast<int>((ms * fps) / 1000);
    return frame;
}

/**
 * 将时间码帧转为QTime
 * @param frame 时间码帧
 * @param type 时间码类型
 * @return QTime
 */
inline QTime timecode_frame_to_qtime(TimeCodeFrame frame, TimeCodeType type) {
    int fps = timecode_frames_per_sec(type);
    int ms = static_cast<int>((frame.frames * 1000) / fps);
    return QTime(frame.hours, frame.minutes, frame.seconds, ms);
}

/**
 * 将QTime转为时间码帧
 * @param time QTime
 * @param type 时间码类型
 * @return 时间码帧
 */
inline TimeCodeFrame qtime_to_timecode_frame(QTime time, TimeCodeType type) {
    int fps = timecode_frames_per_sec(type);
    int ms = time.msec();
    int frames = static_cast<int>((ms * fps) / 1000);
    return TimeCodeFrame{time.hour(), time.minute(), time.second(), frames, type};
}
#endif // TIMECODE_FRAME_H
