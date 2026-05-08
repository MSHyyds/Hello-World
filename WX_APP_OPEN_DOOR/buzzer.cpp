#include "buzzer.h"

#define BUZZER_PIN 13

typedef struct {
  uint16_t freq;   // 频率，0 表示静音
  uint16_t time;   // 持续时间 ms
} BuzzerStep;

/* ========= 各种提示音定义 ========= */

// 成功：滴-滴
static const BuzzerStep sound_success[] = {
  {2000, 100},
  {0,    50},
  {2500, 100},
};

// 失败：低-低
static const BuzzerStep sound_fail[] = {
  {800,  200},
  {0,    80},
  {800,  200},
};

// 报警：长鸣
static const BuzzerStep sound_warning[] = {
  {1500, 800},
};

// 上电：升调
static const BuzzerStep sound_startup[] = {
  {1200, 80},
  {1800, 80},
  {2400, 80},
};

/* ========= 状态机变量 ========= */

static const BuzzerStep* currentSound = nullptr;
static uint8_t stepIndex = 0;
static uint32_t stepStartTime = 0;
static uint8_t stepCount = 0;
static bool buzzerActive = false;


static const int melody[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_C4};
static const int duration[] = {300, 300, 300, 300};

static int musicIndex = 0;
static unsigned long lastTime = 0;
static bool playing = false;

/* ========= 内部函数 ========= */

static void buzzerStart(const BuzzerStep* sound, uint8_t count) {
  currentSound = sound;
  stepCount = count;
  stepIndex = 0;
  stepStartTime = millis();
  buzzerActive = true;

  if (sound[0].freq > 0)
    tone(BUZZER_PIN, sound[0].freq);
  else
    noTone(BUZZER_PIN);
}

/* ========= 对外接口 ========= */

void buzzerInit(void) {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void buzzerTask(void) {
  if (!buzzerActive) return;

  uint32_t now = millis();
  if (now - stepStartTime >= currentSound[stepIndex].time) {
    stepIndex++;
    stepStartTime = now;

    if (stepIndex >= stepCount) {
      noTone(BUZZER_PIN);
      buzzerActive = false;
      return;
    }

    if (currentSound[stepIndex].freq > 0)
      tone(BUZZER_PIN, currentSound[stepIndex].freq);
    else
      noTone(BUZZER_PIN);
  }
}

void buzzerStop(void) {
  noTone(BUZZER_PIN);
  buzzerActive = false;
}

/* ========= 播放接口 ========= */

void buzzerPlaySuccess(void) {
  buzzerStart(sound_success, sizeof(sound_success) / sizeof(BuzzerStep));
}

void buzzerPlayFail(void) {
  buzzerStart(sound_fail, sizeof(sound_fail) / sizeof(BuzzerStep));
}

void buzzerPlayWarning(void) {
  buzzerStart(sound_warning, sizeof(sound_warning) / sizeof(BuzzerStep));
}

void buzzerPlayStartup(void) {
  buzzerStart(sound_startup, sizeof(sound_startup) / sizeof(BuzzerStep));
}


void musicStart(void)
{
  musicIndex = 0;
  playing = true;
  lastTime = millis();
  tone(BUZZER_PIN, melody[musicIndex]);
}

void musicTask(void)
{
  if (!playing) return;

  if (millis() - lastTime >= duration[musicIndex]) {
    noTone(BUZZER_PIN);
    musicIndex++;

    if (musicIndex >= (int)(sizeof(melody) / sizeof(melody[0]))) {
      playing = false;
      return;
    }

    tone(BUZZER_PIN, melody[musicIndex]);
    lastTime = millis();
  }
}

