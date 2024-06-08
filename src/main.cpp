#include <Arduino.h>
#include <Ticker.h>

//Function Declaration
extern void ble_setup();
void alarm_blink();

//Variables
const int sens_pin = 0;//A0;
int sens_value = 0; //pure sensor
float sens_voltage = 0;
double factor = 2; //calibration factor
double adc_calibr =3.3;
float real_voltage =0;
float alarm_voltage1 =11;
float alarm_voltage2 =10;
int alarm_flag = 0;

Ticker hTicker; //for alarm

void setup() {
    Serial.begin(115200);

  //internal led
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); //led = OFF
  pinMode(sens_pin, INPUT); // declare the sens_pin as an INPUT

   //инициализация прерывания (5 sec.)
  hTicker.attach_ms(1500, alarm_blink);


  delay(10000);  //10 sec for DEBUG ...

  Serial.println();
  Serial.println("----------------Start Info-----------------");
  Serial.printf("Total heap:\t%d \r\n", ESP.getHeapSize());
  Serial.printf("Free heap:\t%d \r\n", ESP.getFreeHeap());
  Serial.println("ADC_PIN= "+sens_pin);

  ble_setup(); //start BLE server

  Serial.println("-----------------------------------------"); 
}

void loop() {

  // read the value from the sensor:
  sens_value = analogRead(sens_pin);

  sens_voltage =sens_value*adc_calibr/4096;
  real_voltage = sens_voltage * factor;

  if     (real_voltage < alarm_voltage2) alarm_flag=2; //10V
  else if(real_voltage < alarm_voltage1) alarm_flag=1; //11V
  else alarm_flag=0;


  Serial.print("sens_value: "+ String(sens_value));Serial.print("  sens_voltage = "+ String(sens_voltage));
  Serial.print("  factor: "+ String(factor));Serial.println("  real_voltage = "+ String(real_voltage));

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

}