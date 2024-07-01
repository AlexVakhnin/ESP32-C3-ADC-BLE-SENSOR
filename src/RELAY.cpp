#include <Arduino.h>

#define RELAY_ON 0x0
#define RELAY_OFF 0x1

//Global Variables
//extern String ds1;
//extern String ds2;

extern float real_voltage;
extern float old_real_voltage;
extern float alarm_h;
extern float alarm_l;
extern boolean doShutdown;
extern long ble_pcounter;
extern String dispstatus;
extern boolean doPowerOn;

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

  //high threshold event (voltage go from low to high __/-- )
  if(real_voltage >= alarm_h && old_real_voltage < alarm_h){
    Serial.println("Event- TO HIGHT..");
    //проверка, если реле выключено, тогда просто включанм его..
    if(digitalRead(orange_pin)==RELAY_OFF)
      doPowerOn=true; //ожидание зарядки и включение питания

  }
  //low threshold event (voltage go from high to low --\__ )
  //критически низкий уровень для аккумулятора !!!
  else if(real_voltage <= alarm_l && old_real_voltage > alarm_l){
    Serial.println("Event- TO LOW..");
    //проверка, если реле включено, тогда просто выключанм его..
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
  //power on handling, if the battery is charged
  if (doPowerOn && real_voltage>alarm_h && old_real_voltage > alarm_h){
    digitalWrite(orange_pin, RELAY_ON); //relay = ON
    dispstatus = "ON ";
    doPowerOn=false;
    Serial.println("Event-PowerOn..");
  }
  //DEBUG
  //Serial.println("Relay: "+String(digitalRead(orange_pin)));
  
}