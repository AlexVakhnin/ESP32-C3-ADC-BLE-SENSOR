#include <Arduino.h>
#include <Ticker.h>
//#include <nvs_flash.h> //для стирания всего NVRAM

//Function Declaration
extern void disp_show();
extern void disp_setup();
extern void ble_setup();
void alarm_blink();
void relay_control();

//Variables
String ds1=""; //display line 1
String ds2=""; //display line 2
const int sens_pin = 0;//A0 for ADC0
const int charg_pin = 20;//battery charging relay pin
//const int supply_pin = 21;//battery supply relay pin

String dev_name = "ADC-SENSOR#2"; //name of BLE service
int sens_value = 0; //pure sensor
float sens_voltage = 0; //Voltage ADC Input
double factor = 2; //calibration attenuator factor
double adc_calibr =3.3; //calibration ADC factor
float real_voltage =0; //measuring voltage
float old_real_voltage=0; //contains the result of the previous measurement
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

  //LED (internal)
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //led = OFF

  //ADC
  pinMode(sens_pin, INPUT); // declare the sens_pin as an INPUT

  //battery charging relay
  pinMode(charg_pin, OUTPUT);
  digitalWrite(charg_pin, HIGH); //relay = OFF

  //battery supply relay
  //pinMode(supply_pin, OUTPUT);
  //digitalWrite(supply_pin, HIGH); //relay = OFF

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
  sens_voltage =sens_value*adc_calibr/4096; // calculate
  real_voltage = sens_voltage * factor; //new real voltage

  // relay control logic processing
  relay_control();

  //alarms
  if     (real_voltage < alarm_l) alarm_flag=2; //11V
  else if(real_voltage > alarm_h) alarm_flag=1; //14.4V
  else alarm_flag=0;

  //Serial.print("sens_value: "+ String(sens_value));Serial.print("  sens_voltage = "+ String(sens_voltage));
  //Serial.print("  factor: "+ String(factor));Serial.println("  real_voltage = "+ String(real_voltage));
  Serial.printf("v=%3f\r\n",real_voltage);

  old_real_voltage=real_voltage; //after processing we can keep it as old
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

// relay control logic processing
void relay_control(){

  //high thresholde event (voltage go from low to high __/-- )
  if(real_voltage >= alarm_h && old_real_voltage < alarm_h){
    digitalWrite(charg_pin, HIGH); //charging = OFF
    Serial.println("....charging = OFF");
    //delay(500); 
    //digitalWrite(supply_pin, LOW); //supplying = ON
  }
  //low thresholde event (voltage go from high to low --\__ )
  else if(real_voltage <= alarm_l && old_real_voltage > alarm_l){
    //digitalWrite(supply_pin, HIGH); //supplying = OFF
    //delay(500);
    digitalWrite(charg_pin, LOW); //charging = ON
    Serial.println("....charging = ON");

  }
  
}