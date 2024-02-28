#define RESET_CANCEL 0
#define RESET_OK 1

void drawResetScreen() {
  static uint8_t screen_pos = 0;
  if (isEncTurnCW(enc_state)) {
    if (screen_pos == RESET_CANCEL) screen_pos = RESET_OK;
    else if (screen_pos == RESET_OK) screen_pos = RESET_CANCEL;
    update_display = true;
  } else if (isEncTurnCCW(enc_state)) {
    if (screen_pos == RESET_CANCEL) screen_pos = RESET_OK;
    else if (screen_pos == RESET_OK) screen_pos = RESET_CANCEL;
    update_display = true;
  } else if (isEncClick(enc_state)) {
    update_display = true;
    if (screen_pos == RESET_OK) initEEPROM();
    screen_pos = 0;
    mode = MAIN;
    return;
  }
  if (update_display) {
    display.clearDisplay();

    display.setCursor((SCREEN_WIDTH - 15 * TEXT_CHAR_WIDTH) / 2, 0);
    display.print("Reset settings?");

    display.setCursor(0, TEXT_CHAR_HEIGHT);
    if (screen_pos == RESET_CANCEL) displayPrintInvColor(F("[CANCEL]"));
    else display.print(F("[CANCEL]"));

    display.setCursor(SCREEN_WIDTH - 4 * TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT);
    if (screen_pos == RESET_OK) displayPrintInvColor(F("[OK]"));
    else display.print(F("[OK]"));

    display.display();
    update_display = false;
  }
}
