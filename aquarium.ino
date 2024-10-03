#define TANK 2

// RELAY: A0, A1
//
// PWM:   3, 5, 6, 9, 10, 11
// pins:
// 3  - 5  (PWM1)
// 5  - 11 (PWM2)
// 6  - 12 (PWM3)
// 9  - 15 (PWM4)
// 10 - 16 (PWM5)
// 11 - 17 (PWM6)
//
// RTC error LED
// 8  - 14 

#define RELAY_KIND 0
#define PWM_KIND 1

#if (TANK == 1)
// living room
#define EEPROM_FLAG 100
#define PWM_EXT_DIMM 0

#define RELAY1_PIN A0 // Aerator
#define RELAY2_PIN A1 // CO2

#define RELAY_ON HIGH
#define RELAY_OFF LOW

#define PWM1_PIN 5 // Light
#define PWM1_MAX 255

#define CHANNELS_ARR_LENGTH 3
const char *channel_names[] = {
  "Aerator",
  "CO2",
  "Light"
};
#elif (TANK == 2)
// Office
#define EEPROM_FLAG 100
#define PWM_EXT_DIMM 1

#define RELAY1_PIN A0 // Relay 1
#define RELAY2_PIN A1 // Relay 2

#define RELAY_ON HIGH
#define RELAY_OFF LOW

#define PWM1_PIN 3  // Light1
#define PWM1_MAX 255

#define PWM2_PIN 5  // Light2
#define PWM2_MAX 255

#define PWM3_PIN 6  // Light3
#define PWM3_MAX 255

#define CHANNELS_ARR_LENGTH 5
const char *channel_names[] = {
  "Relay 1",
  "Relay 2",
  "Light 1",
  "Light 2",
  "Light 3"
};
#endif

#define PWM_DEPTH 31
const uint8_t pwm_levels[] = {
  0, 1, 2, 3, 4, 5, 7, 9, 12,
  15, 18, 22, 27, 32, 38, 44, 51, 58,
  67, 76, 86, 96, 108, 120, 134, 148, 163,
  180, 197, 216, 235, 255
};

#define PRG_STATE_ON true
#define PRG_STATE_OFF false
// ----------
#define CHANNEL_MODE_OFF 0
#define CHANNEL_MODE_AUTO 1
#define CHANNEL_MODE_MANUAL 2
// ----------
#define PROGRAM_PER_CHANNEL 5
#define PROGRAMS_ARR_LENGTH (CHANNELS_ARR_LENGTH * PROGRAM_PER_CHANNEL)
// ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_RESET -1
#define TEXT_CHAR_WIDTH 6
#define TEXT_CHAR_HEIGHT 16
// ----------
#define DST 1
#define STD 0
#define DST_EEPROM_ADDR 500
#define RTC_LOST_POWER_ADDR 501
#define RTC_LOST_POWER_LED_PIN 8
// ----------
#define MAIN 0
#define MAIN_MENU 1
#define SET_DATETIME 2
#define RESET 3
#define CHANNELS 4
#define CHANNEL_SETTINGS 5
#define CHANNEL_MODE_SETUP 6
#define PROGRAMS 7
#define PROGRAM_SETTINGS 8
#define PROGRAM_SETTINGS_START_TIME 9
#define PROGRAM_SETTINGS_DURATION 10
#define PROGRAM_SETTINGS_VALUE 11
// ----------
#define DISPLAY_TIMEOUT_MILLIS 180000 // 3min
// ----------
#define incVal(v, diff, max) min((v) + (diff), (max))
#define decVal(v, diff, min) max((v) - (diff), (min))

#define displayPrintInvColor(x) ( \
  { \
    display.setTextColor(BLACK, WHITE); \
    display.print((x)); \
    display.setTextColor(WHITE); \
  })

#define displayPrintlnInvColor(x) ( \
  { \
    display.setTextColor(BLACK, WHITE); \
    display.println((x)); \
    display.setTextColor(WHITE); \
  })
// ----------

// ==========
#include <avr/wdt.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <RTClib.h>

const uint8_t channel_pwm_max[] = {
#ifdef RELAY1_PIN
  0,  // unused
#endif
#ifdef RELAY2_PIN
  0,  // unused
#endif
#ifdef PWM1_PIN
  PWM1_MAX,
#endif
#ifdef PWM2_PIN
  PWM2_MAX,
#endif
#ifdef PWM3_PIN
  PWM3_MAX,
#endif
#ifdef PWM4_PIN
  PWM4_MAX,
#endif
#ifdef PWM5_PIN
  PWM5_MAX,
#endif
#ifdef PWM6_PIN
  PWM6_MAX,
#endif
};

#define EEPROM_CHANNEL_CELL_SIZE 10
#define channelAddr(chNum) (chNum * EEPROM_CHANNEL_CELL_SIZE)
struct channel {
  uint8_t kind;
  uint8_t pin;
  uint8_t mode;
  uint8_t offVal;
  uint8_t maxVal;
  uint8_t val;
  uint32_t timer;
};

#define EEPROM_PRG_CELL_SIZE 6
#define programAddr(chNum, prgNum) (CHANNELS_ARR_LENGTH * EEPROM_CHANNEL_CELL_SIZE + chNum * PROGRAM_PER_CHANNEL * EEPROM_PRG_CELL_SIZE + prgNum * EEPROM_PRG_CELL_SIZE)
struct program {
  bool enabled;
  uint8_t startAtHour;
  uint8_t startAtMin;
  uint16_t workMin;
  uint8_t val;
};

struct window {
  uint8_t start;
  uint8_t end;
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RESET);
RTC_DS3231 rtc;

uint8_t pwm_pins[CHANNELS_ARR_LENGTH];
volatile uint8_t pwm_duties[CHANNELS_ARR_LENGTH];

uint8_t enc_state = 0;
uint8_t mode = MAIN;

bool update_display = false;

void setup() {
  Serial.begin(9600);

  initDisplay();
  display.setTextSize(1, 2);

  if (!isEEPROMReady()) {
    initEEPROM();
    // DateTime now = rtc.now();
    // EEPROM.write(DST_EEPROM_ADDR, isDST(now) ? DST : STD);
  }

  initClock();
  initEncoder();

  pinMode(RTC_LOST_POWER_LED_PIN, OUTPUT);
  for (uint8_t c = 0; c < CHANNELS_ARR_LENGTH; c++) {
    channel ch;
    EEPROM.get(channelAddr(c), ch);
    pinMode(ch.pin, OUTPUT);
    digitalWrite(ch.pin, ch.val);
    if (ch.kind == PWM_KIND) pwm_pins[c] = ch.pin;
  }

  wdt_enable(WDTO_4S);
}

void loop() {
  static uint32_t check_timer = 0;
  static uint32_t display_timer = 0;

  digitalWrite(RTC_LOST_POWER_LED_PIN, lostPower() ? HIGH : LOW); 

  enc_state = tickEncoder();
  if (mode == MAIN) {
    drawMainScreen();
  } else if (mode == MAIN_MENU) {
    drawMainMenuScreen();
  } else if (mode == SET_DATETIME) {
    drawSetDatetimeScreen();
  } else if (mode == CHANNELS) {
    drawChannelsScreen();
  } else if (mode == RESET) {
    drawResetScreen();
  } else if (mode == CHANNEL_SETTINGS) {
    drawChannelSettingsScreen();
  } else if (mode == CHANNEL_MODE_SETUP) {
    drawChannelModeSetupScreen();
  } else if (mode == PROGRAMS) {
    drawProgramsScreen();
  } else if (mode == PROGRAM_SETTINGS) {
    drawProgramSettingsScreen();
  } else if (mode == PROGRAM_SETTINGS_START_TIME) {
    drawProgramSettingsStartTimeScreen();
  } else if (mode == PROGRAM_SETTINGS_DURATION) {
    drawProgramSettingsDurationScreen();
  } else if (mode == PROGRAM_SETTINGS_VALUE) {
    drawProgramSettingsValueScreen();
  }

  if (millis() - check_timer > 500) {
    DateTime now = rtc.now();
    if (now.minute() == 0 && now.second() == 0) adjustRtcIfNeeded(now);

    for (uint8_t c = 0; c < CHANNELS_ARR_LENGTH; c++) {
      channel ch;
      EEPROM.get(channelAddr(c), ch);

      if (ch.kind == RELAY_KIND) digitalWrite(ch.pin, ch.val);
      else if (ch.kind == PWM_KIND && mode != PROGRAM_SETTINGS_VALUE) pwm_duties[c] = ch.val;

      if (ch.kind == PWM_KIND) writePWM(ch.pin, min(pwm_levels[pwm_duties[c]], channel_pwm_max[c]));

      if (ch.timer == 0 || now.second() == 0) {
        if (ch.mode == CHANNEL_MODE_AUTO) {
          uint32_t nowSeconds = toSeconds(now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
          if (ch.timer <= nowSeconds) {
            program prg;
            bool activePrg = 0;
            uint32_t startSeconds;
            uint32_t endSeconds;
            for (uint8_t p = 0; !activePrg && (p < PROGRAM_PER_CHANNEL); p++) {
              EEPROM.get(programAddr(c, p), prg);
              if (prg.enabled) {                
                startSeconds = toSeconds(now.year(), now.month(), now.day(), prg.startAtHour, prg.startAtMin, 0);
                endSeconds = startSeconds + prg.workMin * 60;
                if (nowSeconds >= startSeconds && nowSeconds < endSeconds) activePrg = 1;
              }
            }
            if (activePrg) {
              EEPROM.update(channelAddr(c) + 5, prg.val);
              EEPROM.put(channelAddr(c) + 6, endSeconds);
            } else if (ch.val != ch.offVal) {
              EEPROM.update(channelAddr(c) + 5, ch.offVal);
            }
          }
        }
      }
    }
    check_timer = millis();
  }
  if (millis() - display_timer > DISPLAY_TIMEOUT_MILLIS) {
    turnDisplayOff();
  }
  if (!isEncNothing(enc_state)) {
    turnDisplayOn();
    display_timer = millis();
  }
  wdt_reset();
}

void drawMainScreen() {
  static uint32_t update_display_timer = 0;
  if (isEncHold(enc_state)) {
    mode = MAIN_MENU;
    update_display = true;
    return;
  }
  if (millis() - update_display_timer > 50) {
    display.clearDisplay();
    display.setCursor(0, 0);

    printCurrentTime();
    display.setCursor(0, TEXT_CHAR_HEIGHT);
    display.write(mode);
    display.display();

    update_display_timer = millis();
  }
}

void printLines(char *lines[], uint8_t pos, uint8_t len) {
  window win = calculateWindow(pos, len);
  for (uint8_t i = win.start; i < win.end; i++) {
    if (i == pos) display.write('>');
    else display.write(' ');
    display.println(lines[i]);
  }
}

void printTable(char *names[], char *values[], uint8_t pos, uint8_t len) {
  window win = calculateWindow(pos, len);
  for (uint8_t i = win.start; i < win.end; i++) {
    if (i == pos) display.write('>');
    else display.write(' ');
    display.print(names[i]);
    display.println(values[i]);
  }
}

window calculateWindow(uint8_t pos, uint8_t len) {
  uint8_t start = (pos < SCREEN_HEIGHT / TEXT_CHAR_HEIGHT) ? 0 : (pos - SCREEN_HEIGHT / TEXT_CHAR_HEIGHT + 1);
  uint8_t end = start + SCREEN_HEIGHT / TEXT_CHAR_HEIGHT;
  if (end > len) end = len;
  return { start, end };
}

void format2Digits(uint8_t d, char s[]) {
  if (d < 10) {
    s[0] = '0';
    s[1] = '0' + d;
  } else {
    s[0] = '0' + d / 10;
    s[1] = '0' + d % 10;
  }
  s[2] = '\0';
}

void writePWM(uint8_t pin, int val) {
#if (PWM_EXT_DIMM == 1)
  analogWrite(pin, ~val);
#else
  analogWrite(pin, val);
#endif
}
