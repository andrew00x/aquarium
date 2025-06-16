#define CHANNEL_SETTINGS_ARR_LENGTH 3
const char *channel_settings[] = {
  "Mode: ",
  "State: ",
  "Programs: "
};

#define CHANNEL_SETTINGS_MODE_ARR_LENGTH 3
const char *channel_modes[] = {
  "Off",
  "Auto",
  "Manual"
};

uint8_t channel_pos = 0;
channel active_channel;

void drawChannelsScreen() {
  if (isEncTurnCW(enc_state)) {
    channel_pos = incVal(channel_pos, 1, CHANNELS_ARR_LENGTH);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    channel_pos = decVal(channel_pos, 1, 0);
    update_display = true;
  } else if (isEncClick(enc_state)) {
    if (channel_pos == CHANNELS_ARR_LENGTH) {
      channel_pos = 0;
      mode = MAIN;
    } else {
      EEPROM.get(channelAddr(channel_pos), active_channel);
      mode = CHANNEL_SETTINGS;
    }
    update_display = true;
    return;
  }
  if (update_display) {
    display.clearDisplay();
    display.setCursor(0, 0);
    printLines(channel_names, channel_pos, CHANNELS_ARR_LENGTH);
    if (channel_pos == CHANNELS_ARR_LENGTH) displayPrintlnInvColor(F("[BACK]"));
    display.display();
    update_display = false;
  }
}

void drawChannelSettingsScreen() {
  static bool hold_lock = false;
  static uint8_t screen_pos = 0;
  static uint32_t read_channel_timer = 0;
  if (isEncTurnCW(enc_state)) {
    screen_pos = incVal(screen_pos, 1, CHANNEL_SETTINGS_ARR_LENGTH);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    screen_pos = decVal(screen_pos, 1, 0);
    update_display = true;
  } else if (active_channel.mode == CHANNEL_MODE_MANUAL && screen_pos == 1 && isEncHold(enc_state) && !hold_lock) {
    hold_lock = true;
    if (active_channel.val == active_channel.offVal) active_channel.val = active_channel.maxVal;
    else active_channel.val = active_channel.offVal;
    EEPROM.put(channelAddr(channel_pos), active_channel);
    update_display = true;
  } else if (isEncRelease(enc_state)) {
    hold_lock = false;
  } else if (isEncClick(enc_state)) {
    if (screen_pos == CHANNEL_SETTINGS_ARR_LENGTH) {
      screen_pos = 0;
      mode = CHANNELS;
    } else if (screen_pos == 0) {
      mode = CHANNEL_MODE_SETUP;
    } else if (screen_pos == 2) {
      mode = PROGRAMS;
    }
    update_display = true;
    return;
  }
  if (millis() - read_channel_timer > 500) {
    EEPROM.get(channelAddr(channel_pos) + 6, active_channel.val);
    update_display = true;
    read_channel_timer = millis();
  }
  if (update_display) {
    char *currChannelSettings[CHANNEL_SETTINGS_ARR_LENGTH];
    currChannelSettings[0] = channel_modes[active_channel.mode];
    currChannelSettings[1] = (active_channel.val == active_channel.offVal) ? "OFF" : "ON";
    if (active_channel.prgNum) {
      char prgNum[2];
      itoa(active_channel.prgNum, prgNum, 10);
      currChannelSettings[2] = prgNum;
    } else {
      currChannelSettings[2] = "-";
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    printTable(channel_settings, currChannelSettings, screen_pos, CHANNEL_SETTINGS_ARR_LENGTH);
    if (screen_pos == CHANNEL_SETTINGS_ARR_LENGTH) displayPrintlnInvColor(F("[BACK]"));
    display.display();
    update_display = false;
  }
}

void drawChannelModeSetupScreen() {
  if (isEncTurnCW(enc_state)) {
    active_channel.mode = incVal(active_channel.mode, 1, CHANNEL_SETTINGS_MODE_ARR_LENGTH - 1);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    active_channel.mode = decVal(active_channel.mode, 1, 0);
    update_display = true;
  } else if (isEncClick(enc_state)) {
    if (active_channel.mode == CHANNEL_MODE_AUTO) active_channel.timer = 0;
    else active_channel.val = active_channel.offVal;
    EEPROM.put(channelAddr(channel_pos), active_channel);
    mode = CHANNEL_SETTINGS;
    update_display = true;
    return;
  }
  if (update_display) {
    display.clearDisplay();
    display.setCursor(0, 0);
    printLines(channel_modes, active_channel.mode, CHANNEL_SETTINGS_MODE_ARR_LENGTH);
    display.display();
    update_display = false;
  }
}
