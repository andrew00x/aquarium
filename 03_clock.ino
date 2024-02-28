#define MARCH 3
#define OCTOBER 10

#define SET_DATETIME_YEAR 1
#define SET_DATETIME_MONTH 2
#define SET_DATETIME_DAY 3
#define SET_DATETIME_HOUR 4
#define SET_DATETIME_MIN 5
#define SET_DATETIME_SEC 6
#define SET_DATETIME_CANCEL 7
#define SET_DATETIME_OK 8

uint8_t setup_datetime_step = 0;

void initClock() {
  if (!rtc.begin()) {
    display.println(F("ERR: RTC not found"));
    display.display();
    while (1) delay(100);
  } else if (rtc.lostPower()) {
    display.println(F("RTC lost power, set the time."));
    display.display();
    delay(2000);
    DateTime datetime = DateTime(F(__DATE__), F(__TIME__));
    rtc.adjust(datetime);
    if (isDST(datetime)) EEPROM.update(DST_EEPROM_ADDR, DST);
    else EEPROM.update(DST_EEPROM_ADDR, STD);
  }
}

void adjustRtcIfNeeded(DateTime& datetime) {
  if (isDST(datetime)) {
    if (EEPROM.read(DST_EEPROM_ADDR) != DST) {
      rtc.adjust(DateTime(datetime.year(), datetime.month(), datetime.day(), datetime.hour() + 1, datetime.minute(), datetime.second()));
      EEPROM.write(DST_EEPROM_ADDR, DST);
    }
  } else if (EEPROM.read(DST_EEPROM_ADDR) != STD) {
    rtc.adjust(DateTime(datetime.year(), datetime.month(), datetime.day(), datetime.hour() - 1, datetime.minute(), datetime.second()));
    EEPROM.write(DST_EEPROM_ADDR, STD);
  }
}

bool isDST(DateTime& datetime) {
  uint8_t month = datetime.month();
  if (month > MARCH && month < OCTOBER) return true;
  if (month == MARCH || month == OCTOBER) {
    uint8_t day = datetime.day();
    uint8_t firstSunday = (day - datetime.dayOfTheWeek()) % 7;
    uint8_t lastSunday = firstSunday + ((31 - firstSunday) / 7) * 7;
    if (month == MARCH) {
      return day > lastSunday || (day == lastSunday && datetime.hour() >= 3);
    }
    return day < lastSunday || (day == lastSunday && datetime.hour() < 3);
  }
  return false;
}

void printCurrentTime() {
  DateTime now = rtc.now();
  display.setCursor((SCREEN_WIDTH - 8 * TEXT_CHAR_WIDTH) / 2, 0);

  char s[3];

  format2Digits(now.hour(), s);
  display.print(s);
  display.print(':');

  format2Digits(now.minute(), s);
  display.print(s);
  display.print(':');

  format2Digits(now.second(), s);
  display.print(s);
}

void drawSetDatetimeScreen() {
  if (setTime()) mode = MAIN;
}

bool setTime() {
  static uint16_t year;
  static uint8_t month;
  static uint8_t day;
  static uint8_t hour;
  static uint8_t min;
  static uint8_t sec;
  if (setup_datetime_step == 0) {
    DateTime now = rtc.now();
    year = now.year();
    month = now.month();
    day = now.day();
    hour = now.hour();
    min = now.minute();
    sec = now.second();
    setup_datetime_step = SET_DATETIME_YEAR;
  } else {
    if (isEncTurnCW(enc_state)) {
      if (setup_datetime_step == SET_DATETIME_YEAR) year = incVal(year, 1, 2099);
      else if (setup_datetime_step == SET_DATETIME_MONTH) month = incVal(month, 1, 12);
      else if (setup_datetime_step == SET_DATETIME_DAY) day = incVal(day, 1, 31);
      else if (setup_datetime_step == SET_DATETIME_HOUR) hour = incVal(hour, 1, 23);
      else if (setup_datetime_step == SET_DATETIME_MIN) min = incVal(min, 1, 59);
      else if (setup_datetime_step == SET_DATETIME_SEC) sec = incVal(sec, 1, 59);
      else if (setup_datetime_step == SET_DATETIME_CANCEL) setup_datetime_step = SET_DATETIME_OK;
      else if (setup_datetime_step == SET_DATETIME_OK) setup_datetime_step = SET_DATETIME_CANCEL;
      update_display = true;
    } else if (isEncTurnCCW(enc_state)) {
      if (setup_datetime_step == SET_DATETIME_YEAR) year = decVal(year, 1, 2000);
      else if (setup_datetime_step == SET_DATETIME_MONTH) month = decVal(month, 1, 1);
      else if (setup_datetime_step == SET_DATETIME_DAY) day = decVal(day, 1, 1);
      else if (setup_datetime_step == SET_DATETIME_HOUR) hour = decVal(hour, 1, 0);
      else if (setup_datetime_step == SET_DATETIME_MIN) min = decVal(min, 1, 0);
      else if (setup_datetime_step == SET_DATETIME_SEC) sec = decVal(sec, 1, 0);
      else if (setup_datetime_step == SET_DATETIME_OK) setup_datetime_step = SET_DATETIME_CANCEL;
      else if (setup_datetime_step == SET_DATETIME_CANCEL) setup_datetime_step = SET_DATETIME_OK;
      update_display = true;
    } else if (isEncClick(enc_state)) {
      update_display = true;
      if (setup_datetime_step < SET_DATETIME_CANCEL) {
        setup_datetime_step++;
      } else if (setup_datetime_step == SET_DATETIME_OK || setup_datetime_step == SET_DATETIME_CANCEL) {
        if (setup_datetime_step == SET_DATETIME_OK) {
          DateTime datetime = DateTime(year, month, day, hour, min, sec);
          rtc.adjust(datetime);
          if (isDST(datetime)) EEPROM.update(DST_EEPROM_ADDR, DST);
          else EEPROM.update(DST_EEPROM_ADDR, STD);
        }
        setup_datetime_step = 0;
        return true;
      }
    }
    uint8_t maxdays = daysInMonth(year, month);
    if (day > maxdays) day = maxdays;
  }
  if (update_display) {
    if (setup_datetime_step < SET_DATETIME_HOUR) printDateScreen(year, month, day);
    else printTimeScreen(hour, min, sec);
  }
  update_display = false;
  return false;
}

void printDateScreen(uint16_t year, uint8_t month, uint8_t day) {
  display.clearDisplay();

  display.setCursor((SCREEN_WIDTH - 10 * TEXT_CHAR_WIDTH) / 2, 0);
  if (setup_datetime_step == SET_DATETIME_YEAR) {
    display.setTextColor(BLACK, WHITE);
    display.print(year);
    display.setTextColor(WHITE);
  } else {
    display.print(year);
  }
  display.print('-');
  char s[3];
  format2Digits(month, s);
  if (setup_datetime_step == SET_DATETIME_MONTH) {
    displayPrintInvColor(s);
  } else {
    display.print(s);
  }
  display.print('-');
  format2Digits(day, s);
  if (setup_datetime_step == SET_DATETIME_DAY) {
    displayPrintInvColor(s);
  } else {
    display.print(s);
  }
  display.display();
}

void printTimeScreen(uint8_t hour, uint8_t min, uint8_t sec) {
  display.clearDisplay();

  display.setCursor((SCREEN_WIDTH - 8 * TEXT_CHAR_WIDTH) / 2, 0);
  char s[3];
  format2Digits(hour, s);
  if (setup_datetime_step == SET_DATETIME_HOUR) {
    displayPrintInvColor(s);
  } else {
    display.print(s);
  }
  display.print(':');
  format2Digits(min, s);
  if (setup_datetime_step == SET_DATETIME_MIN) {
    displayPrintInvColor(s);
  } else {
    display.print(s);
  }
  display.print(':');
  format2Digits(sec, s);
  if (setup_datetime_step == SET_DATETIME_SEC) {
    displayPrintInvColor(s);
  } else {
    display.print(s);
  }

  display.setCursor(0, TEXT_CHAR_HEIGHT);
  if (setup_datetime_step == SET_DATETIME_CANCEL) {
    displayPrintInvColor(F("[CANCEL]"));
  } else {
    display.print(F("[CANCEL]"));
  }

  display.setCursor(SCREEN_WIDTH - 4 * TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT);
  if (setup_datetime_step == SET_DATETIME_OK) displayPrintInvColor(F("[OK]"));
  else display.print(F("[OK]"));

  display.display();
}

uint8_t daysInMonth(uint16_t year, uint8_t month) {
  if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) return 31;
  if (month == 2) {
    return (year - 2000) % 4 == 0 ? 29 : 28;
  }
  return 30;
}

uint32_t toSeconds(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
  if (year >= 2000) year -= 2000;
  uint16_t thisYearDays = day;
  for (uint8_t i = 1; i < month; i++) thisYearDays += daysInMonth(year, i);
  uint32_t days = (thisYearDays + 365 * year + year / 4);
  return (((days * 24 + hour) * 60) + min) * 60 + sec;
}

void formatMinutes(uint16_t minutes, char str[]) {
  str[0] = '0' + ((minutes / 60) / 10);
  str[1] = '0' + ((minutes / 60) % 10);
  str[2] = ':';
  str[3] = '0' + ((minutes % 60) / 10);
  str[4] = '0' + ((minutes % 60) % 10);
  str[5] = '\0';
}

void formatTime(uint8_t hours, uint8_t minutes, char str[]) {
  str[0] = '0' + (hours / 10);
  str[1] = '0' + (hours % 10);
  str[2] = ':';
  str[3] = '0' + (minutes / 10);
  str[4] = '0' + (minutes % 10);
  str[5] = '\0';
}
