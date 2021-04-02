#include <Arduino.h>
#include <avr/eeprom.h>
#include <EEPROM.h>

#include <TinyWireM.h>

#include <radio.h>
#include <RDA5807M.h>
#include <RDSParser.h>

#include <Tiny4kOLED.h>
#define TINY4KOLED_QUICK_BEGIN

#include "font.h"

#define pinA 1
#define pinB 3
#define pinC 4
//volatile bool doButtons = false;
volatile bool buttonUp = false;
volatile bool buttonDn = false;

RDA5807M radio;
RADIO_INFO info;
RDSParser rds;

bool haveRDS = false;
char RDStime[6];

struct EEPROMdataSave {
  RADIO_FREQ radioF;
  byte volume;
};
EEPROMdataSave dataSave;

unsigned long nextDispTime = 0;

byte i;

void UpdateFrequency() {
  haveRDS = false;
  updateSave();
}

void updateSave() {
  delay(50);
  dataSave.radioF = radio.getFrequency();
  EEPROM.put(127, dataSave);
}

void updateLoad() {
  EEPROM.get(127, dataSave);
}

#define toFind 13 //CR character is transmitted at the end of RDS text
#define tfl 3 //to fill lenght - number of spaces to add
void ProcessServiceText(char *text) {
  char * pch;
  pch = strchr (text, toFind);
  if (pch != NULL) {
    i = pch - text;
    if (i < 62) {
      text[i] = ' ';
      text[i + 1] = ' ';
      text[i + 2] = ' ';
      text[i + 3] = '\0';
    }
  }
  haveRDS = true;
}

void DisplayServiceTime(byte hour, byte min) {
  if (0 < hour && hour < 25 && 0 < min && min < 60) {
    sprintf(RDStime, "%02d:%02d\0", hour, min);
  }
}

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}

// Pin change ISR
ISR (PCINT0_vect) {
  buttonUp = !digitalRead(pinA);
  buttonDn = !digitalRead(pinB);
  GIFR = 1 << PCIF;                      // Clear interrupt flag
}

void setup() {
  pinMode(pinA, INPUT_PULLUP); //set pinA as an input
  pinMode(pinB, INPUT_PULLUP); //set pinB as an input
  pinMode(pinC, INPUT_PULLUP); //set pinC as an input

  GIMSK = 1 << PCIE; //Enable interrupt
  PCMSK = 1 << pinA | 1 << pinB; //Add pinA and pinB to interrupt
  GIFR = 1 << PCIF; // Clear interrupt flag

  updateLoad();
  if (dataSave.volume == 255) dataSave.volume = 8; //When you flash this program to Attiny, EEPROM is cleared to 255. This saves you from having to click your way down to 15. If you need more program space, this should be the first line to comment out
  delay(400);
  TinyWireM.begin();

  radio.init();

  radio.setMono(false);
  radio.setMute(false);
  radio.setVolume(dataSave.volume);
  radio.setBandFrequency(RADIO_BAND_FMWORLD,  dataSave.radioF);
  delay(100);

  radio.attachReceiveRDS(RDS_process);
  rds.attachTextCallback(ProcessServiceText);
  rds.attachTimeCallback(DisplayServiceTime);

  oled.begin();
  oled.setRotation(0);
  oled.clear();

  oled.on();
  oled.switchRenderFrame();
  oled.clear();

  oled.setFont(font);
  oled.switchFrame();
  UpdateFrequency();
}

void loop() {
  if (digitalRead(pinC)) {
    if (buttonUp) {
      if (dataSave.volume < 15) {
        dataSave.volume++;
        radio.setVolume(dataSave.volume);
        updateSave();
        nextDispTime -= 400;
      }
      buttonUp = false;
    }
    if (buttonDn) {
      if (dataSave.volume > 0) {
        dataSave.volume--;
        radio.setVolume(dataSave.volume);
        updateSave();
        nextDispTime -= 400;
      }
      buttonDn = false;
    }
  }
  else {
    if (buttonUp) {
      radio.seekUp(true);
      delay(100);
      UpdateFrequency();
      buttonUp = false;
    }
    if (buttonDn) {
      radio.seekDown(true);
      delay(100);
      UpdateFrequency();
      buttonDn = false;
    }
  }

  radio.checkRDS();
  radio.getRadioInfo(&info);
  if (millis() > nextDispTime) {
    nextDispTime = millis() + 400;
    renderOled();
  }
}

int  textOffset;
#define screenCapacity 21
char buf[5];
void renderOled() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("RSSI:"));
  sprintf(buf, "%2d", info.rssi);
  oled.print(buf);
  oled.print(F(" Vol:"));
  sprintf(buf, "%2d  ", dataSave.volume);
  oled.print(buf);

  oled.print(RDStime);

  oled.setCursor(0, 1);
  if (info.tuned) {
    oled.print(F("Tuned"));
  }

  oled.setCursor(8 * 6, 1);
  if (info.stereo) {
    oled.print(F("Stereo"));
  }
  else {
    oled.print(F(" Mono "));
  }

  if (info.rds) {
    oled.print(F("   RDS"));
  }

  oled.setCursor(0, 2);
  sprintf(buf, "%3d.%d",  dataSave.radioF / 100, ( dataSave.radioF % 100) / 10);
  oled.print(buf);
  oled.print(F(" MHz    "));

  if (haveRDS) {
    oled.print(rds.programServiceName);
    oled.setCursor(0, 3);
    if (strlen(rds._RDSText) - tfl < screenCapacity) {
      oled.print(rds._RDSText);
    }
    else {
      for (i = 0; i < screenCapacity; i++) {
        if (31 < rds._RDSText[(i + textOffset) % strlen(rds._RDSText)] && rds._RDSText[(i + textOffset) % strlen(rds._RDSText)] < 126) {
          oled.print(rds._RDSText[(i + textOffset) % strlen(rds._RDSText)]);
        }
        else {
          oled.print(' ');
        }
      }
      if (textOffset++ >= strlen(rds._RDSText)) textOffset = 0, ProcessServiceText(rds._RDSText);
    }
  }
  oled.switchFrame();
}
