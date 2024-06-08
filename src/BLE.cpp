#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>

//Declaration
//void storage_factor(String sfact);
void storage_factor_u(String su);
void storage_adc_u(String su);
extern int sens_value;
extern float sens_voltage;
extern double factor;
extern double adc_calibr;
extern float real_voltage;


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
uint32_t value = 0;

Preferences preferences; //for NVRAM



// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "450475bb-56a2-4c97-9973-301831e65a30"
#define CHARACTERISTIC_UUID "d8182a40-7316-4cbf-9c6e-be507a76d775"

//Declaration
void ble_handle_tx(String str );

//ловим события connect/disconnect от BLE сервера
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Event-Connect..");
      //digitalWrite(8, LOW);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Event-Disconnect..");

      delay(200); // give the bluetooth stack the chance to get things ready
      BLEDevice::startAdvertising();  // restart advertising
      //digitalWrite(8, HIGH);
    }
};

//ловим события от BLE сервиса read/write
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) { //запись в сервис..
        std::string rxValue = pCharacteristic->getValue(); //строка от BLE

        if (rxValue.length() > 0) {
            //печатаем строку от BLE
            String pstr = String(rxValue.c_str());
            Serial.print("BLE received Value: ");Serial.print(pstr);
            
            // Do stuff based on the command received from the app
            if (pstr=="at\r\n") { 
                ble_handle_tx("OK"); //sensor number
            }            
            else if (pstr=="atn\r\n") { 
                ble_handle_tx("ADC-SENSOR#1"); //sensor number
            }            
            else if (pstr=="ati\r\n") { //ati - information
              String s ="sens_pure="+String(sens_value)+"\r\nsens_voltage="+String(sens_voltage)
                  +"\r\nfactor="+String(factor)+"\r\nreal_voltage="+String(real_voltage);
                ble_handle_tx(s); //information for debug
            }
            else if (pstr=="atv\r\n") { //atv - result voltage
                ble_handle_tx(String(real_voltage)); //ответ c учетом калибровки
            }
            //else if (pstr.substring(0,4)=="atf=") { 
            //    storage_factor(pstr.substring(4)); //сохранить коэфициент
            //}
            else if (pstr.substring(0,4)=="atu=") {  //atu= - attenuator calibration
                storage_factor_u(pstr.substring(4)); //калибровка делителя
            }
            else if (pstr.substring(0,4)=="ata=") { //ata= - ADC calibration
                storage_adc_u(pstr.substring(4)); //калибровка ADC
            }
            else ble_handle_tx("???");
            
        }
    }

    void onRead(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) { //чтение
        Serial.println("Event-Read..");
    }

 };


void ble_setup(){

  // Create the BLE Device
  BLEDevice::init("ADC-SENSOR#1");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); //Обработчик событий connect/disconnect от BLE

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                    /*  BLECharacteristic::PROPERTY_READ   |*/
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                    /*  BLECharacteristic::PROPERTY_BROADCAST|*/
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create a BLE Descriptor !!!
  pCharacteristic->addDescriptor(new BLE2902()); //без дискритора не подключается..

  pCharacteristic->setCallbacks(new MyCallbacks()); //обработчик событий read/write

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();  
  

  Serial.print("BLE Server address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());

  preferences.begin("hiveMon", false); //открываем NVRAM
//  factor = preferences.getFloat("myCal_factor", 0.0); //читаем factor 
  factor = preferences.getDouble("myCal_factor", 0.0); //читаем factor 
  adc_calibr = preferences.getDouble("adc_calibr", 3.3); //читаем NVRAM
  preferences.end(); //закрываем NVRAM
}

//ответ клиенту
void ble_handle_tx(String str){

    if (deviceConnected) { //проверка, что подключен клиент BLE
        if(str.length()==0) str="none..\r\n";

        str = str+"\r\n";
        //Serial.println("str="+str);
        pCharacteristic->setValue(str.c_str());
        pCharacteristic->indicate();//для работы с BLE терминалом !!!!!!!

        Serial.print("Send to BLE client: "+str);
    }

}
/*
//сохраняем factor (вводим коэффициент)
void storage_factor(String sfact){
factor=sfact.toDouble(); //String to Double
Serial.println("new factor="+String(factor));
ble_handle_tx("new factor="+String(factor)); //ответ на BLE
preferences.begin("hiveMon", false);
preferences.putDouble("myCal_factor", factor);
preferences.end();
}
*/
//калибровка делителя через U (вводим напряжение)
void storage_factor_u(String su){
float test_volt=su.toFloat(); //округляет до 2-х знаков после дес. точки...???
factor = test_volt/sens_voltage;
Serial.println("new factor="+String(factor));
ble_handle_tx("new factor="+String(factor)); //ответ на BLE
preferences.begin("hiveMon", false);
preferences.putDouble("myCal_factor", factor);
preferences.end();
}

//калибровка ADC через U (вводим напряжение)
void storage_adc_u(String su){
float test_volt=su.toFloat(); //округляет до 2-х знаков после дес. точки...???
adc_calibr = test_volt*4096/sens_value;
Serial.println("new adc_calibr="+String(adc_calibr));
ble_handle_tx("new adc_calibr="+String(adc_calibr)); //ответ на BLE
preferences.begin("hiveMon", false);
preferences.putDouble("adc_calibr", adc_calibr);
preferences.end();
}