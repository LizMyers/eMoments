
// Sketch by LizMyers@
// Last updated: Aug 16, 2020
//
// HARDWARE
// 1. Adafruit Huzzah Feather Esp32 (BlueTooth & WiFi)
// 2. Adafruit NeoTrellis 4x4 Grid
// 3. RESET Button (Restart script)
// 4. Power Switch
// 5. Lipo rechareable battery
// 6. Case: https://www.thingiverse.com/thing:3750704

// Assembled from parts left over from this project: https://learn.adafruit.com/neotrellis-soundboard/

#include "Adafruit_NeoTrellis.h" // Adafruit NeoTrellis 4x4 Grid
#include <BLEDevice.h>
#include <BLEServer.h>

// ********************* These need to be adapted to your build *******************

// How many leds in your strip?
#define NUM_LEDS 16

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 4

// Define LED type, e.g. NEOPIXEL, WS2811. The LED type can usually be found on the LED chip or is provided by the manufacturer.
#define LED_TYPE NEOPIXEL

// Define Mechanical Switch - LED Button Used
#if defined(__SAMD21G18A__) || defined(__AVR_ATmega32U4__)
  #define SWITCH_PIN   9
#elif defined(__AVR_ATmega328P__)
  #define SWITCH_PIN   9
#elif defined(NRF52)
  #define SWITCH_PIN   31
#elif defined(ESP8266)
  #define SWITCH_PIN   0
#elif defined(TEENSYDUINO)
  #define SWITCH_PIN   4
#elif defined(ESP32)
  #define SWITCH_PIN   15
#endif

// Available LED render patterns, e.g. sorted by creation time.
enum LedRenderPattern {
  RENDER_BY_COLOR = 0,
  RENDER_BY_TIME = 1,
  RENDER_RANDOMLY = 2,
  RENDER_ONE_COLOR = 3,
};
//------------- Set the LED render pattern for this build. -------------------//

// Testing Patterns and Setting Default
//LedRenderPattern PATTERN = RENDER_ONE_COLOR;
LedRenderPattern PATTERN = RENDER_RANDOMLY;
//LedRenderPattern PATTERN = RENDER_BY_TIME;
//LedRenderPattern PATTERN = RENDER_BY_COLOR;

//***************************** END settings *********************************

bool bluetoothConnected = false;
bool sortByColor = false;
bool sortByTime = false;
bool sortRandomly = false;
bool oneColor = false;
bool randomMode = false;

String colorString = "";

//instantiate trellis 
Adafruit_NeoTrellis trellis;

//define a callback for key presses
TrellisCallback blink(keyEvent evt){
  
  // Check is the pad pressed? Change display mode by pushing btns in columns 0 - 3
  if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING) {
    if (evt.bit.NUM == 0 || evt.bit.NUM == 4 || evt.bit.NUM == 8 || evt.bit.NUM == 12) {
        sortByColor = true;
        sortByTime= false;
        oneColor = false;
        sortRandomly = false;
     } else if (evt.bit.NUM == 1 || evt.bit.NUM == 5 || evt.bit.NUM == 9 || evt.bit.NUM == 13) {
        sortByColor = false;
        sortByTime= true;
        oneColor = false;
        sortRandomly = false;
     } else if (evt.bit.NUM == 2 || evt.bit.NUM == 6 || evt.bit.NUM == 10 || evt.bit.NUM == 14) {
        sortByColor = false;
        sortByTime= false;
        oneColor = true;
        sortRandomly = false;
     } else if (evt.bit.NUM == 3 || evt.bit.NUM == 7 || evt.bit.NUM == 11 || evt.bit.NUM == 15) {
        sortByColor = false;
        sortByTime= false;
        oneColor = false;
        sortRandomly = true;
     } 


     //turn button white when pressed to acknowledge user interaction
     trellis.pixels.setPixelColor(evt.bit.NUM, 0xFFFFFF); 
  } else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING) {
  // or is the pad released?
    Serial.println("Felt that button press!");
    //trellis.pixels.setPixelColor(evt.bit.NUM, 0); //off falling
  }
  
      //turn neopixels off btw display modes
      for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
        trellis.pixels.setPixelColor(i,0x000000);
        trellis.pixels.show();
        delay(50);
      }

  // Turn on/off the neopixels!
  trellis.pixels.show();

  return 0;
}

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    bluetoothConnected = true;
    Serial.println("BLUETOOTH CONNECTED <<< :-)");
  };

  void onDisconnect(BLEServer* pServer) {
    bluetoothConnected = false;
    colorString = "";
    Serial.println("BLUE TOOTH DISCONNECTED <-:-( ");
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();
      Serial.println("Obtained colors from bluetooth:");
      Serial.println(value);
      colorString = value;
    }
};


void setup() {

  // initing serial port
  Serial.begin(115200);

  // set up the switch
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  trellis.begin();
  trellis.pixels.setBrightness(90);
  delay(2000);

  //*********NeoTrellis Setup ***********//

  //while(!Serial);
  
  if (!trellis.begin()) {
    Serial.println("Could not start trellis, check wiring?");
    while(1);
  } else {
    Serial.println("NeoPixel Trellis started");
  }

  //activate all keys and set callbacks
  for(int i=0; i<NEO_TRELLIS_NUM_KEYS; i++){
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
    trellis.registerCallback(i, blink);
  }

  //do a little animation to show we're on
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i,0xFFFFFF);
    //trellis.pixels.setPixelColor(i, Wheel(map(i, 0, trellis.pixels.numPixels(), 0, 255)));
    trellis.pixels.show();
    delay(50);
  }
  //turn neopixels off
  for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {
    trellis.pixels.setPixelColor(i,0x000000);
    trellis.pixels.show();
    delay(50);
  }

  // init bluetooth stuff
  BLEDevice::init("eMoments-NeoTrellis");
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
}

void loop() {

  // Listen for a switch event
  if (!digitalRead(SWITCH_PIN)) {
    Serial.println("Switch pressed!");
  }
  
  trellis.read();
  delay(2000);

  showColorsFromString(colorString);
  delay(2000);

}

//*************************** Start LED rendering ********************************

// List of LED render functions. Each function in the list is defined below.
typedef void (*LedRenderPatterns[])();
LedRenderPatterns ledRenderPatterns = { renderByColor, renderByTime, renderRandomly, renderOneColor };

// Shows the colors using the render function configured in PATTERN.
void showColorsFromString(String colorString) {
  Serial.print("Showing colors: ");
  Serial.println(colorString);
  Serial.println("");

// Switch Patterns
  if (colorString.length() > 0) { 
      if (sortByColor) {
        Serial.println("RENDER_BY_COLOR");
        LedRenderPattern PATTERN = RENDER_BY_COLOR;
        ledRenderPatterns[PATTERN]();
        Serial.print("PATTERN = ");
        Serial.println(PATTERN);
      } else if (sortByTime) {
        Serial.println("RENDER_BY_TIME");
        LedRenderPattern PATTERN = RENDER_BY_TIME;
        ledRenderPatterns[PATTERN]();
        Serial.print("PATTERN = ");
        Serial.println(PATTERN);
      } else if(oneColor) {
        Serial.println("RENDER_ONE_COLOR");
        LedRenderPattern PATTERN = RENDER_ONE_COLOR;
        ledRenderPatterns[PATTERN]();
        Serial.print("PATTERN = ");
        Serial.println(PATTERN);
      } else {
        Serial.println("Pattern = RENDER_RANDOMLY");
        LedRenderPattern PATTERN = RENDER_RANDOMLY;
        ledRenderPatterns[PATTERN]();
        Serial.print("PATTERN = ");
        Serial.println(PATTERN);
      }
  }
 
  delay(2000);
  trellis.pixels.show();
}

// Renders the colors by creation time.
void renderByTime()
{
  //Serial.println("Rendering by time created: ");
  // The colorString is already sorted when retrieved from server. We just render it.
  renderColorString();
}

// Renders the colors ordered by color (Red -> Orange -> Blue -> Yellowgreen -> Green)
void renderByColor()
{
  Serial.println("Rendering by color: ");
  char x = '0';
  int green = 0;
  int yellowGreen = 0;
  int blue = 0;
  int orange = 0;
  int red = 0;

  // Count how often each color occurs in the colorString.
  for (int i = 0; i < min(static_cast<int>(colorString.length()), static_cast<int>(NUM_LEDS)); i++) {
    if (colorString[i] == '0') red +=1;
    else if (colorString[i] == '1') orange +=1;
    else if (colorString[i] == '2') blue +=1;
    else if (colorString[i] == '3') yellowGreen +=1;
    else if (colorString[i] == '4') green +=1;
    else blue +=1;
  }

  //Check Values
  Serial.print("red: ");
  Serial.println(red);
  
  Serial.print("orange: ");
  Serial.println(orange);

  Serial.print("blue: ");
  Serial.println(blue);

  Serial.print("yellowGreen: ");
  Serial.println(yellowGreen);

  Serial.print("green: ");
  Serial.println(green);
  
  
  // Render each color
  int ledIndex = 0;
  ledIndex = renderColorRange(ledIndex, red, '0');
  ledIndex = renderColorRange(ledIndex, orange, '1');
  ledIndex = renderColorRange(ledIndex, blue, '2');
  ledIndex = renderColorRange(ledIndex, yellowGreen, '3');
  ledIndex = renderColorRange(ledIndex, green, '4');
}

// Renders the colors in random order.
void renderRandomly()
{
  //Serial.println("Render randomly: ");
  shuffleColorString();
  renderColorString();
}

// Render the first color on all leds
void renderOneColor() {
  char c = '6';
  if(colorString.length() > 0) c = colorString.charAt(0);
    for (uint16_t i=0; i<trellis.pixels.numPixels(); i++) {

  for (int i = 0; i < NUM_LEDS; i++) {
    if (c == '0') trellis.pixels.setPixelColor(i,0xCC0000);        // red
      else if (c == '1') trellis.pixels.setPixelColor(i,0xB34700); // orange
      else if (c == '2') trellis.pixels.setPixelColor(i,0x002DFF); // blue
      else if (c == '3') trellis.pixels.setPixelColor(i,0xCCE100); // yellowGreen
      else if (c == '4') trellis.pixels.setPixelColor(i,0x00CC00); // green
      else trellis.pixels.setPixelColor(i,0x002DFF);               // blue
  }

 }
   
}

// Shuffles the order of the colors in the colorString.
void shuffleColorString()
{
  int n = colorString.length();
  if (n > 1)
  {
    int i;
    for (i = 0; i < n - 1; i++)
    {
      int j = i + rand() / (RAND_MAX / (n - i) + 1);
      char t = (colorString)[j];
      (colorString)[j] = (colorString)[i];
      (colorString)[i] = t;
    }
  }
}

// Writes a color block into the LED array.

int renderColorRange(int index, int count, char x) {
  for (int i = 0; i < count; i++, index++) {
    if (x == '0') trellis.pixels.setPixelColor(i,0xCC0000);            // red
      else if (x == '1') trellis.pixels.setPixelColor(index,0xB34700); // orange
      else if (x == '2') trellis.pixels.setPixelColor(index,0x002DFF); // blue
      else if (x == '3') trellis.pixels.setPixelColor(index,0xCCE100); // yellowGreen
      else if (x == '4') trellis.pixels.setPixelColor(index,0x00CC00); // green
  }
  return index;
}

// Writes the colors into the LED array.
void renderColorString() {
  for (int i = 0; i < min(static_cast<int>(colorString.length()), static_cast<int>(NUM_LEDS)); i++) {
    char c = '6';
    if(i < colorString.length()) c = colorString.charAt(i);
    //Serial.println(c);
    if (c == '0') trellis.pixels.setPixelColor(i,0xCC0000);
      else if (c == '1') trellis.pixels.setPixelColor(i,0xB34700);
      else if (c == '2') trellis.pixels.setPixelColor(i,0x002DFF);
      else if (c == '3') trellis.pixels.setPixelColor(i,0xCCE100);
      else if (c == '4') trellis.pixels.setPixelColor(i,0x00CC00);
      else trellis.pixels.setPixelColor(i,0x002DFF);
  }
}


//******************************** End LED rendering ********************************
