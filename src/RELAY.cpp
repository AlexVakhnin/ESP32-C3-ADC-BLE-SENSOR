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
extern long pause_counter;
extern int zone_flag;
extern int old_zone_flag;
extern boolean ac220v_flag;
extern boolean old_ac220v_flag;

//battery charging relay pin
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

 
  //критически низкий уровень для аккумулятора !!!
  // BMS-3S-1 отключает при уровне 9.3 вольта..
  if(zone_flag ==ZONE_LOW && digitalRead(orange_pin)==RELAY_ON){
    digitalWrite(orange_pin, RELAY_OFF); //relay = OFF
    Serial.println("Critically low level..RELAY OFF (CHECK ERROR..)");
    //ESP.restart();
    dispstatus = "OFF";
    doShutdown=false;
    doPowerOn=false;
    pause_counter=0;
    doPause=true;
  }

  //shutdown handling
  if (doShutdown && pause_counter > 30){    //1 мин. для shutdown
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
  if (doPause && pause_counter > 60){  //2 мин. пауза
    doPause=false;
    doShutdown = false; //отмена dhutdown
    doPowerOn=true;
    dispstatus = "WON";
  }
 
}