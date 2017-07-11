#include <Arduino.h>
#include <RCSwitch.h>

#include <stdio.h>
#include <string.h>

#include "rf.h"
#include "timesrv.h"
#include "fblog.h"

RCSwitch mySwitch = RCSwitch();
uint32_t RadioCode;
bool RF_StatusEnable = false;

uint32_t RadioCodes[10][5];
uint16_t RadioCodesLen = 0;
uint32_t RadioCodesTx[10];
uint16_t RadioCodesTxLen = 0;

uint32_t Timers[10][2];
uint16_t TimersLen = 0;

void RF_ResetRadioCodeDB(void) { RadioCodesLen = 0; }
void RF_ResetRadioCodeTxDB(void) { RadioCodesTxLen = 0; }

uint32_t t247_last = 0;
void RF_ResetTimerDB(void) {
  time_t mytime = getTime();
  t247_last = 60*((mytime/3600)%24) + (mytime/60)%60;
  TimersLen = 0;
}

void RF_AddRadioCodeDB(String id, String type, String action, String delay, String action_d) {
  RadioCodes[RadioCodesLen][0] = atoi(id.c_str());
  RadioCodes[RadioCodesLen][1] = atoi(type.c_str());
  RadioCodes[RadioCodesLen][2] = atoi(action.c_str());
  RadioCodes[RadioCodesLen][3] = atoi(delay.c_str());
  RadioCodes[RadioCodesLen][4] = atoi(action_d.c_str());
  RadioCodesLen++;
}

void RF_AddRadioCodeTxDB(String string) {
  RadioCodesTx[RadioCodesTxLen] = atoi(string.c_str());
  RadioCodesTxLen++;
}

void RF_AddTimerDB(String action, String hour, String minute) {
  uint32_t evtime = 60*atoi(hour.c_str()) + atoi(minute.c_str());
  Timers[TimersLen][0] = atoi(action.c_str());
  Timers[TimersLen][1] = evtime;
  TimersLen++;
}

bool RF_CheckRadioCodeDB(uint32_t code) {
  bool res = false;

  uint8_t i = 0;
  Serial.printf("RF_CheckRadioCodeDB: code %x\n", code);
  while ((i < RadioCodesLen) && (res == false)) {
    Serial.printf("radio table: %x, %x\n", code, RadioCodes[i]);
    if (code == RadioCodes[i][0]) {
      Serial.printf("radio code found in table\n");
      res = true;
    }
    i++;
  }

  return res;
}

uint32_t RF_GetRadioCode(void) {
  uint32_t Code;

  Code = RadioCode;
  RadioCode = 0;

  return Code;
}

bool RF_TestInRange(uint32_t t_test, uint32_t t_low, uint32_t t_high) {
  bool ret = false;
  Serial.printf(">> %d, %d, %d\n", t_low, t_test, t_high);
  ret = (t_test >= t_low) && (t_test <= t_high);
  return ret;
}

void RF_MonitorTimers(void) {
  // get time
  time_t mytime = getTime();
  // Serial.printf(">> %d, %d, %d\n", (mytime/3600)%24, (mytime/60)%60, (mytime)%60);
  uint32_t t247 = 60*((mytime/3600)%24) + (mytime/60)%60;
  Serial.printf(">> t247 %d\n", t247);
  // loop over timers
  for (uint8_t i=0; i<TimersLen; i++) {
    // test in range
    bool res = RF_TestInRange(Timers[i][1], t247_last, t247);
    if (res == true) {
      // action
      Serial.printf(">>>>>>>>>>>>>>>>>>>> action on timer %d at time %d\n", i, t247);
      String log = "action on timer " + String(i) + " at time " + String(t247) + "\n";
      fblog_log(log, false);
    }
  }
  t247_last = t247;
}

void RF_Enable(void) {
  if (RF_StatusEnable == false) {
    RF_StatusEnable = true;
    RadioCode = 0;
    Serial.printf("RF Enable\n");
    mySwitch.enableReceive(D7); // gpio13 D7
  }
}

void RF_Disable(void) {
  if (RF_StatusEnable == true) {
    RF_StatusEnable = false;
    RadioCode = 0;
    Serial.printf("RF disable\n");
    mySwitch.disableReceive(); // gpio13 D7
  }
}

void RF_Loop() {
  if (mySwitch.available()) {

    uint32_t value = (uint32_t)mySwitch.getReceivedValue();
    mySwitch.resetAvailable();

    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      // Serial.printf(">>%x\n", value);
      // Serial.print(" / ");
      // Serial.print(mySwitch.getReceivedBitlength());
      // Serial.print("bit ");
      // Serial.print("Protocol: ");
      // Serial.println(mySwitch.getReceivedProtocol());
      if (RadioCode == 0) {
        Serial.printf("radio code: %x\n", value);
        RadioCode = value;
      } else {
        Serial.printf(".\n", value);
      }
    }
  }
}

/* main function task */
bool RF_Task(void) {
  bool ret = true;

  return ret;
}
