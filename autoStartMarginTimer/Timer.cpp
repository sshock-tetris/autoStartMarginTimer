#include "stdafx.h"

#include "Timer.h"

Timer::Timer() : m_start_time({}), m_fps(60), m_frame_count(0) {
  QueryPerformanceFrequency(&m_frequency);
}

Timer::~Timer() {}

void Timer::Start(int fps) {
  m_fps = fps;
  m_frame_count = 0;

  QueryPerformanceCounter(&m_start_time);
}

int Timer::Run(void) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);

  int count = static_cast<int>((now.QuadPart - m_start_time.QuadPart) /
                               (m_frequency.QuadPart / m_fps));
  if (count != m_frame_count) {
    int ret = count - m_frame_count;
    m_frame_count = count;
    return ret;
  }

  return 0;
}
