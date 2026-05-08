#ifndef STALED_H
#define STALED_H

#include <Arduino.h>

typedef enum{
  LED_OFF = 0,
  LED_ON,
  LED_BLINK_SLOW,
  LED_BLINK_FAST
}LED_MODE;

typedef struct{
  uint8_t pin;
  LED_MODE mode;
  uint32_t interval;
  uint32_t lastTick;
  uint8_t level;
}LED_CTRL;


void ledInit(uint8_t pin);
void ledSetMode(LED_MODE mode);
void ledUpdate(void);





#endif