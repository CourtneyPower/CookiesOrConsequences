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
#include "IoTTimer.h"

// Let Device OS manage the connection to the Particle Cloud
//SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

const byte ROWS = 4;
const byte COLS = 4;
char customKey;
const int BRI = 80;
int gearAngle;
const int PIXELCOUNT = 5;
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Servo myServo;
char hexaKeys [ROWS][COLS] = { //had to switch 3rd and 4th row due to wiring issue
  {'1', '2', 'A', '3'},
  {'4', '5', 'B', '6'},
  {'7', '8', 'C', '9'},
  {'*', '0', 'D', '#'}
};
byte rowPins [ROWS] = {D8, D9, D16, D15};
byte colPins [COLS] = {D17, D18, D19, D14};
Keypad customKeypad = Keypad (makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char secretKey[4] = {'9', '0', '0', '5'}; //unlock code
char attemptKey[4]; //array to store code guess
int i, x, currentTime, triggered;
bool openSesameFunction(char compareSecret[4], char compareAttempt[4]);
bool passwordMatch;
const int OLED_RESET = -1;
Adafruit_SSD1306 display(OLED_RESET);
const int GOODWEMO = 2; //smell
const int BADWEMO = 4; //alarm
const int BULB_3 = 3;
const int MOTION_PIR = D4;
IoTTimer ambianceTimer; 
IoTTimer attemptTimer;
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
  pixel.show(); //start with pixels empty
  x=0; //character position in array via keypad
  gearAngle = 90; //locked angle
  myServo.write(gearAngle); //move lock to locked position
 display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
 display.display();
 delay(2000);
 display.clearDisplay();
 display.display();
 display.setTextSize(2);
display.setTextColor(WHITE);
display.setCursor(0,0);
triggered = LOW; //start with no movement
delay(5000); //allow signal to Wemos
wemoWrite(GOODWEMO, LOW);
wemoWrite(BADWEMO, LOW);
setHue(BULB_3,false,0,0,0);

ambianceTimer.startTimer(1); //immediately allow ambiance lighting to start once in LOOP
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
currentTime = millis();
// neopixels to always be on as interior lighting
  if(ambianceTimer.isTimerReady()) {
    setHue(BULB_3,false,0,0,0); //Hue light is off until someone approaches
    pixel.clear();
    for (i=0; i<= PIXELCOUNT; i++) {
    pixel.setPixelColor(i, teal);
    pixel.show();
    }
  } //close isTimerReady - Ambiance

triggered = digitalRead(MOTION_PIR);
//Serial.printf("triggered is %i\n", triggered); //useful in testing to see if PIR is working
if (triggered == HIGH) { //motion detected
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Hungry?\n"); //OLED screen
    display.printf("Enter the Code\n");
    display.printf("GO SLOW\n");
    display.display(); //show messages on OLED screen
          pixel.clear();
      pixel.setPixelColor(0,yellow);
      pixel.setPixelColor(1, yellow);
       pixel.setPixelColor(2, yellow);
        pixel.setPixelColor(3, yellow);
         pixel.setPixelColor(4, yellow);
      pixel.show(); //brute force pixel coloration, next try 'for loop'
    setHue(BULB_3,true,HueYellow,100,255); // HUE light YELLOW - caution someone is near
    triggered = LOW; //assign triggered back to 0 so it can look for motion again
    x=0; //reset array to 0 at beginning of each loop to allow for new code attempt
    attemptTimer.startTimer(20000); 
    while ((x < 4) && !attemptTimer.isTimerReady()) { //is no entries are made within 20 seconds of motion being detected reverts to ambiance state
    customKey = customKeypad.getKey();
      if (customKey) { //if keys have been pressed
      attemptKey[x] = customKey; //button press is stored in attemptKey array
      pixel.clear();
      pixel.setPixelColor(0,purple);
      pixel.setPixelColor(1, purple);
       pixel.setPixelColor(2, purple);
        pixel.setPixelColor(3, purple);
         pixel.setPixelColor(4, purple);
      pixel.show();
       display.clearDisplay();
       display.display();
      setHue(BULB_3,true,HueViolet,255,175); //Hue lights and pixels are purple
      x = x+1; //increment x to fill the array
      }
    } //end of filling the guess array, x<4
  } 
  // else if (triggered == LOW) { 
  //  setHue(BULB_3, false, 0,0,0);
  // }
 
 if (x>=4){ //attemptKey has been filled positions 0, 1, 2, 3
  //Serial.printf("AttemptKey is %c, %c, %c, %c\n", attemptKey[0], attemptKey[1], attemptKey[2], attemptKey[3]);
x=0;
passwordMatch = openSesameFunction(secretKey, attemptKey); //array comparison function
display.clearDisplay();
display.setCursor(0,0); //reset cursor position to 0,0 for each attempt
//display.display();
if (passwordMatch){
    ambianceTimer.startTimer(5000); //if no movement green will stay on for 5 seconds before going back to ambiance state
  pixel.clear();
  pixel.setPixelColor(0, green);
  pixel.setPixelColor(1, green);
    pixel.setPixelColor(2, green);
      pixel.setPixelColor(3, green);
        pixel.setPixelColor(4, green);
  pixel.show();
  display.printf("Take Only One\n"); //OLED screen
  display.printf("Reenter Code to Lock\n");
  display.display();
wemoWrite(GOODWEMO, HIGH); //aroma on
wemoWrite(BADWEMO, LOW); //alarm off
setHue(BULB_3,true,HueGreen,100,255);//room lights are green

    if (gearAngle == 0) { //if correct code is inputted but lock in unlocked (gear angle = 0) then the system is reset to lock position and OLED shows 'locked'
    gearAngle = 90;
    myServo.write(gearAngle); //write gear to lock position
    display.clearDisplay();
        display.setCursor(0,0);
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
ambianceTimer.startTimer(5000); //if no movement, red lights stay on 5 seconds
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
setHue(BULB_3,true,HueRed,255,175); //room lights are red
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
  openSesame = false; //is any of the digits don't match, the function stops and will return false
  break;
  }
}
return openSesame;
}