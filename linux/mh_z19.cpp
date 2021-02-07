#include "mh_z19.h"
#include <wiringSerial.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
using namespace std;

MHZ19::MHZ19(const char* serial){
  strcpy(m_serial, serial);
}

int MHZ19::getCO2(){
  int fd;
  if ((fd = serialOpen (m_serial, 9600)) < 0){
    cout<<  "Unable to open device: " << m_serial << endl;
    return 1;
  }
  char  cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  char cmd2[9];
  write (fd,cmd,9);
  read(fd,cmd2,9);

  int ppm = 0;
  if((int)cmd2[0] == 0xff && (int)cmd2[1] == 0x86){
    ppm = (256 * (int) cmd2[2]) + (int) cmd2[3];
  }
  return ppm;
}

