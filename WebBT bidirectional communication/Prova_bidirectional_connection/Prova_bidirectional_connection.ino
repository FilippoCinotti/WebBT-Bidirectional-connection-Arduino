
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h> //Library to use BLE as server
#include <BLE2902.h> 
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

bool _BLEClientConnected = false;
int i=0;


#define ForceMeasurement BLEUUID("783b26f8-740d-4187-9603-82281d6d7e4f") 
BLECharacteristic FICharacteristic(BLEUUID("1bfd9f18-ae1f-4bba-9fe9-0df611340195"), BLECharacteristic::PROPERTY_READ  | BLECharacteristic::PROPERTY_WRITE  | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor FIDescriptor(BLEUUID("2f562183-0ca1-46be-abd6-48d0be28f83d"));

//Control if the BT is connected
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      _BLEClientConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      _BLEClientConnected = false;
    }
};


/* ###############################################################  CALL back to receive data from Phone */

class MyCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);
 
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
 
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }
 
    }
};
/* ############################################################### */

void InitBLE() {
//Set BT name
  BLEDevice::init("E-glove");
// Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

// Create the BLE Service
  BLEService *pBattery = pServer->createService(ForceMeasurement);

//Create and add the characteristic to pBattery service
  pBattery->addCharacteristic(&FICharacteristic);
//Attach descriptor
  FIDescriptor.setValue("Saluto");
  FICharacteristic.addDescriptor(&FIDescriptor);
  FICharacteristic.addDescriptor(new BLE2902());

  /* ###############################################################  define callback */
//Define the command for Phone-to-Arduino communication handling
  FICharacteristic.setCallbacks(new MyCallbacks());
  /* ############################################################### */

//Add UUID to the custom service
  pServer->getAdvertising()->addServiceUUID(ForceMeasurement);
//Start service
  pBattery->start();

// Start advertising
  pServer->getAdvertising()->start();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Force measured values - BLE");
//Launch BT initialization  
  InitBLE();
}


  
void loop() {


if (_BLEClientConnected){
// Generate message structure
  char invio[20];
  i++;
//Format data in the string  
  sprintf( invio, "Buondi Rune, n %d", i );
//Send value  
  FICharacteristic.setValue(invio);
//Send notification  
  FICharacteristic.notify();
//Print data we have sent for debugging 
  Serial.println(invio);
//Wait a second  
  delay(1000);
}


}
