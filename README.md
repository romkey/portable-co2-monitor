# Portable CO2 Monitor

This is a quick hack to throw together a portable CO2 monitor. I'm attending an event and was curious about airflow and ventilation in the rooms. Logging CO2 levels is a helpful way to see if rooms are well ventilated and air is being exchanged.

## Operation

The monitor logs CO2, temperature and humidity readings once every minute and then goes to deep sleep. The ESP32 has a limited amount of memory which persists during deep sleep; the monitor uses this. When this buffer is full, the monitor copies the readings to a CSV file on a SPIFFS filesystem in flash storage and starts over

After taking a reading the monitor shows the current reading and a graph of the 64 most recent readings..

## Hardware

- Unexpected Maker Feather S3 ESP32-S3
- Sparkfun Qwiic Sensirion SCD41 CO2 sensor
- Adafruit Qwiic 128x64 OLED display

I chose these for ease of use. Qwiic connectors allowed rapid prototyping without soldering or breadboards. As this project is meant to be mobile, breadboard connections would be too unreliable. 

You can certainly use other ESP32 boards and other sources of displays and CO2 sensors; you'll need to change `platformio.ini` to specify the correct board, 
