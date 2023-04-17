#include <InvertedTM1638.h>
#include <TM1638.h>
#include <TM1638QYF.h>
#include <TM1640.h>
#include <TM16XX.h>
#include <TM16XXFonts.h>
#include <Streaming.h>
#include <Wire.h>
#include <DS3231.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// -- EXTENSION BOARD -------
#define TM1638_STB D4
#define TM1638_CLK D5
#define TM1638_DIO D6
// -- OLED i2c -------
#define OLED_RESET -1
#define OLED_SCREEN_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(OLED_RESET);

TM1638 module(TM1638_DIO, TM1638_CLK, TM1638_STB);
Servo myservo;  // create servo object to control a servo
DS3231 rtc;
RifTime t;

//Variable declarations
byte saveCount; //Variable which is used to track the amount of saves that have been used.
byte buttons; //Variable which is used save the value of the button presses on the extension board.
//byte value; NOT USED
byte change0, change1, change2, change3; //Variables which are used for the on screen number updates.
byte presses0, presses1, presses2, presses3; //Variables which are used for the extension board number updates.
byte pin0, pin1, pin2, pin3; //Variables which are used for saving the PIN which will be used in the PIN menu.
bool inMain, inGenerator, inGenerate, inPin, inSaved, inEaster; //Variables which are used for knowing in which menu the user is.
bool generated; //Variable which is used to know if a password has be generated.
bool lock; //Variable which when set to true locks the device.
bool clearDigits; //Variable which when set to true clears the digits on the extension board display.
bool slot0, slot1, slot2, slot3; //Variables which are used to know which save slot is free.
bool option0, option1, option2, option3; //Variables which are used for selecting saves in the saved menu using the potentiometer.
bool serialLog; //Used for running information to the serial once
const char difficult[] = "0123456789!@#$%^&*abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; //Variable containing all the letters and symbols used for password generation.
byte seconds; //Variable which is used for storing the RTC seconds.
String generatedChar, generatedFull; //Variables which are used for the password generation.
String saved0, saved1, saved2, saved3; //Variables which are used as save slots.
int pos; //Variable which is equal to the value from the potentiometer after it has been re-mapped to a range of 0-180
int sensorValue; //Variable which is equal to the raw value from the potentiometer
int LED0 = D7; //Variable which is equal to the D7 pin on the board
int LED1 = D8; //Variable which is equal to the D8 pin on the board

// 'logo', 128x64px
const unsigned char logo [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x3e, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x01, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x01, 0xf8, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x03, 0xf8, 0x60, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xf8, 0x60, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xf8, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x07, 0xff, 0xe0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x3f, 0xdc, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x7f, 0x80, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0xff, 0x80, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xfc, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x03, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xdb, 0xff, 0xff, 0xff, 0xf2, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xda, 0x64, 0x75, 0x10, 0x77, 0x88, 0x88, 0x24, 0xcf, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xc7, 0x8d, 0xc4, 0xd5, 0xf4, 0xaa, 0xab, 0xab, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xde, 0x36, 0x4a, 0xd7, 0xf6, 0x8b, 0x8a, 0x2b, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xdc, 0x04, 0x6b, 0x14, 0x78, 0x8b, 0x8a, 0x24, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


void setup() {
  //Start serial
  Serial.begin(115200);
  Serial.print("\n");
  //Set up the 7-segment display on the expansion board
  module.clearDisplay(); //Clears the 7-segment display
  module.setupDisplay(true, 1); //Sets the 7-segment display on at intensity of 1
  
  // -- OLED --------------
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS); //Initialises display
  display.drawBitmap(0, 0, logo, 128, 64, WHITE);
  display.display(); //Displays
  delay(2000); //Pauses for 2s
  display.clearDisplay(); //Clears the OLED screen
  display.setTextColor(WHITE); //Sets the text colour
  display.invertDisplay(true); //Inverts the display
  mainMenu(); //Runs main menu function which displays the main menu
  menuSelect(); //Runs menu select function which allows the buttons on the extension board as a way for choosing options

  //-- SERVO --------------
  myservo.attach(D3); //Attaches the servo to pin D3

  //-- LED ---------
  pinMode(LED0, OUTPUT); //Sets the LED at pin D7 as output
  pinMode(LED1, OUTPUT); //Sets the LED at pin D8 as output
}

void loop() {    
  sensorValue = analogRead(A0); //Reads the raw value from the potentiometer and saves it into sensorValue
  pos = map(sensorValue, 0, 1023, 180, 0); //Remaps the raw value from the potentiometer into a range from 0 to 180 and saves it into variable pos.
  myservo.write(pos); //Sets the position of the servo to the value from pos.
  if (inSaved) { //Checks if user is in the in Saved menu
    servoMenu(); //Runs the servoMenu() function which allows the user to choose which save file they want to interact with using the potentiometer and servo.
  }
  screenDigits(); //Updates OLED screen digits when appropriate button is pressed.
  extensionDigits();//Updates extension board digits when appropriate button is pressed.
  menuSelect();//Enables function called menuSelect which allows the use of the extension board buttons as a way of selecting menu options.
  if (inSaved == false) { //Checks if user is not in the saved menu
    digitalWrite(LED0, LOW); //Turns LED0 OFF
    digitalWrite(LED1, LOW); //Turns LED1 OFF
  }
}

void screenDigits() { //Updates OLED screen digits when appropriate button is pressed.
  if ((presses0 != change0 || presses1 !=change1 || presses2 != change2 || presses3 != change3) && inPin == true){ //Checks if the value of the OLED screen digits is different then the value of the extension board digits and if the user is in the PIN menu.
    change0 = presses0; //Sets the value 1st OLED screen digit to the same value as the 1st 7-segmemt display digit
    change1 = presses1; //Sets the value 2nd OLED screen digit to the same value as the 2nd 7-segmemt display digit
    change2 = presses2; //Sets the value 3rd OLED screen digit to the same value as the 3rd 7-segmemt display digit
    change3 = presses3; //Sets the value 4th OLED screen digit to the same value as the 4th 7-segmemt display digit
    if (lock == false) { //Checks if device is not locked. If not locked displays the digit updates on the unlocked pin menu
      display.clearDisplay(); //Clears the OLED screen
      display.setCursor(2,4); //Sets position of text.
      display.setTextSize(1); // Set the text size to 1
      display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
      display << "LOCK" << "     BACK" << "  UNLOCK" <<endl; //Display buttons text
      display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
      display.drawFastVLine(48,0,14, WHITE);//Draws the 1st vertical line
      display.drawFastVLine(86,0,14, WHITE);//Draws the 2nd vertial line
      display.setCursor(2,18); //Sets position of text
      display << "Choose PIN" << endl; //Display message
      display.setCursor(2,28); //Sets position of text
      display << "The PIN will be:" << change0 << change1 << change2 << change3; //Displays what the pin will be using the value of change0, change1, change2, change3
      Serial << "The PIN will be:" << change0 << change1 << change2 << change3 << endl;
      display.display(); //Displays
      delay(300); //Pauses for 300 ms
    } else  if (lock == true) { //Checks if device is locked. If locked displays the digit updates on the locked pin menu
      display.clearDisplay(); //Clears the OLED screen
      display.setCursor(2,4); //Sets position of text.
      display.setTextSize(1); // Set the text size to 1
      display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
      display << "LOCK" << "     BACK" << "  UNLOCK" <<endl; //Display buttons text
      display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
      display.drawFastVLine(48,0,14, WHITE);//Draws the 1st vertical line
      display.drawFastVLine(86,0,14, WHITE);//Draws the 2nd vertial line
      display.setCursor(2,18); //Sets position of text
      display << "Device is locked." << endl; //Display message
      display.setCursor(2,28); //Sets position of text
      display << "Enter PIN" << endl; //Display message
      display.setCursor(2,38);
      display << "Entered PIN: " << change0 << change1 << change2 << change3; //Displays what the pin will be using the value of change0, change1, change2, change3
      Serial << "Entered PIN: " << change0 << change1 << change2 << change3 << endl; //Displays what the pin will be using the value of change0, change1, change2, change3
      display.display(); //Displays
      delay(300); //Pauses for 300 ms
    }
  }
}

void extensionDigits() { //Updates the extension board 7-segment display digits when appropriate button is presses.
  buttons=module.getButtons(); // Gets the value of the value of the button presses and sets them equal to the variable called buttons.
  if (inPin == true) { // Checks if user is in PIN menu
    if (clearDigits == true) { //Checks if digits need to be cleared
      module.setDisplayDigit(0, 0, false); //Sets the 1st 7-segment display to 0
      module.setDisplayDigit(0, 1, false); //Sets the 2nd 7-segment display to 0
      module.setDisplayDigit(0, 2, false); //Sets the 3rd 7-segment display to 0
      module.setDisplayDigit(0, 3, false); //Sets the 4th 7-segment display to 0
      //Sets the presses variables to 0
      presses0 = 0; 
      presses1 = 0;
      presses2 = 0;
      presses3 = 0;
      //Sets thhe change variables to 0
      change0 = 0;
      change1 = 0;
      change2 = 0;
      change3 = 0;
      clearDigits = false; //Stop the if statement from looping by setting clearDigits to false
      delay(300); //Waits 300ms
    }
    switch (buttons) { //Switch statement which gets the value from the buttons as condition
      case 1: //If 1st button is pressed
        presses0++; //Adds 1 to the presses0 variable
        module.setLED(1, 0); //Turns the 1st LED on the expansion board ON
        delay(300); //Pauses for 300ms
        module.setLED(0, 0); //Turns the 1st LED on the expansion board OFF
        if (presses0 > 9){ //Checks if the presses0 variable is bigger then 9
          presses0 = 0; //Sets the presses0 variable to 0 to avoid going into hex
        }
        module.setDisplayDigit(presses0, 0, false); //Sets the 1st 7-segment display to the value of presses0
        break;
      case 2: //If 2nd button is pressed
        presses1++; //Adds 1 to the presses1 variable
        module.setLED(1, 1); //Turns the 2nd LED on the expansion board ON
        delay(300); //Pauses for 300ms
        module.setLED(0, 1); //Turns the 2nd LED on the expansion board OFF
        if (presses1 > 9){ //Checks if the presses1 variable is bigger then 9
          presses1 = 0; //Sets the presses1 variable to 0 to avoid going into hex
        }
        module.setDisplayDigit(presses1, 1, false); //Sets the 2nd 7-segment display to the value of presses1
        break;
      case 4: //If 3rd button is pressed
        presses2++; //Adds 1 to the presses2 variable
        module.setLED(1, 2); //Turns the 3rd LED on the expansion board ON
        delay(300); //Pauses for 300ms
        module.setLED(0, 2); //Turns the 3rd LED on the expansion board OFF
        if (presses2 > 9){ //Checks if the presses2 variable is bigger then 9
          presses2 = 0; //Sets the presses2 variable to 0 to avoid going into hex
        }
        module.setDisplayDigit(presses2, 2, false); //Sets the 3rd 7-segment display to the value of presses2
        break;
      case 8: //If 4th button is pressed
        presses3++; //Adds 1 to the presses3 variable
        module.setLED(1, 3); //Turns the 4th LED on the expansion board ON
        delay(300); //Pauses for 300ms
        module.setLED(0, 3); //Turns the 4th LED on the expansion board OFF
        if (presses3 > 9){ //Checks if the presses3 variable is bigger then 9
          presses3 = 0; //Sets the presses3 variable to 0 to avoid going into hex
        }
        module.setDisplayDigit(presses3, 3, false); //Sets the 4th 7-segment display to the value of presses2
        break;
    }
  }
}

void menuSelect() { //Use of extension board buttons as a way of selecting menu options.
      buttons=module.getButtons(); // read which buttons are pressed
      switch (buttons) { //Switch statement which gets the value from the buttons as condition
        case 16: //If 5th button is pressed
          module.setLED(1, 4); //Turns the 5th LED on the expansion board ON
          delay(300); //Pauses for 300ms
          module.setLED(0, 4); //Turns the 5th LED on the expansion board OFF
          if (inMain == true) { //Checks if user is in the main menu. GENERATOR BUTTON
            generatorMenu(); //Runs the generatorMenu function which displays the generator menu
            delay(300); //Pauses for 300ms
          } else if (inGenerator == true) { //Checks if user is in the generator menu. GENERATE BUTTON
            display.clearDisplay(); //Clears the OLED screen
            generatedFull = ""; //Sets the generatedFull variable to empty string
            generatorMenu(); //Runs the generatorMenu function which displays the generator menu
            generateDifficult(); //Runs the generateDifficult function which generates a password
            delay(300); //Pauses for 300ms
          } else if (inPin == true && lock == false) { //Checks if user is in pin menu and if the device is unlocked. LOCK BUTTON
            //Sets the pin variables equal to the presses variables
            pin0 = presses0;
            pin1 = presses1;
            pin2 = presses2;
            pin3 = presses3;
            lock = true; //Sets lock variable to true which locks the device
            clearDigits = true; //Sets clearDigits variable to true which clears the digits
            lockedPinMenu(); //Runs the lockedPinMenu function which displays the locked pin menu
            delay(300); //Pauses for 300ms
          } else  if (lock == true) { //Checks if the device is locked
            display.setCursor(2,54); //Sets position of text
            display << "ALREADY LOCKED !!! "; //Displays message
            display.display(); //Displays
            delay(2000); //Pauses for 2s
            display.clearDisplay(); //Clears OLED screen
            lockedPinMenu(); //Runs the lockedPinMenu function which displays the locked pin menu
            delay(300); //Pauses for 300ms
          }
          break;
        case 32: //If 6th button is pressed
          module.setLED(1, 5); //Turns the 6th LED on the expansion board ON
          delay(300); //Pauses for 300ms
          module.setLED(0, 5); //Turns the 6th LED on the expansion board OFF
          if (inMain == true) { //Checks if user is in the main menu. PIN BUTTON
            pinMenu(); //Runs the pinMenu function which displays the pinMenu
            clearDigits = true; //Sets clearDigits variable to true which clears the digits
            delay(300); //Pauses for 300ms
          } else if (lock == false && inPin == true) { //Checks if device is unlocked and user is in the pin menu. BACK BUTTON
            mainMenu(); //Goes back to main menu
            delay(300); //Pauses for 300ms
          } else if (inSaved == true) { //Checks if user is in the saved menu. BACK BUTTON
            mainMenu(); //Goes back to main menu
            delay(300); //Pauses for 300ms
          } else if (inGenerator == true) { //Checks if user is in the generator menu. BACK BUTTON
            mainMenu(); //Goes back to main menu
            delay(300); //Pauses for 300ms
          } else if (inEaster == true) { //Checks if user is in the generator menu. BACK BUTTON
            mainMenu(); //Goes back to main menu
            delay(300); //Pauses for 300ms
          } 
          break;
        case 64: //If 7th button is pressed 
          module.setLED(1, 6); //Turns the 7th LED on the expansion board ON
          delay(300); //Pauses for 300ms
          module.setLED(0, 6); //Turns the 7th LED on the expansion board OFF
          if (inMain == true) { //Checks if user is in the main menu. SAVED BUTTON
            savedMenu(); //Runs the savedMenu function which displays the saved menu
            delay(300); //Pauses for 300ms
          } else if (inGenerator == true && generated == true) { //Checks if user is in the generator menu and if a password is generated. SAVE BUTTON
            saveGenerated(); //Runs the saveGenerated function which saves the generated password
            delay(300); //Pauses for 300ms
          } else if (inPin == true && lock == true && presses0 == pin0 && presses1 == pin1 && presses2 == pin2 && presses3 == pin3) { //Checks if user is in the pin menu, device is locked and the presses variables are equal to the pin variables. UNLOCK BUTTON
            lock = false; //Sets the lock variable to false which unlock the device
            clearDigits = true; //Sets clearDigits variable to true which clears the digits
            pinMenu(); //Runs the pinMenu function which displays the pin menu
            delay(300); //Pauses for 300ms
          } else if (inPin == true && lock == true && (presses0 != pin0 || presses1 != pin1 || presses2 != pin2 || presses3 != pin3)) { //Checks if user is in the pin menu, device is locked and the presses variables are not equal to the pin variables. UNLOCK BUTTON
            display.setCursor(2,54); //Sets position of text
            display << "INCORRECT PIN !!! "; //Displays message
            display.display(); //Displays
            delay(2000); //Pauses for 2s
            display.clearDisplay(); //Clears OLED screen
            lockedPinMenu(); //Runs the lockedPinMenu function which displays the locked pin menu.
            clearDigits = true; //Sets clearDigits variable to true which clears the digits
            delay(300); //Pauses for 300ms
          } else if (inPin == true && lock == false) { //Checks if user in the pin menu and the device is not locked. UNLOCK BUTTON
            display.setCursor(2,54); //Sets position of text
            display << "ALREADY UNLOCKED!!! "; //Displays message
            display.display(); //Displays
            delay(2000); //Pauses for 2s
            display.clearDisplay(); //Clears OLED screen
            pinMenu(); //Runs the PinMenu function which displays the pin menu.
            clearDigits = true;
            delay(300);
          } else if (inSaved == true) { //Checks if user is in the saved menu. DELETE BUTTON
            if (option0 == true && saved0 != "") { //Checks if the 1st save is selected and if the 1st save is not equal to an empty string
              saved0 = ""; //Sets the 1st save to an empty string
              slot0 = false; //Sets the slot0 variable to false indicating that the slot is not in use
              saveCount--; //Removes 1 from the save count
              Serial << "Save slot 1 was deleted" << endl;
              savedMenu(); //Runs the savedMenu function which displays the saved menu.
              delay(300); //Pauses for 300ms
            } else if (option1 == true && saved1 != "") { //Checks if the 2nd save is selected and if the 2nd save is not equal to an empty string
              saved1 = ""; //Sets the 2nd save to an empty string
              slot1 = false; //Sets the slot1 variable to false indicating that the slot is not in use
              saveCount--; //Removes 1 from the save count
              Serial << "Save slot 2 was deleted" << endl;
              savedMenu(); //Runs the savedMenu function which displays the saved menu
              delay(300); //Pauses for 300ms         
            } else if (option2 == true && saved2 != "") { //Checks if the 3rd save is selected and if the 3rd save is not equal to an empty string
              saved2 = ""; //Sets the 3rd save to an empty string
              slot2 = false; //Sets the slot2 variable to false indicating that the slot is not in use
              saveCount--; //Removes 1 from the save count
              Serial << "Save slot 3 was deleted" << endl;
              savedMenu(); //Runs the savedMenu function which displays the saved menu
              delay(300); //Pauses for 300ms 
            } else if (option3 == true && saved3 != "") { //Checks if the 4th save is selected and if the 4th save is not equal to an empty string
              saved3 = ""; //Sets the 4th save to an empty string
              slot3 = false; //Sets the slot3 variable to false indicating that the slot is not in use
              saveCount--; //Removes 1 from the save count
              savedMenu(); //Runs the savedMenu function which displays the saved menu
              Serial << "Save slot 4 was deleted" << endl;
              delay(300); //Pauses for 300ms
            }   
          }
          break;
        case 128:
          module.setLED(1, 7); //Turns the 7th LED on the expansion board ON
          delay(300); //Pauses for 300ms
          module.setLED(0, 7); //Turns the 7th LED on the expansion board OFF
          if (inMain == true) {
            easterEgg();
          }
          break;
    }
}

void generateDifficult() {
  module.setLEDs(0x00FF); //Sets all the LEDS on the extension board ON
  generated = true; //Sets the generated variable to true indicating that a password was generated
  rtc.getTime(t); //Gets the time from the RTC
  seconds = rtc.getSecond(); //Gets the seconds from the RTC and stores them in a variable called seconds
  delay(500); //Pauses for 500ms
  randomSeed(seconds); //Initializes the pseudo-random number generator with the seed set to the seconds variable
  Serial<<"Generated difficult password: ";
  int y= 0; //Declares a variable called y which will be used to change the position of the characters being displayed on the screen in the for loop
  for (int i = 0; i < 8; i++){
    display.setTextSize(2); //Sets the text size to 2
    display.setCursor(12 + y, 45); //Sets the text position
    y += 13; //Increases the value of y to make enough space for the other characters to be displayed
    generatedChar = difficult[random(71) % 70]; //Sets the generatedChar variable to a random letter or symbol using the dificult variable
    module.setLED(0, i); //Turns the LED of the extension board in position i OFF
    display << generatedChar; //Displays the randomly generated character
    Serial << generatedChar; //Displays the randomly generated character in serial
    display.display();  //Displays
    display.setTextSize(1); //SEts the text size to 1
    yield(); //
    generatedFull += generatedChar; //Adds the randomly generated characters to the generatedFull variable
  }
}

void pinMenu() {
  Serial << endl;
  Serial << "You are in the PIN MENU" << endl;
  module.setDisplayToString(" PIN", false, 4); //Displays PIN on the 7-segment display
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "LOCK" << "     BACK" << "  UNLOCK" <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(48,0,14, WHITE);//Draws the 1st vertical line
  display.drawFastVLine(86,0,14, WHITE);//Draws the 2nd vertical line
  display.setCursor(2,18); //Sets position of text
  display << "Choose PIN" << endl; //Displays message
  Serial << "Choose PIN" << endl;
  display.display(); //Displays
  inMain = false; //Sets the inMain variable to false. Indicating user is not in the main menu
  inPin = true; //Sets the inPin variable to true. Indicating user is in the pin menu
}

void lockedPinMenu() {
  Serial << endl;
  Serial << "You are in the LOCKED PIN MENU" << endl;
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "LOCK" << "     BACK" << "  UNLOCK" <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(48,0,14, WHITE); //Draws the 1st vertical line
  display.drawFastVLine(86,0,14, WHITE); //Draws the 2nd vertical line
  display.setCursor(2,18); //Sets position of text
  display << "Device is locked." << endl; //Displays message
  Serial << "Device is locked." << endl; //Displays message in serial
  display.setCursor(2,28); //Sets position of text
  display << "Enter PIN" << endl; //Displays message
  Serial << "Enter PIN" << endl; //Displays message in serial
  display.display(); //Displays
  inMain = false; //Sets the inMain variable to false. Indicating user is not in the main menu
  inPin = true; //Sets the inPin variable to true. Indicating user is in the pin menu
}

void mainMenu() {
  Serial << endl;
  Serial << "You are in the MAIN MENU" << endl;
  module.setDisplayToString("MAINMENU");
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "GENERATOR" << "  PIN" << "  SAVED" <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(61,0,14, WHITE); //Draws the 1st vertical line
  display.drawFastVLine(92,0,14, WHITE); //Draws the 2nd vertical line
  display.setCursor(2,18); //Sets position of text
  display << "Welcome to Martin's" << endl; //Displays message
  Serial << "Welcome to Martin's" << endl; //Displays message in serial
  display.setCursor(2,28); //Sets position of text
  display << "password generator" << endl; //Displays message
  Serial << "password generator" << endl; //Displays message in serial
  display.setCursor(2,38); //Sets position of text
  display << "use the buttons to" << endl; //Displays message
  Serial << "use the buttons to" << endl; //Displays message in serial
  display.setCursor(2,48); //Sets position of text
  display << "choose your menu." << endl; //Displays message
  Serial << "choose your menu." << endl; //Displays message in serial
  display.display(); //Displays
  inMain = true; //Sets the inMain variable to true. Indicating user is in the main menu
  inGenerator = false; //Sets the inGenerator variable to false. Indicating user is not in the generator menu
  inPin = false; //Sets the inPin variable to false. Indicating user is not in the pin menu
  inSaved = false; //Sets the inSaved variable to false. Indicating user is not in the saved menu
}

void generatorMenu() {
  Serial << endl;
  Serial << "You are in the GENERATOR MENU" << endl;
  module.clearDisplay(); //Clears the 7-segment display
  module.setDisplayToString("GENERATE"); //Displays GENERATE on the 7-segment display
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "GENERATE" << "  BACK" << "  SAVE" <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(55,0,14, WHITE); //Draws the 1st vertical line
  display.drawFastVLine(92,0,14, WHITE); //Draws the 2nd vertical line
  display.setCursor(2,18); //Sets position of text
  display << "Press GENERATE to" << endl; //Displays message
  Serial << "Press GENERATE to" << endl; //Displays message
  display.setCursor(2,28); //Sets position of text
  display << "generate a password" << endl; //Displays message
  Serial << "generate a password" << endl; //Displays message
  display.display(); //Displays
  inMain = false; //Sets the inMain variable to false. Indicating user is not in the main menu
  inGenerator = true; //Sets the inGenerator variable to true. Indicating user is in the generator menu
}

void savedMenu() {
  Serial << endl;
  Serial << "You are in the SAVED menu" << endl;
  module.clearDisplay(); //Clears the 7-segment display
  module.setDisplayToString("SAVED"); //Displays SAVED on the 7-segment display
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "///////" << "  BACK" << "  DELETE" <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(48,0,14, WHITE); //Draws the 1st vertical line
  display.drawFastVLine(86,0,14, WHITE); //Draws the 2nd vertical line
  display.setCursor(2,17); //Sets position of text
  display << "Used " << saveCount << "/4 saves:" << endl; //Displays how many saves are used using the saveCount variable
  Serial << "Used " << saveCount << "/4 saves:" << endl;
  display.setCursor(2,25); //Sets position of text
  display << "Save 1: " << saved0 << endl; //Displays message
  Serial << "Save 1: " << saved0 << endl; //Displays message
  display.setCursor(2,35); //Sets position of text
  display << "Save 2: " << saved1 << endl; //Displays message
  Serial << "Save 2: " << saved1 << endl; //Displays message
  display.setCursor(2,45); //Sets position of text
  display << "Save 3: " << saved2 << endl; //Displays message
  Serial << "Save 3: " << saved2 << endl; //Displays message
  display.setCursor(2,55); //Sets position of text
  display << "Save 4: " << saved3 << endl; //Displays message
  Serial << "Save 4: " << saved3 << endl; //Displays message
  display.display(); //Displays
  inMain = false; //Sets the inMain variable to false. Indicating user is not in the main menu
  inSaved = true; //Sets the inSaved variable to true. Indicating user is in the saved menu
}

void saveGenerated() { //Saves the generator password
  if (slot0 == false) { //Checks if the 1st slot is empty
    saved0 = generatedFull; //Sets the saved0 variable equal to the generatedFull variable
    Serial << endl;
    Serial << "You have saved: " << saved0 << endl;
    slot0 = true; //Sets the slot0 variable to true. Indicating the 1st save slot is taken
    saveCount++; //Adds 1 to the save count variable
    generated = false; //Sets the generated variable to false
  } else if (slot1 == false) { //Checks if the 2nd slot is empty
    saved1 = generatedFull; //Sets the saved1 variable equal to the generatedFull variable
    Serial << endl;
    Serial << "You have saved: " << saved1 << endl;
    slot1 = true; //Sets the slot1 variable to true. Indicating the 2nd save slot is taken
    saveCount++; //Adds 1 to the save count variable
    generated = false; //Sets the generated variable to false
  } else if (slot2 == false) { //Checks if the 3rd slot is empty
    saved2 = generatedFull; //Sets the saved2 variable equal to the generatedFull variable
    Serial << endl;
    Serial << "You have saved: " << saved2 << endl;
    slot2 = true; //Sets the slot2 variable to true. Indicating the 3rd save slot is taken
    saveCount++; //Adds 1 to the save count variable
    generated = false; //Sets the generated variable to false
  } else if (slot3 == false) { //Checks if the 4th slot is empty
    saved3 = generatedFull; //Sets the saved3 variable equal to the generatedFull variable
    Serial << endl;
    Serial << "You have saved: " << saved3 << endl;
    slot3 = true; //Sets the slot3 variable to true. Indicating the 3rd save slot is taken
    saveCount++; //Adds 1 to the save count variable
    generated = false; //Sets the generated variable to false
  } else { //If all slot are taken
    Serial << endl;
    Serial << "SAVE SLOTS FULL" << endl;
  }
}

void servoMenu() { //Uses the potentiometer to select save slots
  if (pos >=0 && pos <45) { //Checks the position
    option0 = false;
    option1 = false;
    option2 = false;
    option3 = true; //Sets the option3 variable to true indicating the 4th save is selected
    digitalWrite(LED0, LOW); //Turns LED 0 and 1 OFF to represent 4 using the LEDs
    digitalWrite(LED1, LOW);
  } else if (pos >=45 && pos < 90) { //Checks the position
    option0 = false;
    option1 = false;
    option2 = true; //Sets the option2 variable to true indicating the 3rd save is selected
    option3 = false;
    digitalWrite(LED0, HIGH); //Turns LED 0 and 1 ON to represent 3 in binary using the LEDs
    digitalWrite(LED1, HIGH);
  } else if (pos >=90 && pos <135) { //Checks the position
    option0 = false;
    option1 = true; //Sets the option1 variable to true indicating the 2nd save is selected
    option2 = false;
    option3 = false;
    digitalWrite(LED0, HIGH); //Turns LED 0 ON and LED 1 off to represent 2 in binary using the LEDs
    digitalWrite(LED1, LOW);
  } else if (pos >=135 && pos <= 180) { //Checks the position
    option0 = true; //Sets the option0 variable to true indicating the 1st save is selected]
    option1 = false;
    option2 = false;
    option3 = false;
    digitalWrite(LED0, LOW); //Turns LED 0 OFF and LED 1 ON to represent 1 in binary using the LEDs
    digitalWrite(LED1, HIGH);
  }
}

void easterEgg() {
  Serial << endl;
  Serial << "You are in the EASTER EGG" << endl;
  module.clearDisplay(); //Clears the 7-segment display
  module.setDisplayToString("TIME", false, 0);
  int power = 0;
  for (int i = 8; i >= 0; i--) {
    module.setDisplayDigit(i, 7, false);
    Serial << i << endl;
    module.setLED(1, i-1); //Sets all the LEDS on the extension board ON
    module.setupDisplay(true, power);
    power++;
    delay(1000);
    if (i == 0) {
      Serial << "EASTER RAN" << endl;
      delay(250);
      module.setLEDs(0x0000);
      delay(250);
      module.setLEDs(0x00FF);
      delay(250);
      module.setLEDs(0x0000);
      delay(250);
      module.setLEDs(0x00FF);
      delay(250);
      module.setLEDs(0x0000);
      delay(250);
      module.setLEDs(0x00FF);
      delay(250);
      module.setLEDs(0x0000);
      delay(250);
      module.setLEDs(0x00FF);
      delay(250);
      module.setLEDs(0x0000);
      delay(250);
      module.setLEDs(0x00FF);
      delay(250);
      module.setLEDs(0x0000);
    }
  }
  display.clearDisplay(); //Clears the OLED screen
  display.setCursor(2,4); //Sets position of text
  display.setTextSize(1); //Sets the text size to 1
  display.drawFastHLine(0, 0, 127, WHITE ); //Draws the top horizontal line
  display << "   SUS " << "  MODE" << "   ON   " <<endl; //Displays the buttons text
  display.drawFastHLine(0, 14, 127, WHITE ); //Draws the bottom horizontal line
  display.drawFastVLine(48,0,14, WHITE); //Draws the 1st vertical line
  display.drawFastVLine(86,0,14, WHITE); //Draws the 2nd vertical line
  display.setCursor(2,17); //Sets position of text
  display.setTextSize(2);
  display << "It is just a timer";
  Serial << "It is just a timer" << endl;
  display.display(); //Displays
  inMain = false; //Sets the inMain variable to false. Indicating user is not in the main menu
  inEaster = true; //Sets the inSaved variable to true. Indicating user is in the saved menu
}
