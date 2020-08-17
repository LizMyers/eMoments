
//Sketch by LizMyers@
//Last updated: Aug 16, 2020

//What it does: 
// 1. Scrolls "Gooooooooogle!". Number of "o"s & colors come from eMoments data
// 2. Single Tap toggles btw "render by time" and "render all one color" patterns
// 3. Extend project by using accelerometer data to change patterns
//
//    REQUESTED FEATURE: Return tags: So we can show the related topic.
//    Example: "#gtech" -> "Gooooooooogle!" 
//
// Hardare:
// a) Adafruit Huzzah Feather ESP32 (BT + WiFi)
// b) Adafruit DotStar Feather Wing (12 x 6)
// c) Adarfuit LIS3DH Accelerometer (accepts single/double tap)

//for LIS3DH sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

//for DotStar Matrix
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <Fonts/TomThumb.h>

//for BlueTooth
#include <BLEDevice.h>
#include <BLEServer.h>

#if defined(ESP8266)
  #define DATAPIN    13
  #define CLOCKPIN   14
#elif defined(__AVR_ATmega328P__)
  #define DATAPIN    2
  #define CLOCKPIN   4
#elif defined(ARDUINO_NRF52832_FEATHER)
  #define DATAPIN    7
  #define CLOCKPIN   16
#elif defined(TEENSYDUINO)
  #define DATAPIN    9
  #define CLOCKPIN   5
#elif defined(ARDUINO_ARCH_WICED)
  #define DATAPIN    PA4
  #define CLOCKPIN   PB5
#elif defined(ESP32)
  #define DATAPIN    27
  #define CLOCKPIN   13
#else // // 32u4, M0, M4, nrf52840 and 328p
  #define DATAPIN    11
  #define CLOCKPIN   13
#endif

#define SHIFTDELAY 120
#define BRIGHTNESS 20

// I2C Sensor
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
#define CLICKTHRESHHOLD 80

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
                                  12, 6, DATAPIN, CLOCKPIN,
                                  DS_MATRIX_BOTTOM     + DS_MATRIX_LEFT +
                                  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
                                  DOTSTAR_BGR);

const uint16_t primaryColors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

const uint16_t colorArray[] = {
  matrix.Color(255, 0, 0),   // red
  matrix.Color(200, 200, 0), //orange
  matrix.Color(90, 200, 255), //blue
  matrix.Color(180, 255, 0), //yellowGreen
  matrix.Color(0, 255, 0),   //green
  matrix.Color(180, 0, 255),  //purple, eMoments color
};

int yardstick = -70;//initializing wrap-around distance
String colorString = "";
String msgString = "eMoments!"; //initial msg shows we are up and running
String msgStr1 = "";
String msgStr2 = "";
String msgStr3 = "";
String dataMsg = "== not enough data == ";

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool bluetoothConnected = false;
bool singleTap = false;// render one color
//bool doubleTap = true; //default render by time

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bluetoothConnected = true;
    msgString = "******";
    Serial.println("BT Connecting <<< :-) ");
    delay(20);
  };

  void onDisconnect(BLEServer* pServer) {
    bluetoothConnected = false;
    colorString = "";
    msgString = "<=||=||=||=||=||=";
    Serial.println("BT DISCONNECTED! <<< :-( ");
    delay(20);
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();
      Serial.println("");
      Serial.println("Obtained colors from bluetooth:");
      Serial.println(value);

      Serial.print("ColorStringLength: ");
      Serial.println(value.length());

      colorString = value;
      
     //dotstar width limit: keep max number of "o"s under 20 so we can finish word on screen

     if (colorString.length() > 20) {
          for(int i = 0; i < 20; i++) {
            msgStr2.concat("o");
          }
          msgString = ("G" + msgStr2 +"gle! ");
      } else if (colorString.length() >=7) { // 7 chars = just enough to write "Google!"
          for(int i = 0; i < (colorString.length() - 5); i++) {
            msgStr2.concat("o");
          }
          msgString = ("G" + msgStr2 +"gle! ");
      } else { //we do not have enough data to build the string
        msgString = dataMsg;
      }
    
      //msgString = ("G" + msgStr2 +"gle! ");
      Serial.print("msgString: ");
      Serial.println(msgString); 

      Serial.print("msgStringLength: ");
      Serial.println(msgString.length());
       
      //controls rollover/blank space btw iterations
      yardstick = ((msgString.length() + 62) * -1);
      Serial.print("yardstick: ");
      Serial.println(yardstick); 

      delay(1000);

    }
};



void setup() {
  
  //while (!Serial) delay(500); 
 
  Serial.begin(115200);
  Serial.println("\nDotstar Matrix Wing");

   if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  //Serial.println("LIS3DH found!");

  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  
  //Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  //Serial.println("G");

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(2, CLICKTHRESHHOLD);
  delay(100);

  matrix.begin();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(false);
  matrix.setBrightness(BRIGHTNESS);

  for (byte i = 0; i < 3; i++) {
    matrix.fillScreen(primaryColors[i]);
    matrix.show();
    delay(500);
  }
  
   // init bluetooth stuff
  BLEDevice::init("eMoments-DotStar");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic  *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("DefaultValue");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Bluetooth setup done!");

delay(500);

}

int x = matrix.width();
int pass = 0;

void loop() {

  matrix.fillScreen(0);
  matrix.setCursor(x, 5);
  msgStr2 = "";

     for (int i = 0; i < msgString.length(); i++) { 
        
        char c = '6';

        if(singleTap) { //singleTap = display single color or most recent
            
          if(colorString.length() > 6) { 
            c = colorString.charAt(0);
          } else if (colorString.length() > 0) {
            matrix.setTextColor(colorArray[5]); 
          }
          
          if (c == '0') matrix.setTextColor(colorArray[0]);      // red
          else if (c == '1') matrix.setTextColor(colorArray[1]); // orange
          else if (c == '2') matrix.setTextColor(colorArray[2]); // blue
          else if (c == '3') matrix.setTextColor(colorArray[3]); // yellowGreen
          else if (c == '4') matrix.setTextColor(colorArray[4]); // green
          else matrix.setTextColor(colorArray[5]);               // purple
  
          // write the letter
          matrix.print(msgString[i]);
          
         } else { //no taps
          
          if(colorString.length() > 6) { 
            c = colorString.charAt(i);
          } else if (colorString.length() > 0) {
            matrix.setTextColor(colorArray[5]); 
          }
        
          if (c == '0') matrix.setTextColor(colorArray[0]);      // red
          else if (c == '1') matrix.setTextColor(colorArray[1]); // orange
          else if (c == '2') matrix.setTextColor(colorArray[2]); // blue
          else if (c == '3') matrix.setTextColor(colorArray[3]); // yellowGreen
          else if (c == '4') matrix.setTextColor(colorArray[4]); // green
          else matrix.setTextColor(colorArray[5]);               // purple
  
          // write the letter
          matrix.print(msgString[i]);

        } // end if singleTap
     
      }//end for loop

  
  if (--x < yardstick) {
    x = matrix.width();
  }

  matrix.show();
  delay(SHIFTDELAY);
  
  uint8_t click = lis.getClick();
  if (click == 0) return;
  if (! (click & 0x30)) return;
  Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.println("): ");

  //toggle btw all one color and render by time
  
    if (click && singleTap == 1) {
      singleTap = false;
      return;
    } else {
      singleTap = true;
      return;
    }
  
  delay(1000);
  
}
