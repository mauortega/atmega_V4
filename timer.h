#ifndef TIMER_H
#define TIMER_H

struct Timer {
  unsigned long start;
  unsigned long timeout;
  void (*pfunc)(void);

  void Start(void) {
    start = millis();
  }

  void Verify(void) {
    unsigned long now = millis();

    if (now < start)
      Start();

    if (now > (start + timeout)) {
      Start();
      (*pfunc)();
    }
  }
};

#endif
