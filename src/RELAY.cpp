#include <Arduino.h>

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
const int orange_pin = 20;//battery charging relay pin


//relay output init
void relay_init(){

  pinMode(orange_pin, OUTPUT);
  digitalWrite(orange_pin, HIGH); //relay = OFF
  doPowerOn=true; //ожидание зарядки и включение питания
}

// relay control logic processing (for Orange Pi to right restarting..)
void relay_control(){

  //high threshold event (voltage go from low to high __/-- )
  if(real_voltage >= alarm_h && old_real_voltage < alarm_h){
    Serial.println("....rizing near HIGHT");
    //проверка, если реле выключено, тогда просто включанм его..
    if(digitalRead(orange_pin)==1)
      doPowerOn=true; //ожидание зарядки и включение питания

  }
  //low threshold event (voltage go from high to low --\__ )
  else if(real_voltage <= alarm_l && old_real_voltage > alarm_l){
    //digitalWrite(orange_pin, LOW);
    Serial.println("....failing near LOW");

  }
  else if(real_voltage < alarm_l && old_real_voltage < alarm_l){
    //Serial.println("....high zone");
  }
  else if(real_voltage > alarm_h && old_real_voltage > alarm_h){
    //Serial.println("....low zone");

  } else {
    //Serial.println("....middle zone");
  }

  //shutdown handling
  if (doShutdown && ble_pcounter>9){
    digitalWrite(orange_pin, HIGH); //relay = OFF
    dispstatus = "OFF";
    doShutdown=false;
    Serial.println("Event-Shutdown: "+String(ble_pcounter));
  }
  //power on handling, if the battery is charged
  if (doPowerOn && real_voltage>alarm_h && old_real_voltage > alarm_h){
    digitalWrite(orange_pin, LOW); //relay = ON
    dispstatus = "PON";
    doPowerOn=false;
    Serial.println("Event-PowerOn: "+String(ble_pcounter));
  }
  //DEBUG
  //Serial.println("Relay: "+String(digitalRead(orange_pin)));
  
}