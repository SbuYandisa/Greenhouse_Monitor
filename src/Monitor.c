//Created: Yandisa Madinga & Live Tembiso
//Modified: 01/10/2019
/*#define BLYNK_PRINT stdout
#ifdef RASPBERRY
    #include <BlinkApiWiringPi.h>
#else
   #include <BlynkApiLinux.h> 
#endif
#include <BlynkOptionsParser.h>
#include <BlynkSocket.h>
*/
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include  "Current_Time.h"
#include "Monitor.h"
#include <pthread.h>
#include <sched.h>
#include <math.h>
//#include <softPwm.h>
typedef int bool;
#define true 1
#define false 0

int hours, mins, secs, HH, MM, SS, RTC, lightValue, tempValue, interval=1000, counter, disMin = 4, disSec, sHH, sMM, sSS;
long lastInterruptTime = 0;
bool reading = true, flash = true, stopThread = false;
float V_out, humValue;
int tempV;
float humV;
//SetGPIO
void initGPIO(void){
    wiringPiSetup(); //initializes wiringPi
    mcp3004Setup(BASE, SPI_CHAN);
    RTC = wiringPiI2CSetup(RTCaddress);           //Sets up the RTC
    pinMode(LED, OUTPUT);
    for(int i; i<sizeof(Buttons)/sizeof(Buttons[0]); i++){
        pinMode(Buttons[i], INPUT);
        pullUpDnControl(Buttons[i], PUD_UP);
    }
   //Attach interrupts to buttons
    if(wiringPiISR(0, INT_EDGE_FALLING, &readingInterval)<0){
        printf("Unable to setup interval button!");
        }
    if(wiringPiISR(2, INT_EDGE_FALLING, &dismissAlarm)<0){
        printf("Unable to setup dismiss button");
    }
    if(wiringPiISR(3, INT_EDGE_FALLING, &reset)<0){
        printf("Unable to setup reset system time button");
mcp3004Setup    }

    if(wiringPiISR(1, INT_EDGE_FALLING, &stop_start)<0){
        printf("Unable to setup stop/start button");
    }

    wiringPiI2CWriteReg8(RTC, HR, 0b0);
    wiringPiI2CWriteReg8(RTC, MIN, 0b0);
    wiringPiI2CWriteReg8(RTC, SEC, 0b10000000);

}
//-----------------------------------------------------------------------------------------------
//Change the hour format to 12 hours
int hformat(int hours){
    if(hours >= 24){
        hours = 0;
    }
    else if(hours > 12){
        hours -=12;
    }
    return (int)hours;
}
//------------------------------------------------------------------------------------------------
/*This function undoes decCompensation in order to write to the correct base 16 value through I2C */
int hexCompensation(int units){
    int unitsU = units%0x10;

    if(units>=0x50){
        units = 50 + unitsU;
    }
    else if (units>=0x40){
        units = 40 + unitsU;
    }
    else if (units>=0x30){
        units = 30 + unitsU;
    }
    else if(units>=0x20){
        units = 20 + unitsU;
    }
    else if(units>=0x10){
        units = 10 + unitsU;
    }
    return units;
}
//-----------------------------------------------------------------------------------------------
/*This function undoes hexCompensation in  order to write the correct base 16 value through I2C*/
int decCompensation(int units){
    int unitsU = units%10;
    if(units>=50){
        units = 0x50 + unitsU;
    }
    else if(units >=40){
        units = 0x40 + unitsU;
    }
    else if(units>=30){
        units = 0x30 + unitsU;
    }
    else if(units>=20){
        units = 0x20 + unitsU;
    }
    else if(units>=10){
        units = 0x10 + unitsU;
    }
    return units;
}
//----------------------------------------------------------------------------------------------
//This function is responsible for changing interval when a user pushes the button
void readingInterval(void){
    long interruptTime = millis();
    if(interruptTime-lastInterruptTime>200){
        counter++;
        if(counter==1){
            interval = 2000;
        }
        else if(counter==2){
            interval = 5000;
        }
        else{
            counter = 0;
            interval = 1000;
        }
    }
    lastInterruptTime = interruptTime;
}
//----------------------------------------------------------------------------------------------
//LED flasher acting as an alarm 
void ledPWM(void){
    if(flash==true){
        digitalWrite(LED, 1);
    }
    
    else if(flash==false){
        digitalWrite(LED, 0);
        if(disSec >= 59){
            disSec = 0;
            disMin++;
        }
        else if((disMin >= 1) && (V_out < 0.65 || V_out >2.65)){
            flash = true;
            disMin = 0;
        }
    }
}
//---------------------------------------------------------------------------------------------
//Set alarm off
void dismissAlarm(void){
    flash = false;
    digitalWrite(LED,0);
    disSec = 0;
}
//----------------------------------------------------------------------------------------------
/* reading thread*/
void *readingThread(void *threadargs){
    lightValue = analogRead(BASE + 1);
    tempValue = analogRead(BASE + 2);
    humValue = analogRead(BASE + 0);
    return 0;
}
//----------------------------------------------------------------------------------------------------

//System time
void SystemTime(void){
        if(sSS >=59){
            sMM++;
            sSS  = 0;
        }
        else if(sMM >=59){
            sHH++;
            sMM = 0;
        }
        else if(sHH >=24){
            sHH = 0;
        }
}

//-------------------------------------------------------------------------------------------------
//Reset System Time
void reset(void){
    sSS = 0;
    sMM = 0;
    sHH = 0;
}
void stop_start(void){
    stopThread  = !stopThread;
}
int main(void){
    initGPIO();
    //softPwmCreate(LED, 0, 100);
    getTime();
    pthread_t tid;
    for(;;){
       if(stopThread==false){
        pthread_create(&tid, NULL, readingThread, (void *)&tid);
        //To be changed/updated
         tempV =  (tempValue/256) + 20;
         humV = fabs((humValue-9)/310);
       //Correct volyage value over DAC
        V_out = ((float)lightValue/1023)*humV;
        }
        hours =hexCompensation(wiringPiI2CReadReg8(RTC, HR));
        mins = hexCompensation(wiringPiI2CReadReg8(RTC, MIN));
        secs = hexCompensation(wiringPiI2CReadReg8(RTC, SEC)-0b10000000);

        if(V_out < 0.65 || V_out >2.65){
            ledPWM();
        }
        printf("The LDR value: %d\n", lightValue);
        printf("The temperature value: %d\n", tempV);
        printf("The humidity value: %.1f\n", humV);
        printf("The current time is: %02d:%02d:%02d\n", hours, mins, secs);
        printf("The system time is: %02d:%02d:%02d\n", sHH, sMM, sSS);
        printf("The Vout value: %.2f\n", V_out);
        delay(interval);
        sSS += interval/1000; 
        disSec += interval/1000;
        SystemTime(); 
    }
    return 0;
}
//-----------------------------------------------------------------------------------------------------
//This functions will toggle a flag that is checked in main
void getTime(void){
    HH = getHours();
    MM = getMins();
    SS = getMins();
    
    HH = decCompensation(HH);
    wiringPiI2CWriteReg8(RTC, HR, HH);
    MM = decCompensation(MM);
    wiringPiI2CWriteReg8(RTC, MIN, MM);
    SS = decCompensation(SS);
    wiringPiI2CWriteReg8(RTC, SEC, SS+0b10000000);
}
//-------------------------------------------------------------------------------------------------------
