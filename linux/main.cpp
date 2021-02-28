/**
 * @file       main.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wiringPiI2C.h>
#include "bme280.h"
#include "mh_z19.h"
//#define BLYNK_DEBUG
#define BLYNK_PRINT stdout
#ifdef RASPBERRY
  #include <BlynkApiWiringPi.h>
#else
  #include <BlynkApiLinux.h>
#endif
#include <BlynkSocket.h>
#include <BlynkOptionsParser.h>
#define COCORO_CONFIG "--config_file ~/.config/my_raspi_home/cocoro.yml"

static BlynkTransportSocket _blynkTransport;
BlynkSocket Blynk(_blynkTransport);

static const char *auth, *serv;
static uint16_t port;

#include <BlynkWidgets.h>

BlynkTimer tmr;

BLYNK_WRITE(V1)
{
  printf("Got a value for V%d: %s\n", V1, param[0].asStr());
  string cmd = "cocoro switch ";
  if(param[0].asInt() == 1){
    cmd += "on ";
  }else if(param[0].asInt() == 0){
    cmd += "off ";
  }
  cmd += COCORO_CONFIG;
  system(cmd.c_str());
}

BLYNK_WRITE(V2)
{
  printf("Got a value for V%d: %s\n", V2, param[0].asStr());
  string cmd = "cocoro humi ";
  if(param[0].asInt() == 1){
    cmd += "on ";
  }else if(param[0].asInt() == 0){
    cmd += "off ";
  }
  cmd += COCORO_CONFIG;
  system(cmd.c_str());
}

BLYNK_WRITE(V3)
{
  printf("Got a value for V%d: %s\n", V3, param[0].asStr());
  string cmd = "cocoro mode ";
  if(param[0].asInt() == 1){
    cmd += "recommendation ";
  }else if(param[0].asInt() == 2){
    cmd += "effective ";
  }else if(param[0].asInt() == 3){
    cmd += "auto ";
  }else if(param[0].asInt() == 4){
    cmd += "pollen ";
  }else if(param[0].asInt() == 5){
    cmd += "quiet ";
  }
  cmd += COCORO_CONFIG;
  system(cmd.c_str());
}

void bme280chart(){
  int fd = wiringPiI2CSetup(BME280_ADDRESS);
  if(fd < 0) {
    printf("BME280 Device not found");
  }

  bme280_calib_data cal;
  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);

  bme280_raw_data raw;
  getRawData(fd, &raw);

  int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);
  float t = compensateTemperature(t_fine);
  float p = compensatePressure(raw.pressure, &cal, t_fine) / 100;
  float h = compensateHumidity(raw.humidity, &cal, t_fine);

  Blynk.virtualWrite(V20,  t);
  Blynk.virtualWrite(V21,  h);
  Blynk.virtualWrite(V22,  p);
}

void mhz19Chart(){
  MHZ19 m("/dev/serial0");
  int co2 = m.getCO2();
  if(co2 != 0){
    Blynk.virtualWrite(V23,  co2);
  }
}

void myTimerEvent(){
  bme280chart();
  mhz19Chart();
}

void setup()
{
  Blynk.begin(auth, serv, port);
  tmr.setInterval(1000, myTimerEvent);

}

void loop()
{
  Blynk.run();
  tmr.run();
}


int main(int argc, char* argv[])
{
  parse_options(argc, argv, auth, serv, port);

  setup();
  while(true) {
      loop();
  }

  return 0;
}

