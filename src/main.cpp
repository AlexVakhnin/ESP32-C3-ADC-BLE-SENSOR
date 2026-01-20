#include <Arduino.h>
//#include <Ticker.h>
//#include <nvs_flash.h> //для стирания всего NVRAM

#define ORANGE_RELAY_PIN 20
#define AC220V_THRESHOLD 7

extern void ble_setup();
extern void relay_control();
extern void relay_init();
//void alarm_blink();
void relay_control();

const int sens_pin = 0;//A0 for ADC0
const int sens1_pin = 1;//A1 for ADC1
int orange_pin = ORANGE_RELAY_PIN;

String dev_name = "UPS-RASP-PI"; //name of BLE service

int sens_value = 0; //pure sensor 0
int sens1_value = 0; //pure sensor 1
float sens_voltage = 0; //Voltage ADC0 Input
float sens1_voltage = 0; //Voltage ADC1 Input
double factor = 5.13; //calibration attenuator0 factor
double factor1 = 5.06; //calibration attenuator1 factor
double adc_calibr =2.99; //calibration ADC factor (опорное напряжение)
float real_voltage =0; //measuring voltage ADC0 with attenuator
float real1_voltage =0; //measuring voltage ADC1 with attenuator
float old_real_voltage=0; //contains the result of the previous measurement
float old_real1_voltage=0; //contains the result of the previous measurement

float alarm_h =12.00; //high threshold value
float alarm_l =10.00; //low threshold value
int zone_flag = 0; //зона по уровню заряда АКБ(1-высокий 0-средний 2-низкий)
int old_zone_flag = 0; //old voltage zone
boolean ac220v_flag = false; //220v ON! indicator
boolean old_ac220v_flag = false; //old 220v ON! indicator

long ble_pcounter = 0; //счетчик времени между обращениями от orange pi
long ble_period = 0; //ble connect period
long pause_counter = 0; //счетчик интервалов ожидания для модуля управления реле

boolean doShutdown =false; //команда - выполнить shutdown
boolean doPowerOn =false; // команда - включить питание
boolean doPause =false; // команда - ожидать

//буквенное обозначение состояния устройства
//"WON" - ожидается включение реле питания (при выполнении условий)
//"WOF" - выполняется процедура shutdown
//"ON"/"OFF" - реле питания "включено"/"выключено"
String dispstatus = "WON";

//Ticker hTicker; //for alarm indicate

void setup() {

    // запускаем один раз !!!, если нужно, стирает NVRAM (#include <nvs_flash.h>)
    //nvs_flash_erase();      // erase the NVS partition and...
    //nvs_flash_init();       // initialize the NVS partition.
    //while (true); //STOP..

    Serial.begin(115200);

  //LED (internal)
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //led = OFF

  //ADC
  pinMode(sens_pin, INPUT); // ADC0 pin input
  pinMode(sens1_pin, INPUT); // ADC1 pin input

  //relay output init
  relay_init();

  //delay(10000);  //10 sec for Platformio start terminal...

  Serial.println();
  Serial.println("----------------Start Info-----------------");
  Serial.printf("Total heap:\t%d \r\n", ESP.getHeapSize());
  Serial.printf("Free heap:\t%d \r\n", ESP.getFreeHeap());
  Serial.println("ADC_PIN= "+String(sens_pin));
  Serial.println("ADC1_PIN= "+String(sens1_pin));

  ble_setup(); //start BLE server
  Serial.println("-----------------------------------------");
 
}

void loop() {

  // read the value from the sensor:
  old_real_voltage=real_voltage; //save old voltage
  sens_value = analogRead(sens_pin);
  sens_voltage =sens_value*adc_calibr/4096; // calculate
  real_voltage = sens_voltage * factor; //real voltage sensor0

  old_real1_voltage=real1_voltage; //save old voltage
  sens1_value = analogRead(sens1_pin);
  sens1_voltage =sens1_value*adc_calibr/4096; // calculate
  real1_voltage = sens1_voltage * factor1; //real voltage sensor1

  //voltage zone..(9.3 - 12.6) with BMS-3S-1
  old_zone_flag = zone_flag;
  if     (real_voltage < alarm_l && old_real_voltage < alarm_l) zone_flag=2; //<10V
  else if(real_voltage > alarm_h && old_real_voltage > alarm_h) zone_flag=1; //>11.5V
  else zone_flag=0;

  //ac_220v handling
  old_ac220v_flag = ac220v_flag;
  if(real1_voltage > AC220V_THRESHOLD && old_real1_voltage > AC220V_THRESHOLD) ac220v_flag=true;
  else ac220v_flag=false;

  ble_pcounter++; pause_counter++;

  // логика обслуживания реле
  relay_control();

  delay(2000); 
}
