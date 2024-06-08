#include <Arduino.h>

//Function Declaration
extern void ble_setup();

const int sens_pin = 0;//A0;
int sens_value = 0; //pure sensor
float sens_voltage = 0;
float factor = 2; //calibration factor
float real_voltage =0;

void setup() {
    Serial.begin(115200);

  //internal led
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(sens_pin, INPUT); // declare the sens_pin as an INPUT


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

  sens_voltage =sens_value/(4096/3.3);
  real_voltage = sens_voltage * factor;

  Serial.print("sens_value: "+ String(sens_value));Serial.print("  sens_voltage = "+ String(sens_voltage));
  Serial.print("  factor: "+ String(factor));Serial.println("  real_voltage = "+ String(real_voltage));

  delay(2000); 
}
