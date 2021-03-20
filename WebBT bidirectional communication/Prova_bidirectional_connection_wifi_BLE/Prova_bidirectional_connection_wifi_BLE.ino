
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h> 
#include <BLE2902.h> 
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
  #include <AsyncTCP.h>


AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "ORBI21";
const char* password = "marofiri19";

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

bool _BLEClientConnected = false;
int i=0, k=0;
char inviowifi[20];

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<style>
 .cardt{
    max-width: 350px;
     min-height: 250px;
     background: #02b875;
     padding: 30px;
     box-sizing: border-box;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  
   <div class="cardt">
  <h3>Thumb force:        </h3>
  <h1><span id="Thumb">%THUMBFORCE%</span> N</h1>
  </div>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input3: <input type="text" name="input3">
    <input type="submit" value="Submit">
  </form>
</body>
<script>


setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Thumb").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "/thumb", true);
  xhttp.send();
}, 50 ) ;
</script></html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "THUMBFORCE"){
    return String(inviowifi);
  }
  return String();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}





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
      //Serial.println(rxValue[0]);
 
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
//Setup Wifi functions
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  // Send messages
    server.on("/thumb", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", inviowifi);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    // Send web page with input fields to client
    request->send_P(200, "text/html", index_html, processor);
  
  });
  server.onNotFound(notFound);
  server.begin();
  
}


void loop() {
// Generate message structure
  
  k++;
//Format data in the string  
  sprintf( inviowifi, "Buondi Rune, n %d", k );

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

delay(1000);

}
