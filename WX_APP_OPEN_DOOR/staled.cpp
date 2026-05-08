#include "staled.h"



LED_CTRL staled;

void ledInit(uint8_t pin)
{
  staled.pin = pin;
  staled.mode = LED_OFF;
  staled.interval = 0;
  staled.lastTick = 0;
  staled.level = HIGH;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}


void ledSetMode(LED_MODE mode)
{
  staled.mode = mode;

  switch (mode) {
    case LED_OFF:
      digitalWrite(staled.pin, HIGH);
      break;

    case LED_ON:
      digitalWrite(staled.pin, LOW);
      break;

    case LED_BLINK_SLOW:
      staled.interval = 1000;   // 1 秒翻转
      break;

    case LED_BLINK_FAST:
      staled.interval = 200;    // 200 ms 翻转
      break;
  }
}


void ledUpdate(void)
{
  uint32_t now = millis();

  if (staled.mode == LED_BLINK_SLOW ||
      staled.mode == LED_BLINK_FAST)
  {
    if (now - staled.lastTick >= staled.interval)
    {
      staled.lastTick = now;
      staled.level = !staled.level;
      digitalWrite(staled.pin, staled.level);
    }
  }
}

