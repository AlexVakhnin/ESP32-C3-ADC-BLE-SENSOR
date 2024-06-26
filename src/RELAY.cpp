#include <Arduino.h>

//Global Variables
//extern String ds1;
//extern String ds2;

extern float real_voltage;
extern float old_real_voltage;
extern float alarm_h;
extern float alarm_l;

//Variables
const int charg_pin = 20;//battery charging relay pin


//relay output init
void relay_init(){

  pinMode(charg_pin, OUTPUT);
  digitalWrite(charg_pin, HIGH); //relay = OFF
}

// relay control logic processing
void relay_control(){

  //high thresholde event (voltage go from low to high __/-- )
  if(real_voltage >= alarm_h && old_real_voltage < alarm_h){
    digitalWrite(charg_pin, HIGH); //charging = OFF
    Serial.println("....charging = OFF");
  }
  //low thresholde event (voltage go from high to low --\__ )
  else if(real_voltage <= alarm_l && old_real_voltage > alarm_l){
    digitalWrite(charg_pin, LOW); //charging = ON
    Serial.println("....charging = ON");

  }
  
}