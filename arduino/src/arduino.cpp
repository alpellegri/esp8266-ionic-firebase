#include <Arduino.h>

#include <Ticker.h>

#include <stdio.h>
#include <string.h>

#include "ap.h"
#include "sta.h"
#include "ee.h"

#define LED     D0 // Led in NodeMCU at pin GPIO16 (D0).
#define BUTTON  D3 // flash button at pin GPIO00 (D3)

Ticker flipper;

int mode;
int flip_mode = 1;
int count = 0;

int status_scheduler = 10;
int scheduler_cnt = 0;
bool scheduler_flag = false;

void setflip_mode(int mode)
{
  flip_mode = mode;
}

void flip(void)
{
  int trig;

  if (flip_mode == 0)
  {
    trig = (flip_mode == 0)?(1):(5);
    if (count >= trig)
    {
      int state = digitalRead(LED);
      digitalWrite(LED, !state); // set pin to the opposite state
      count = 0;
    }
  }
  else if (flip_mode == 2)
  {
    digitalWrite(LED, 0);
  }
  else
  {
  }
  count++;

  /* scheduler */
  if (scheduler_cnt < status_scheduler)
  {
    scheduler_cnt++;
  }
  else
  {
    scheduler_cnt = 0;
    scheduler_flag = 1;
  }
}

void setup()
{
  bool ret;

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT);
  Serial.begin(115200);

  EE_setup();

  flipper.attach(0.1, flip);

  Serial.println();
  Serial.println("Starting");

  mode = 0;
  if (mode == 0)
  {
    ret = AP_Setup();
  }
  else if (mode == 1)
  {
    ret = STA_Setup();
    if (ret == false)
    {
      mode = 0;
      AP_Setup();
    }
  }
  else
  {
  }
}

void loop() {
  bool ret;
  // Serial.printf("loop %x\n", cnt);
  // Serial.printf("heap: %d\n\n", ESP.getFreeHeap());

  if (mode == 0)
  {
    AP_Loop();
  }
  else if (mode == 1)
  {
    STA_Loop();
  }
  else
  {
  }

  if (scheduler_flag == true)
  {
    scheduler_flag = false;
    if (mode == 0)
    {
      ret = AP_Task();
      if (ret == false)
      {
        /* try to setup STA */
        mode = STA_Setup();
      }
    }
    else if (mode == 1)
    {
      ret = STA_Task();
      if (ret == false)
      {
        mode = 0;
        AP_Setup();
      }
    }
    else
    {
    }
  }
}
