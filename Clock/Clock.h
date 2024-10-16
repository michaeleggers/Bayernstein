//
// Created by benek on 10/16/24.
//

#ifndef CLOCK_H
#define CLOCK_H

#include "SDL.h"

class Clock {
private:
  Clock() = default;

  Uint64 m_TicksPerSecond;
  Uint64 m_TicksPerFrame;
  Uint64 m_StartCounter;
  Uint64 m_EndCounter;
  // copy ctor and assignment should be private
  Clock(const Clock &);
  Clock &operator=(const Clock &);

public:
  static Clock *Instance();
  void Init();

  [[nodiscard]] double GetTime();
  [[nodiscard]] double GetMillisecondsPerFrame();
  void StartCounter();
  void EndCounter();
};

#endif // CLOCK_H
