//
// Created by benek on 10/16/24.
//

#ifndef CLOCK_H
#define CLOCK_H

#define Clock CrudeTimer::Instance()

class CrudeTimer {
  private:
    CrudeTimer() = default;

  public:
    // copy ctor and assignment should be deleted
    CrudeTimer(const CrudeTimer&)                     = delete;
    CrudeTimer&          operator=(const CrudeTimer&) = delete;
    static CrudeTimer*   Instance();
    [[nodiscard]] double GetTime();
};

#endif // CLOCK_H