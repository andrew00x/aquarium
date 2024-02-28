#define MENU_SIZE 3
const char *menu[] = {
  "Time",
  "Channels",
  "Reset"
};

void drawMainMenuScreen() {
  static int8_t screen_pos = -1;
  if (screen_pos < 0) screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    screen_pos = incVal(screen_pos, 1, MENU_SIZE);
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    screen_pos = decVal(screen_pos, 1, 0);
    update_display = true;
  } else if (isEncClick(enc_state)) {
    update_display = true;
    if (screen_pos == MENU_SIZE) {
      screen_pos = -1;
      mode = MAIN;
    }
    if (screen_pos == 0) mode = SET_DATETIME;
    if (screen_pos == 1) mode = CHANNELS;
    if (screen_pos == 2) mode = RESET;
    update_display = true;
    return;
  }
  if (update_display) {
    display.clearDisplay();
    display.setCursor(0, 0);
    printLines(menu, screen_pos, MENU_SIZE);
    if (screen_pos == MENU_SIZE) displayPrintlnInvColor(F("[BACK]"));
    display.display();
    update_display = false;
  }
}
