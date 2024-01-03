/* 
 * Project IoT Cookie Jar
 * Author: Courtney Power
 * Date: PT_IoT_Midterm
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Keypad_Particle.h"
#include "IoTClassroom_CNM.h"
#include "Colors.h"
#include "neopixel.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

// Let Device OS manage the connection to the Particle Cloud
//SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

const byte ROWS = 4;
const byte COLS = 4;
char customKey;
const int BRI = 10;
int gearAngle;
const int PIXELCOUNT = 1;
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Servo myServo;
char hexaKeys [ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins [ROWS] = {D8, D9, D16, D15};
byte colPins [COLS] = {D17, D18, D19, D14};
Keypad customKeypad = Keypad (makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char secretKey[4] = {'9', '0', '0', '5'};
char attemptKey[4];
int i, x;
bool openSesameFunction(char compareSecret[4], char compareAttempt[4]);
bool passwordMatch;
const int OLED_RESET = -1;
Adafruit_SSD1306 display(OLED_RESET);
const int GOODWEMO = 2;
const int BADWEMO = 4;
// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
Serial.begin(9600);
waitFor(Serial.isConnected, 15000);
WiFi.on();
  WiFi.clearCredentials();
  WiFi.setCredentials("IoTNetwork");
  
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

  myServo.attach(A2);
  pixel.begin();
  pixel.setBrightness(BRI);
  pixel.show();
  x=0;
  gearAngle = 90;
  myServo.write(gearAngle);
 display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
 display.display();
 delay(2000);
 display.clearDisplay();
 display.display();
 display.setTextSize(2);
display.setTextColor(WHITE);
display.setCursor(0,0);
delay(20000);
wemoWrite(GOODWEMO, LOW);
wemoWrite(BADWEMO, LOW);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {

  // The core of your code will likely live here.
//Serial.printf("SecretKey is %c, %c, %c, %c\n", secretKey[0], secretKey[1], secretKey[2], secretKey[3]);
// pixels to always be on
for (i=0; i<= PIXELCOUNT; i++) {
  pixel.setPixelColor(i, teal);
  pixel.show();
}

x=0;
while (x < 4){
  customKey = customKeypad.getKey();
  if (customKey) {
 // Serial.printf("Key Pressed: %c\n", customKey);
// Serial.printf("Key Pressed (Hex Code) 0x%02X\n", customKey);
  attemptKey[x] = customKey;
   pixel.setPixelColor(0, purple);
   pixel.show();
   x = x+1;
 }
}
 if (x>=4){
   Serial.printf("AttemptKey is %c, %c, %c, %c\n", attemptKey[0], attemptKey[1], attemptKey[2], attemptKey[3]);
passwordMatch = openSesameFunction(secretKey, attemptKey);
display.setCursor(0,0);
display.clearDisplay();
display.display();
if (passwordMatch){
  pixel.setPixelColor(0, green);
  pixel.show();
  display.printf("Take Only One\n");
  display.display();
wemoWrite(GOODWEMO, HIGH);
wemoWrite(BADWEMO, LOW);

  if (gearAngle == 0) {
    gearAngle = 90;
    myServo.write(gearAngle);
    display.clearDisplay();
    display.display();
      display.printf("LOCKED\n");
  display.display();
  }
  else {
  gearAngle = 0;
  myServo.write(gearAngle);

}
}

if (!passwordMatch) {
  pixel.setPixelColor(0, red);
  pixel.show();
  display.printf("Try Again\n");
display.display();
wemoWrite(BADWEMO, HIGH);
wemoWrite(GOODWEMO, LOW);
}
}

}
//create a function
bool openSesameFunction(char compareSecret[4], char compareAttempt[4]) {
bool openSesame;
int i = 0;
for (i = 0; i<4; i++) {
if (compareSecret[i] == compareAttempt[i]) {
  openSesame = true;
}
else {
  openSesame = false;
}
}
return openSesame;
}