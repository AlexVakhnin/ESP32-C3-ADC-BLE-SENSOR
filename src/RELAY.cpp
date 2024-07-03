#include <Arduino.h>

#define RELAY_ON 1
#define RELAY_OFF 0
#define ZONE_HIGH 1
#define ZONE_MIDDLE 0
#define ZONE_LOW 2

extern float real_voltage;
extern float old_real_voltage;
extern float alarm_h;
extern float alarm_l;
extern boolean doShutdown;
extern long ble_pcounter;
extern String dispstatus;
extern boolean doPowerOn;
extern int zone_flag;
extern int old_zone_flag;

//Variables
//const int orange_pin = ORANGE_RELAY_PIN;//battery charging relay pin
extern int orange_pin;

//relay output init
void relay_init(){

  pinMode(orange_pin, OUTPUT);
  digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
  doPowerOn=true; //ожидание зарядки и включение питания
}

// relay control logic processing (for Orange Pi to right restarting..)
void relay_control(){

  //переход на заряд достаточный для включения питания
  if(old_zone_flag == ZONE_MIDDLE && zone_flag ==ZONE_HIGH){
    Serial.println("Event- TO HIGH..");
    //проверка, если реле выключено, тогда включанм его..
    if(digitalRead(orange_pin)==RELAY_OFF){
      doPowerOn=true; //ожидание зарядки и включение питания
    }
  }
  //переход на критически низкий уровень для аккумулятора !!!
  else if(old_zone_flag == ZONE_MIDDLE && zone_flag ==ZONE_LOW){
    Serial.println("Event- TO LOW..");
    //проверка, если реле включено, тогда выключанм его..
    if(digitalRead(orange_pin)==RELAY_ON){
      ble_pcounter=0;
      dispstatus = "WOF";
      doShutdown = true;
    }
  }

  //shutdown handling
  if (doShutdown && ble_pcounter>9){
    digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
    dispstatus = "OFF";
    doShutdown=false;
    Serial.println("Event-Shutdown: "+String(ble_pcounter));
  }
  //power on handling
  if (doPowerOn && zone_flag ==ZONE_HIGH){
    digitalWrite(orange_pin, RELAY_ON); //relay = ON
    dispstatus = "ON ";
    doPowerOn=false;
    Serial.println("Event-PowerOn..");
  }
  //DEBUG
  //Serial.println("Relay: "+String(digitalRead(orange_pin)));
  
}