#define PRG_DURATION_STEP 5

const char *program_names[] = {
  "Program 1",
  "Program 2",
  "Program 3",
  "Program 4",
  "Program 5"
};

#define PROGRAM_SETTINGS_ARR_LENGTH 4
const char *program_settings[] = {
  "Enabled: ",
  "Start time: ",
  "Work time: ",
  "Value: "
};

uint8_t program_pos = 0;
program active_program;

void drawProgramsScreen() {
  if (isEncTurnCW(enc_state)) {
    program_pos = incVal(program_pos, 1, PROGRAM_PER_CHANNEL);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    program_pos = decVal(program_pos, 1, 0);
    update_display = true;
  } else if (isEncClick(enc_state)) {
    if (program_pos == PROGRAM_PER_CHANNEL) {
      program_pos = 0;
      EEPROM.get(channelAddr(channel_pos), active_channel);
      mode = CHANNEL_SETTINGS;
    } else {
      EEPROM.get(programAddr(channel_pos, program_pos), active_program);
      mode = PROGRAM_SETTINGS;
    }
    update_display = true;
    return;
  }
  if (update_display) {
    display.clearDisplay();
    display.setCursor(0, 0);
    printLines(program_names, program_pos, PROGRAM_PER_CHANNEL);
    if (program_pos == PROGRAM_PER_CHANNEL) displayPrintlnInvColor(F("[BACK]"));
    display.display();
    update_display = false;
  }
}

void drawProgramSettingsScreen() {
  static bool hold_lock = false;
  static uint8_t screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    screen_pos = incVal(screen_pos, 1, PROGRAM_SETTINGS_ARR_LENGTH);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    screen_pos = decVal(screen_pos, 1, 0);
    update_display = true;
  } else if (screen_pos == 0 && isEncHold(enc_state) && !hold_lock) {
    hold_lock = true;
    active_program.enabled = !active_program.enabled;
    active_program.val = active_program.enabled ? active_channel.maxVal : active_channel.offVal;
    EEPROM.put(programAddr(channel_pos, program_pos), active_program);
    resetChannelTimer(channel_pos);
    update_display = true;
  } else if (isEncRelease(enc_state)) {
    hold_lock = false;
  } else if (isEncClick(enc_state)) {
    if (screen_pos == PROGRAM_SETTINGS_ARR_LENGTH) {
      screen_pos = 0;
      mode = PROGRAMS;
    } else if (screen_pos == 1) {
      mode = PROGRAM_SETTINGS_START_TIME;
    } else if (screen_pos == 2) {
      mode = PROGRAM_SETTINGS_DURATION;
    } else if (screen_pos == 3 && active_channel.kind == PWM_KIND) {
      mode = PROGRAM_SETTINGS_VALUE;
    }
    update_display = true;
    return;
  }
  if (update_display) {
    char *currProgramSettings[PROGRAM_SETTINGS_ARR_LENGTH];
    currProgramSettings[0] = active_program.enabled ? "YES" : "NO";

    char startStr[6];
    formatTime(active_program.startAtHour, active_program.startAtMin, startStr);
    currProgramSettings[1] = startStr;

    char workTime[6];
    formatMinutes(active_program.workMin, workTime);
    currProgramSettings[2] = workTime;

    if (active_channel.kind == PWM_KIND) {
      char val[4];
      itoa(convertActiveProgramValueToPercentages(), val, 10);
      currProgramSettings[3] = val;
    } else {
      currProgramSettings[3] = "--";
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    printTable(program_settings, currProgramSettings, screen_pos, PROGRAM_SETTINGS_ARR_LENGTH);
    if (screen_pos == PROGRAM_SETTINGS_ARR_LENGTH) displayPrintlnInvColor(F("[BACK]"));
    display.display();
    update_display = false;
  }
}

void drawProgramSettingsStartTimeScreen() {
  static uint8_t screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    if (screen_pos == 0) active_program.startAtHour = incVal(active_program.startAtHour, 1, 23);
    else if (screen_pos == 1) active_program.startAtMin = incVal(active_program.startAtMin, 1, 59);
    else if (screen_pos == 2) screen_pos = 3;
    else if (screen_pos == 3) screen_pos = 2;
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    if (screen_pos == 0) active_program.startAtHour = decVal(active_program.startAtHour, 1, 0);
    else if (screen_pos == 1) active_program.startAtMin = decVal(active_program.startAtMin, 1, 0);
    else if (screen_pos == 2) screen_pos = 3;
    else if (screen_pos == 3) screen_pos = 2;
    update_display = true;
  } else if (isEncClick(enc_state)) {
    update_display = true;
    if (screen_pos < 2) {
      screen_pos++;
    } else {
      if (screen_pos == 3) {
        EEPROM.put(programAddr(channel_pos, program_pos), active_program);
        resetChannelTimer(channel_pos);
      } else {
        EEPROM.get(programAddr(channel_pos, program_pos), active_program);
      }
      screen_pos = 0;
      mode = PROGRAM_SETTINGS;
      return;
    }
  }
  if (update_display) {
    display.clearDisplay();

    display.setCursor((SCREEN_WIDTH - 5 * TEXT_CHAR_WIDTH) / 2, 0);
    char s[3];
    format2Digits(active_program.startAtHour, s);
    if (screen_pos == 0) displayPrintInvColor(s);
    else display.print(s);
    display.print(':');
    format2Digits(active_program.startAtMin, s);
    if (screen_pos == 1) displayPrintInvColor(s);
    else display.print(s);

    display.setCursor(0, TEXT_CHAR_HEIGHT);
    if (screen_pos == 2) displayPrintInvColor(F("[CANCEL]"));
    else display.print(F("[CANCEL]"));

    display.setCursor(SCREEN_WIDTH - 4 * TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT);
    if (screen_pos == 3) displayPrintInvColor(F("[OK]"));
    else display.print(F("[OK]"));

    display.display();
    update_display = false;
  }
}

void drawProgramSettingsDurationScreen() {
  static uint8_t screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    if (screen_pos == 0) active_program.workMin = incVal(active_program.workMin, PRG_DURATION_STEP, 1440);
    else if (screen_pos == 1) screen_pos = 2;
    else if (screen_pos == 2) screen_pos = 1;
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    if (screen_pos == 0) active_program.workMin = decVal(active_program.workMin, PRG_DURATION_STEP, PRG_DURATION_STEP);
    else if (screen_pos == 1) screen_pos = 2;
    else if (screen_pos == 2) screen_pos = 1;
    update_display = true;
  } else if (isEncClick(enc_state)) {
    update_display = true;
    if (screen_pos < 1) {
      screen_pos++;
    } else {
      if (screen_pos == 2) {
        EEPROM.put(programAddr(channel_pos, program_pos) + 3, active_program.workMin);
        resetChannelTimer(channel_pos);
      } else {
        EEPROM.get(programAddr(channel_pos, program_pos) + 3, active_program.workMin);
      }
      screen_pos = 0;
      mode = PROGRAM_SETTINGS;
      return;
    }
  }
  if (update_display) {
    display.clearDisplay();

    display.setCursor((SCREEN_WIDTH - 5 * TEXT_CHAR_WIDTH) / 2, 0);
    char workTime[6];
    formatMinutes(active_program.workMin, workTime);
    display.print(workTime);

    display.setCursor(0, TEXT_CHAR_HEIGHT);
    if (screen_pos == 1) displayPrintInvColor(F("[CANCEL]"));
    else display.print(F("[CANCEL]"));

    display.setCursor(SCREEN_WIDTH - 5 * TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT);
    if (screen_pos == 2) displayPrintInvColor(F("[OK]"));
    else display.print(F("[OK]"));

    display.display();
    update_display = false;
  }
}

void drawProgramSettingsValueScreen() {
  static uint8_t screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    if (screen_pos == 0) active_program.val = incVal(active_program.val, 1, active_channel.maxVal);
    else if (screen_pos == 1) screen_pos = 2;
    else if (screen_pos == 2) screen_pos = 1;
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    if (screen_pos == 0) active_program.val = decVal(active_program.val, 1, active_channel.offVal);
    else if (screen_pos == 1) screen_pos = 2;
    else if (screen_pos == 2) screen_pos = 1;
    update_display = true;
  } else if (isEncClick(enc_state)) {
    update_display = true;
    if (screen_pos < 1) {
      screen_pos++;
    } else {
      if (screen_pos == 2) {
        EEPROM.update(programAddr(channel_pos, program_pos) + 5, active_program.val);
        resetChannelTimer(channel_pos);
      } else {
        EEPROM.get(programAddr(channel_pos, program_pos) + 5, active_program.val);
      }
      screen_pos = 0;
      mode = PROGRAM_SETTINGS;
      return;
    }
  }
  pwm_duties[channel_pos] = active_program.val;
  if (update_display) {
    display.clearDisplay();

    display.setCursor((SCREEN_WIDTH - 3 * TEXT_CHAR_WIDTH) / 2, 0);
    display.print(convertActiveProgramValueToPercentages());

    display.setCursor(0, TEXT_CHAR_HEIGHT);
    if (screen_pos == 1) displayPrintInvColor(F("[CANCEL]"));
    else display.print(F("[CANCEL]"));

    display.setCursor(SCREEN_WIDTH - 4 * TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT);
    if (screen_pos == 2) displayPrintInvColor(F("[OK]"));
    else display.print(F("[OK]"));

    display.display();
    update_display = false;
  }
}

void resetChannelTimer(uint8_t ch) {
  active_channel.timer = 0;
  EEPROM.put(channelAddr(ch) + 7, active_channel.timer);
}

long convertActiveProgramValueToPercentages() {
  // return active_program.val;
  return map(active_program.val, active_channel.offVal, active_channel.maxVal, 0, 100);
}
