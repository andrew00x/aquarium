#define ENC_CH_A_PIN A2
#define ENC_CH_B_PIN A3
#define ENC_BUTTON_PIN 2

#define ENC_TURN_DEBOUNCE 5
#define ENC_BTN_DEBOUNCE 50
#define ENC_BTN_HOLD_TIME 2000
#define ENC_NOTHING 0
#define ENC_TURN_CW 1
#define ENC_TURN_CCW 2
#define ENC_CLICK 4
#define ENC_HOLD 8

uint32_t debounce_timer;
uint8_t prev_state;
bool btn_flag;

void initEncoder() {
  pinMode(ENC_CH_A_PIN, INPUT_PULLUP);
  pinMode(ENC_CH_B_PIN, INPUT_PULLUP);
  pinMode(ENC_BUTTON_PIN, INPUT_PULLUP);

  prev_state = (digitalRead(ENC_CH_A_PIN) << 1) + digitalRead(ENC_CH_B_PIN);
  btn_flag = 0;
}

uint8_t tickEncoder() {
  uint8_t encState = ENC_NOTHING;
  uint32_t debounceDelta = millis() - debounce_timer;

  uint8_t curState = (digitalRead(ENC_CH_A_PIN) << 1) + digitalRead(ENC_CH_B_PIN);
  if (curState == 0 && debounceDelta > ENC_TURN_DEBOUNCE) {
    debounce_timer = millis();
    debounceDelta = 0;
    if (prev_state == 1) encState |= ENC_TURN_CW;
    if (prev_state == 2) encState |= ENC_TURN_CCW;
  }
  prev_state = curState;

  uint8_t btnState = !digitalRead(ENC_BUTTON_PIN);
  if (btnState && !btn_flag && debounceDelta > ENC_BTN_DEBOUNCE) {
    btn_flag = true;
    debounceDelta = 0;
    debounce_timer = millis();
    encState |= ENC_CLICK;
  }
  if (!btnState && btn_flag && debounceDelta > ENC_BTN_DEBOUNCE) {
    btn_flag = false;
    debounceDelta = 0;
    debounce_timer = millis();
  }
  if (btn_flag && debounceDelta > ENC_BTN_HOLD_TIME) {
    if (btnState) {
      encState |= ENC_HOLD;
    } else {
      debounceDelta = 0;
      debounce_timer = millis();
      btn_flag = false;
    }
  }

  return encState;
}

bool isEncTurnCW(uint8_t encState) {
  return (encState & 0b11) == ENC_TURN_CW;
}

bool isEncTurnCCW(uint8_t encState) {
  return (encState & 0b11) == ENC_TURN_CCW;
}

bool isEncClick(uint8_t encState) {
  return (encState & 0b100) == ENC_CLICK;
}

bool isEncHold(uint8_t encState) {
  return (encState & 0b1000) == ENC_HOLD;
}

bool isEncRelease(uint8_t encState) {
  return (encState & 0b1100) == 0;
}
