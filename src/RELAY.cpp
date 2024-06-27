#include <Arduino.h>

//Global Variables
//extern String ds1;
//extern String ds2;

extern float real_voltage;
extern float old_real_voltage;
extern float alarm_h;
extern float alarm_l;

//Variables
const int orange_pin = 20;//battery charging relay pin


//relay output init
void relay_init(){

  pinMode(orange_pin, OUTPUT);
  digitalWrite(orange_pin, HIGH); //relay = OFF
}

// relay control logic processing (for Orange Pi to right restarting..)
void relay_control(){

  //high threshold event (voltage go from low to high __/-- )
  if(real_voltage >= alarm_h && old_real_voltage < alarm_h){
    //digitalWrite(orange_pin, HIGH);
    Serial.println("....rizing near HIGHT");
  }
  //low threshold event (voltage go from high to low --\__ )
  else if(real_voltage <= alarm_l && old_real_voltage > alarm_l){
    //digitalWrite(orange_pin, LOW);
    Serial.println("....failing near LOW");

  }
  else if(real_voltage < alarm_l && old_real_voltage < alarm_l){
    //Serial.println("....high zone");
  }
  else if(real_voltage > alarm_l && old_real_voltage > alarm_l){
    //Serial.println("....low zone");

  } else {
    //Serial.println("....middle zone");
  }
  
}