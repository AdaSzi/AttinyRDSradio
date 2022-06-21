# AttinyRDSradio

FM radio receiver with Attiny85 RDA5807M. Receives RDS PS, RT and CT

<img src="https://user-images.githubusercontent.com/81822538/113437651-4b995300-93e7-11eb-9ea8-aa49ccadae95.jpg" width="736" height="414">

This is FM radio receiver with RDS functionality based on Attiny85, RDA5807M radio receiver module and 32x128px SSD1306 OLED screen.

## Controls:

- To change volume press Left or Right button (volume range is 0 - 15)
- To seek frequencies hold Middle button and press Left or Right button

## Display:

Display shows this information

- RSSI of current station
- Current volume
- Last received RDS time
- Is the radio is tuned properly?
- Is the station is stereo or mono?
- Is the RDS being received?
- Currenty tuned frequency
- RDS station name (if received)
- RDS text (if received) (scrolling, if it can`t fit on screen)

## Wiring:

3 buttons connected in pulldown configuration:

- Left button   gpio 3 (Attiny85 chip pin 2) 
- Middle button gpio 4 (Attiny85 chip pin 3)
- Right button  gpio 1 (Attiny85 chip pin 6)

All parts should be powered by 3,3V

Rest is standard I2C wiring

## Libraries

AttinyRDSradio uses version of mr. [Matthias Hertel](github.com/mathertel)`s [Radio](https://github.com/mathertel/Radio) library that was modified to work on Attiny85. You can find the modified version in repository of this project

Additionally you will need theese libraries:

- [TinyWireM](github.com/adafruit/TinyWireM)
- [Tiny4KOLED](github.com/datacute/Tiny4kOLED)

## Notes

This program only fits on Attiny85 because it uses 8190 Bytes of Attiny85`s 8192 Bytes of memory

Attiny85 has to run 16 MHz
