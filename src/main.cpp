#include <Arduino.h>
#include <Ticker.h>
//#include <nvs_flash.h> //для стирания всего NVRAM

//Function Declaration
extern void disp_show();
extern void disp_setup();
extern void ble_setup();
void alarm_blink();

//Variables
String ds1="";
String ds2="";
const int sens_pin = 0;//A0;
String dev_name = "ADC-SENSOR#2"; //name of BLE service
int sens_value = 0; //pure sensor
float sens_voltage = 0; //Voltage ADC Input
double factor = 2; //calibration attenuator factor
double adc_calibr =3.3; //calibration ADC factor
float real_voltage =0; //measuring voltage
float alarm_h =11; //led alarm threshold value 1
float alarm_l =10; //led alarm threshold value 2
int alarm_flag = 0;

Ticker hTicker; //for alarm

void setup() {

    // запускаем один раз !!!, если нужно, стирает NVRAM (#include <nvs_flash.h>)
    //nvs_flash_erase();      // erase the NVS partition and...
    //nvs_flash_init();       // initialize the NVS partition.
    //while (true); //STOP..

    Serial.begin(115200);

  //internal led
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //led = OFF
  pinMode(sens_pin, INPUT); // declare the sens_pin as an INPUT

  disp_setup();
   //инициализация прерывания (1.5 sec.)
  hTicker.attach_ms(1500, alarm_blink);


  //delay(10000);  //10 sec for Platformio start terminal...

  Serial.println();
  Serial.println("----------------Start Info-----------------");
  Serial.printf("Total heap:\t%d \r\n", ESP.getHeapSize());
  Serial.printf("Free heap:\t%d \r\n", ESP.getFreeHeap());
  Serial.println("ADC_PIN= "+String(sens_pin));


  ble_setup(); //start BLE server
  ds1="BLE: WAIT";ds2="CONNECT";
  Serial.println("-----------------------------------------"); 
}

void loop() {

  // read the value from the sensor:
  sens_value = analogRead(sens_pin);
  // calculate
  sens_voltage =sens_value*adc_calibr/4096;
  real_voltage = sens_voltage * factor;

  if     (real_voltage < alarm_l) alarm_flag=2; //11V
  else if(real_voltage > alarm_h) alarm_flag=1; //14.4V
  else alarm_flag=0;


  //Serial.print("sens_value: "+ String(sens_value));Serial.print("  sens_voltage = "+ String(sens_voltage));
  //Serial.print("  factor: "+ String(factor));Serial.println("  real_voltage = "+ String(real_voltage));
  Serial.printf("v=%3f\r\n",real_voltage);

  delay(2000); 
}
void alarm_blink(){
  if(alarm_flag==1){
      digitalWrite(8, LOW);
      delay(120);
      digitalWrite(8, HIGH);
  }
  else if (alarm_flag==2){
      digitalWrite(8, LOW);
      delay(120);
      digitalWrite(8, HIGH);
      delay(120);
      digitalWrite(8, LOW);
      delay(120);
      digitalWrite(8, HIGH);
  }
disp_show();
}