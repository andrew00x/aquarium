#define EEPROM_FLAG_ADDR 1022

bool isEEPROMReady() {
  return EEPROM.read(EEPROM_FLAG_ADDR) == EEPROM_FLAG;
}

void initEEPROM() {
  for (int i = 0; i < 1023; i++) {
    EEPROM.write(i, 0);
  }

  int addr = 0;
#ifdef RELAY1_PIN
  EEPROM.put(addr, channel{ RELAY_KIND, RELAY1_PIN, CHANNEL_MODE_OFF, RELAY_OFF, RELAY_ON, 0, RELAY_OFF, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef RELAY2_PIN
  EEPROM.put(addr, channel{ RELAY_KIND, RELAY2_PIN, CHANNEL_MODE_OFF, RELAY_OFF, RELAY_ON, 0, RELAY_OFF, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM1_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM1_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM2_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM2_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM3_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM3_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM4_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM4_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM5_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM5_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
  addr += EEPROM_CHANNEL_CELL_SIZE;
#endif
#ifdef PWM6_PIN
  EEPROM.put(addr, channel{ PWM_KIND, PWM6_PIN, CHANNEL_MODE_OFF, 0, PWM_DEPTH, 0, 0, 0 });
#endif

  channel ch;
  for (uint8_t c = 0; c < CHANNELS_ARR_LENGTH; c++) {
    EEPROM.get(channelAddr(c), ch);
    for (uint8_t p = 0; p < PROGRAM_PER_CHANNEL; p++) {
      EEPROM.put(programAddr(c, p), program{ 0, 0, 0, 0, ch.offVal });
    }
  }

  EEPROM.update(EEPROM_FLAG_ADDR, EEPROM_FLAG);
}
