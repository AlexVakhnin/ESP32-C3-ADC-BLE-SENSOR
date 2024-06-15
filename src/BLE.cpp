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
void storage_alarm_h(String su);
void storage_alarm_l(String su);
void storage_dev_name(String dname);
void help_print();
void reset_nvram();
//Global Variables
extern String ds1;
extern String ds2;
extern String dev_name;
extern int sens_value;
extern float sens_voltage;
extern float real_voltage;
extern double factor; //(nvram)
extern double adc_calibr; //(nvram)
extern float alarm_h; //(nvram)
extern float alarm_l; //(nvram)


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
    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
      //формируем адрес клиента
      char remoteAddress[18];
      sprintf( remoteAddress , "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
      param->connect.remote_bda[0],
      param->connect.remote_bda[1],
      param->connect.remote_bda[2],
      param->connect.remote_bda[3],
      param->connect.remote_bda[4],
      param->connect.remote_bda[5]
      );
      //фильтруем mac адреса..
      //if (param->connect.remote_bda[0]==0x34){
      //    pServer->disconnect(param->connect.conn_id) ;//force disconnect client..
      //}
      Serial.print("Event-Connect..");Serial.println(remoteAddress);
      ds1="R:";ds2="S:"; //вывод на дисплей
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Event-Disconnect..");
      ds1="BLE: WAIT";ds2="CONNECT"; //вывод на дисплей
      delay(300); // give the bluetooth stack the chance to get things ready
      BLEDevice::startAdvertising();  // restart advertising
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
            ds1="R:"+pstr; //вывод на дисплей
            
            // Do stuff based on the command received from the app
            if (pstr=="at"||pstr=="at\r\n") {     //at
                ble_handle_tx("OK"); //sensor number
                ds2="S:OK"; //вывод на дисплей
            }
            else if (pstr=="at?"||pstr=="at?\r\n") { //at? - help
                help_print();
            }            
            else if (pstr=="atz"||pstr=="atz\r\n") { //atz - reset NVRAM
                reset_nvram();
            }            
            else if (pstr=="ati"||pstr=="ati\r\n") { //ati - information
              String s ="name="+dev_name+"\r\nalarm_h="+String(alarm_h)+"\r\nalarm_l="+String(alarm_l)+  
                  +"\r\nsens_pure="+String(sens_value)+"\r\nadc_calibr="+String(adc_calibr)
                  +"\r\nsens_voltage="+String(sens_voltage)                 
                  +"\r\natt_factor="+String(factor)+"\r\nreal_voltage="+String(real_voltage);
                ble_handle_tx(s); //information for debug
            }
            else if (pstr=="atv"||pstr=="atv\r\n") { //atv - result voltage
                ble_handle_tx(String(real_voltage,3)); //ответ c учетом калибровки
                ds2="S:"+String(real_voltage,3); //вывод на дисплей
            }
            else if (pstr.substring(0,4)=="atu=") {  //atu= - attenuator calibration
                storage_factor_u(pstr.substring(4));
            }
            else if (pstr.substring(0,4)=="ata=") { //ata= - ADC calibration
                storage_adc_u(pstr.substring(4));
            }
            else if (pstr.substring(0,4)=="ath=") { //ath= - alarm_h save NVRAM
                storage_alarm_h(pstr.substring(4)); //alarm_h
            }
            else if (pstr.substring(0,4)=="atl=") { //atl= - alarm_l save NVRAM
                storage_alarm_l(pstr.substring(4)); //alarm_l
            }
            else if (pstr.substring(0,4)=="atn=") { //atn= - dev_name save NVRAM
                storage_dev_name(pstr.substring(4)); //dev_name
            }
            else {ble_handle_tx("???");ds2="S:???";}
            
        }
    }

    //FOR DEBUG
    /*
    void onRead(BLECharacteristic* pCharacteristic, esp_ble_gatts_cb_param_t* param) { //Read
        Serial.println("Event-Read..");
    }
    void onNotify(BLECharacteristic* pCharacteristic) { //Notify
        Serial.println("Event-Notify..");
    }
    void onStatus(BLECharacteristic* pCharacteristic, Status s, uint32_t code) { //Status
        Serial.println("Event-Status..");
    }
    */
   
 };

//Init BLE Service
void ble_setup(){

  //читаем все параметры NVRAM
  preferences.begin("hiveMon", true); //открываем пространство имен NVRAM read only
  factor = preferences.getDouble("att_factor", 5.0);
  adc_calibr = preferences.getDouble("adc_calibr", 3.01);//default adc_calibr=3.01 Volt !!!
  alarm_h = preferences.getFloat("alarm_h", 14.4);
  alarm_l = preferences.getFloat("alarm_l", 11.0);
  dev_name = preferences.getString("dev_name", "ADC-SENSOR#0");
  preferences.end(); //закрываем NVRAM

  // Create the BLE Device
  //BLEDevice::init("ADC-SENSOR#1");
  BLEDevice::init(dev_name.c_str());

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); //Обработчик событий connect/disconnect от BLE

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  // все свойства характеристики (READ,WRITE..)- относительно клиента !!!
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ      |
                      BLECharacteristic::PROPERTY_WRITE     |
                      BLECharacteristic::PROPERTY_NOTIFY    |
                      BLECharacteristic::PROPERTY_INDICATE  |
                      BLECharacteristic::PROPERTY_WRITE_NR  
                    );

  // Create a BLE Descriptor !!!
  pCharacteristic->addDescriptor(new BLE2902()); //без дескриптора не подключается..!!!

  pCharacteristic->setCallbacks(new MyCallbacks()); //обработчик событий read/write

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); 
  pAdvertising->addServiceUUID(SERVICE_UUID);  //рекламируем свой SERVICE_UUID(0x07)
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);//Conn.Intrval(0x12) is:0x06 [5 0x12 0x06004000] iOS..
  //pAdvertising->setMinPreferred(0x12);//Conn.Intrval(0x12) is:0x12 [5 0x12 0x12004000] Default
  //pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();  
  
  Serial.print("BLE Server address: ");
  Serial.println(BLEDevice::getAddress().toString().c_str());
  Serial.println("BLE Device name: "+ dev_name);
}

//ответ клиенту
void ble_handle_tx(String str){

    if (deviceConnected) { //проверка, что подключен клиент BLE
        if(str.length()==0) str="none..\r\n";

        str = str+"\r\n";
        //Serial.println("str="+str);
        pCharacteristic->setValue(str.c_str());
        //pCharacteristic->indicate();//для работы с BLE терминалом !!!!!!!
        pCharacteristic->notify(false); //false=indicate; true=wait confirmation

        Serial.print("Send to BLE client: "+str);
    }

}

//калибровка делителя через U (вводим напряжение)
void storage_factor_u(String su){
  float test_volt=su.toFloat(); //округляет до 2-х знаков после дес. точки...???
  factor = test_volt/sens_voltage;
  Serial.println("new factor="+String(factor));
  ble_handle_tx("new factor="+String(factor)); //ответ на BLE

  preferences.begin("hiveMon", false);
  preferences.putDouble("att_factor", factor);
  preferences.end();
}

//калибровка ADC через U (вводим напряжение)
void storage_adc_u(String su){
  float test_volt=su.toFloat();
  adc_calibr = test_volt*4096/sens_value;
  Serial.println("new adc_calibr="+String(adc_calibr));
  ble_handle_tx("new adc_calibr="+String(adc_calibr)); //ответ на BLE

  preferences.begin("hiveMon", false);
  preferences.putDouble("adc_calibr", adc_calibr);
  preferences.end();
}
//
void storage_alarm_h(String su){
  alarm_h = su.toFloat();
  ble_handle_tx("new alarm_h="+String(alarm_h)); //ответ на BLE
  preferences.begin("hiveMon", false);
  preferences.putFloat("alarm_h", alarm_h);
  preferences.end();
}
void storage_alarm_l(String su){
  alarm_l = su.toFloat();
  ble_handle_tx("new alarm_l="+String(alarm_l)); //ответ на BLE
  preferences.begin("hiveMon", false);
  preferences.putFloat("alarm_l", alarm_l);
  preferences.end();
}
void storage_dev_name(String dname){
  //Нужно удалить \r\n в конце строки..
  int n = dname.length(); 
  dev_name = dname.substring(0,n-2);
  ble_handle_tx("new dev_name="+dev_name); //ответ на BLE
  preferences.begin("hiveMon", false);
  preferences.putString("dev_name", dev_name);
  preferences.end();
  delay(3000);
  ESP.restart();
}

void help_print(){
  String  shelp="ati -parameter list";
          shelp+="\r\natv -resulting voltage";
          shelp+="\r\natz -set default parameters";
          shelp+="\r\nata=[U_ADC_in] -ADC calibration";
          shelp+="\r\natu=[U_in] -attenuator calibration";
          shelp+="\r\nath=[U] -alarm H Voltage";
          shelp+="\r\natl=[U] -alarm L Voltage";
          shelp+="\r\natn=[name] -BLE device name";
  ble_handle_tx(shelp);
}
void reset_nvram(){
  preferences.begin("hiveMon", false); //пространство имен (чтение-запись)
  preferences.clear(); //удаляем все ключи в пространстве имен
  preferences.end();
  ble_handle_tx("NVRAM Key Reset.."); //ответ на BLE
  delay(3000);
  ESP.restart();
}