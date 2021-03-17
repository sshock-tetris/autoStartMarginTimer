#pragma once

class Timer {
 public:
  Timer();
  ~Timer();
  void Start(int fps);
  int Run();

 protected:
  LARGE_INTEGER m_frequency;
  LARGE_INTEGER m_start_time;
  int m_fps;
  int m_frame_count;
};
