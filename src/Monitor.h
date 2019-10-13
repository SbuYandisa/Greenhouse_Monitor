#ifndef MONITOR_H
#define MONITOR_H

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <math.h>
#include <mcp3004.h>
// SPI settings
#define SPI_CHAN 0      //Sets the spi channel
#define BASE 100
//definition of functions
int hFormat(int hours);
void initGPIO(void);
void getTime(void);
int analogRead(int channel);
void readingInterval(void);
void dismissAlarm(void);
void ledPWM(void);
void reset(void);
void stop_start(void);
//constant defintion
const char RTCaddress = 0x6f;
const char SEC = 0x00;
const char MIN = 0x01;
const char HR = 0x02;
const char TIMEZONE = 2;

//pins definition
const int LED = 7;
//Buttons
const int Buttons[] = {0, 2, 3, 1};



#endif 
