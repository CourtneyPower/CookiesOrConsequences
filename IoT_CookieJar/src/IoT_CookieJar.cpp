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
const int BRI = 30;
int gearAngle;
const int PIXELCOUNT = 5;
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Servo myServo;
char hexaKeys [ROWS][COLS] = {
  {'1', '2', 'A', '3'},
  {'4', '5', 'B', '6'},
  {'7', '8', 'C', '9'},
  {'*', '0', 'D', '#'}
};
byte rowPins [ROWS] = {D8, D9, D16, D15};
byte colPins [COLS] = {D17, D18, D19, D14};
Keypad customKeypad = Keypad (makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char secretKey[4] = {'9', '0', '0', '5'}; //unlock code
char attemptKey[4];
int i, x, currentTime;
int triggered = 0;
bool openSesameFunction(char compareSecret[4], char compareAttempt[4]);
bool passwordMatch;
const int OLED_RESET = -1;
Adafruit_SSD1306 display(OLED_RESET);
const int GOODWEMO = 2;
const int BADWEMO = 4;
const int BULB_3 = 3;
const int BULB_5 = 5;
const int MOTION_PIR = D4;
// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
pinMode (MOTION_PIR, INPUT);
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
  pixel.clear();
  pixel.show();
  x=0; //characters in array via keypad
  gearAngle = 90; //locked angle
  myServo.write(gearAngle);
 display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
 display.display();
 delay(2000);
 display.clearDisplay();
 display.display();
 display.setTextSize(2);
display.setTextColor(WHITE);
display.setCursor(0,0);

triggered = LOW;
delay(5000); //allow signal to Wemos
wemoWrite(GOODWEMO, LOW);
wemoWrite(BADWEMO, LOW);
setHue(BULB_3,false,0,0,0);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
currentTime = millis();
// neopixels to always be on as interior lighting
pixel.clear();
for (i=0; i<= PIXELCOUNT; i++) {
  pixel.setPixelColor(i, teal);
  pixel.show();
}

triggered = digitalRead(MOTION_PIR);
Serial.printf("triggered is %i\n", triggered);
if (triggered == HIGH) {
    display.clearDisplay();
    display.printf("Hungry?\n"); //OLED screen
    display.printf("Enter the Code\n");
    display.printf("COOKIES\n");
    display.display();
    setHue(BULB_3,true,HueYellow,100,255);
    triggered = LOW;
    x=0; //reset array to 0 at beginning of each loop to allow for new code attempt
    while (x < 4){
    customKey = customKeypad.getKey();
      if (customKey) {
 // Serial.printf("Key Pressed: %c\n", customKey);
      attemptKey[x] = customKey;
      pixel.clear();
      pixel.setPixelColor(0,purple);
      pixel.setPixelColor(1, purple);
       pixel.setPixelColor(2, purple);
        pixel.setPixelColor(3, purple);
         pixel.setPixelColor(4, purple);
      pixel.show();
       display.clearDisplay();
       display.display();
      setHue(BULB_3,true,HueViolet,255,175);
      x = x+1;
      }
    } //end of filling the guess array, x<4
  } 
  else if (triggered == LOW) {
   setHue(BULB_3, false, 0,0,0);
  }
 
 if (x>=4){
  Serial.printf("AttemptKey is %c, %c, %c, %c\n", attemptKey[0], attemptKey[1], attemptKey[2], attemptKey[3]);
x=0;
passwordMatch = openSesameFunction(secretKey, attemptKey);
display.setCursor(0,0); //reset cursor position to 0,0 for each attempt
display.clearDisplay();
//display.display();
if (passwordMatch){
  pixel.clear();
  pixel.setPixelColor(0, green);
  pixel.setPixelColor(1, green);
    pixel.setPixelColor(2, green);
      pixel.setPixelColor(3, green);
        pixel.setPixelColor(4, green);
  pixel.show();
 // display.setTextSize(1);
 //passwordMatch = !passwordMatch;
  display.printf("Take Only One\n"); //OLED screen
  display.printf("Reenter Code to Lock\n");
  display.display();
wemoWrite(GOODWEMO, HIGH); //aroma on
wemoWrite(BADWEMO, LOW); //alarm off
setHue(BULB_3,true,HueGreen,100,255);
    if (gearAngle == 0) { //if correct code is inputted but lock in unlocked (gear angle = 0) then the system is reset to lock position and OLED shows 'locked'
    gearAngle = 90;
    myServo.write(gearAngle); //write gear to lock position
    display.setCursor(0,0);
    display.clearDisplay();
    //display.display();
    display.printf("LOCKED\n");
  display.display();
  wemoWrite(GOODWEMO, LOW); //aroma off
  setHue(BULB_3,false,0,0,0); //lights off
   }
   else {
  gearAngle = 0;
  myServo.write(gearAngle); //unlock the lock
   }
} //close of password match = true
if (!passwordMatch) {
//else {
  pixel.clear();
    pixel.setPixelColor (0,red);
      pixel.setPixelColor(1, red);
       pixel.setPixelColor(2, red);
        pixel.setPixelColor(3, red);
         pixel.setPixelColor(4, red);
  pixel.show();
  display.printf("Try Again\n"); //OLED screen
display.display();
wemoWrite(BADWEMO, HIGH); //alarm on
wemoWrite(GOODWEMO, LOW); //smell off
setHue(BULB_3,true,HueRed,255,175);

} //close password match = false

} //close x>=4

} //close VOID LOOP

//create a function
bool openSesameFunction(char compareSecret[4], char compareAttempt[4]) {
bool openSesame;
int j = 0;
for (j = 0; j<4; j++) {
  if (compareSecret[j] == compareAttempt[j]) {
  openSesame = true;
  }
  else {
  openSesame = false;
  break;
  }
}
return openSesame;
}