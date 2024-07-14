#include <Arduino.h>

#define RELAY_ON 1
#define RELAY_OFF 0
#define ZONE_HIGH 1
#define ZONE_MIDDLE 0
#define ZONE_LOW 2


extern float alarm_h;
extern float alarm_l;
extern boolean doShutdown;
extern String dispstatus;
extern boolean doPowerOn;
extern boolean doPause;
//extern long ble_pcounter;
extern long pause_counter;
extern int zone_flag;
extern int old_zone_flag;
extern boolean ac220v_flag;
extern boolean old_ac220v_flag;

//Variables
//const int orange_pin = ORANGE_RELAY_PIN;//battery charging relay pin
extern int orange_pin;

//relay output init
void relay_init(){

  pinMode(orange_pin, OUTPUT);
  digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
  dispstatus = "WON"; //wait baterry charging
  doPowerOn=true; //ожидание зарядки и включение питания
}

// relay control logic processing (for Orange Pi to right restarting..)
void relay_control(){

  /*
  //переход на заряд достаточный для включения питания
  if(old_zone_flag == ZONE_MIDDLE && zone_flag ==ZONE_HIGH){
    Serial.println("Event- TO HIGH..");
    //проверка, если реле выключено, тогда включанм его..
    if(digitalRead(orange_pin)==RELAY_OFF){
      dispstatus = "WON"; //wait baterry charging
      doShutdown = false; //отмена dhutdown
      doPowerOn=true; //ожидание зарядки и включение питания
    }
  }
  */

  /*
  //переход на критически низкий уровень для аккумулятора !!!
  else if(old_zone_flag == ZONE_MIDDLE && zone_flag ==ZONE_LOW){
    Serial.println("Event- TO LOW..");
    //проверка, если реле включено, тогда выключанм его..
    if(digitalRead(orange_pin)==RELAY_ON){
      //ble_pcounter=0;
      //dispstatus = "WOF";
      //doShutdown = true;
      digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
      dispstatus = "OFF";
      doShutdown=false;
      Serial.println("Сritically low level..RELAY OFF");
    }
  }
*/

  //критически низкий уровень для аккумулятора !!!
  if(zone_flag ==ZONE_LOW && digitalRead(orange_pin)==RELAY_ON){
    digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
    Serial.println("Сritically low level..RELAY OFF (CHECK ERROR..)");
  }

  //shutdown handling
  if (doShutdown && pause_counter > 9){
    digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
    dispstatus = "OFF";
    doShutdown=false;
    doPowerOn=false;
    pause_counter=0;
    doPause=true;
    Serial.println("Event-Shutdown: "+String(pause_counter));
  }

  //power on handling
  if (doPowerOn && zone_flag == ZONE_HIGH && ac220v_flag){ //AC 220v ON !!!
    digitalWrite(orange_pin, RELAY_ON); //relay = ON
    dispstatus = "ON ";
    doPowerOn=false;
    doShutdown = false;
    doPause=false;
    Serial.println("Event-PowerOn..");
  }

  //pause handling
  if (doPause && pause_counter > 19){
    doPause=false;
    doShutdown = false; //отмена dhutdown
    doPowerOn=true;
    dispstatus = "WON";
  }



  //DEBUG
  //Serial.println("Relay: "+String(digitalRead(orange_pin)));
  
}