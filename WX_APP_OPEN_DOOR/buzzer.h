#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>


#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523



void buzzerInit(void);
void buzzerTask(void);

void buzzerPlaySuccess(void);
void buzzerPlayFail(void);
void buzzerPlayWarning(void);
void buzzerPlayStartup(void);
void buzzerStop(void);

void musicStart(void);
void musicTask(void);

#endif
