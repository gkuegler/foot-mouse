#include <Arduino.h>

class Timer
{
private:
  unsigned long duration = 0;
  unsigned long start_time = 0;
  bool auto_reset = true;
  bool enabled = false;

public:
  Timer() {};
  ~Timer() {};

  // Returns true if timer is triggered.
  bool update();

  void start(unsigned long ms);
  void disable();
  void enable();
  void reset();
  void restart();
};

void
Timer::start(unsigned long ms)
{
  duration = ms;
  start_time = millis();
  enabled = true;
}

bool
Timer::update()
{
  if (!enabled) {
    return false;
  }
  auto now = millis();
  bool result = now > (start_time + duration);
  if (result && auto_reset) {
    start_time = now;
  }
  return result;
}

void
Timer::restart()
{
  start_time = millis();
  enabled = true;
}

void
Timer::reset()
{
  start_time = millis();
}

void
Timer::enable()
{
  enabled = true;
}

void
Timer::disable()
{
  enabled = false;
}