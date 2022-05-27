/*
 Wire Master Writer
 by Nicholas Zambetti <http://www.zambetti.com>

 Demonstrates use of the Wire library
 Writes data to an I2C/TWI slave device
 Refer to the "Wire Slave Receiver" example for use with this

 Created 29 March 2006

 This example code is in the public domain.
*/

/*
 LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin byteerface.

 This sketch prbytes "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 1
 * LCD Enable pin to digital pin 0
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

/*
  Button

  Turns on and off a light emitting diode(LED) connected to digital pin 13,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - LED attached from pin 13 to ground
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  - Note: on most Arduinos there is already an LED on the board
    attached to pin 13.

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Button
*/

// Libraries used in this code (LCD, I2C):
#include <LiquidCrystal.h>
#include <Wire.h>

// Declaring CONSTANTS:
// const byte UP_Button_Pin = 6, DOWN_Button_Pin = 7, OK_Button_Pin = 8;       //--------------TEST MODE ONLY----------------
// const byte rs = 1, en = 0, d4 = 5, d5 = 4, d6 = 3, d7 = 2;                  //--------------TEST MODE ONLY----------------
const byte rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 10;       // LCD HW interface
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); 
const byte IODIRA_B[3] = {0, 0, 0};                                       // GPIO extender IC message: Set I/O direction to output
const byte latch_clear[3] = {20, 0, 0};                                   // GPIO extender IC message: Clear outputs

// Declaring VARIABLES:
// bool UP_Button_State = 0, DOWN_Button_State = 0, OK_Button_State = 0;       //--------------TEST MODE ONLY----------------
int Buttons_State = 0;                         // Variables to store button state
byte wire_addr = 0;                           // GPIO extender ICs physical address (between 32(0x20) and 40(0x27))
byte wr_addr = 0, wr_bit = 0;                 // GPIO extender ICs address (0 to 7), byte (0 to 7) control
byte Config_buffer = 0;                       // Configuration buffer for storing relay number
long latch[8] = {0, 0, 0, 0, 0, 0, 0, 0};     // 16-bit output latches for GPIO extender ICs coversion from relay number to bitwise output control
byte latch_send[2] = {0,0};                   // 8-bit output latches for GPIO extender ICs coversion from relay number to bitwise output control
byte cnt = 0;                                 // Configuration counter (0 to 50)
byte i = 0;                                   // Loop counter

// LUT for displaying the name of the configurations on the LCD
const char *Config_name[] = {"NGI X760 M8", "NGI X760 M12", "NGI X760 M16", "NGI X761 M8", "NGI X761 M12", "NGI X761 M16", "NGI X260 M8", "NGI X260 M12", "NGI X260 M16", "NGI L560 M8", "NGI L560 M12", "NGI L560 M16", "NGI L550 M8", "NGI L550 M12", "NGI L550 M16", "NGI L550 M8", "NGI L550 M12", "NGI L550 M16", "ADC L550 M8", "ADC L550 M12", "ADC L550 M16", "NGI L551 M8", "NGI L551 M12", "NGI L551 M16", "ADC L551 M8", "ADC L551 M12", "ADC L551 M16", "NGI X540 M8", "NGI X540 M12", "NGI X540 M16", "ADC X540 M8", "ADC X540 M12", "ADC X540 M16", "ADC X760 M4", "ADC X760 M14", "ADC X760 M18", "ADC X761 M8", "ADC X761 M14", "ADC X761 M18", "ADC X260 M8", "ADC X260 M14", "ADC X260 M18", "ADC L560 M8", "ADC L560 M14", "ADC L560 M18", "ADC X540 M8", "ADC X540 M14", "ADC X540 M18", "ADC L663 M8", "ADC L663 M14", "ADC L663 M18"};

// LUT for storing the configurations relay numbers
const PROGMEM byte Config_data[51][18] = {{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,57,63,67,0,0,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,57,63,67,0,0,0},{1,5,10,16,26,31,36,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,57,63,67,0,0,0},{1,6,10,17,26,32,36,39,0,0,0,0,0,0,0,0,0,0},{1,4,10,15,26,32,36,39,40,44,51,55,0,0,0,0,0,0},{1,4,10,15,26,32,36,39,40,44,51,55,57,63,67,0,0,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,57,63,67,0,0,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,58,64,67,0,0,0},{1,7,11,18,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,24,29,34,38,43,47,49,53,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,0,0,66,69,71,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,40,44,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,40,44,51,55,58,64,67,0,0,0},{1,7,0,0,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,48,52,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,40,44,51,55,0,0,66,69,71,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,58,64,67,0,0,0},{1,5,10,16,0,31,0,38,0,0,0,0,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,0,0,0,0,0,0},{1,6,10,17,26,32,36,39,41,45,51,55,59,62,67,0,0,0},{0,0,0,0,22, 24,27,29,34,38,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,49,53,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,0,0,65,68,71,0},{1,7,14,21,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,49,53,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,0,0,65,68,71,0},{1,7,11,18,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,49,53,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,0,0,65,68,71,0},{1,7,14,21,25,30,33,35,37, 39,0,0,0,0,0,0,0,0},{1,7,11,18,23,28,35,39,43,47,48,52,60,0,0,0,0,0},{2,8,12,19,22,27,33,37,40,44,51,55,56,61,65,68,71,0},{1,7,0,0,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,49,53,60,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,0,0,66,69,71,0},{1,7,14,21,24,29,34,38,0,0,0,0,0,0,0,0,0,0},{1,7,11,18,25,30,35,39,43,47,49,53,0,0,0,0,0,0},{3,9,12,19,22,27,33,37,41,45,51,55,58,64,0,0,70,72}};

// LUT for storing the configurations relay numbers
const PROGMEM byte LED_data[51][18] = {{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,21,23,27,0,0,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,21,23,27,0,0,0},{1,5,7,11,13,14,16,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,21,23,27,0,0,0},{1,6,7,12,13,15,16,18,0,0,0,0,0,0,0,0,0,0},{1,4,7,10,13,15,16,18,2,8,28,28,0,0,0,0,0,0},{1,4,7,10,13,15,16,18,2,8,28,28,21,23,27,0,0,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,21,23,27,0,0,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,29,29,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,29,29,22,24,27,0,0,0},{1,7,13,16,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,5,11,14,17,29,29,3,9,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,29,29,0,0,22,24,27,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,2,8,29,29,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,2,8,29,29,22,24,27,0,0,0},{1,7,0,0,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,29,29,2,8,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,2,8,29,29,0,0,22,24,27,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,28,28,22,24,27,0,0,0},{1,5,7,11,0,14,0,17,0,0,0,0,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,29,29,0,0,0,0,0,0},{1,6,7,12,13,15,16,18,3,9,29,29,24,22,27,0,0,0},{0,0,0,0,1,5,7,11,14,17,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,28,28,3,9,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,28,28,0,0,21,23,27,0},{1,7,28,28,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,28,28,3,9,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,28,28,0,0,21,23,27,0},{1,7,13,16,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,28,28,3,9,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,28,28,0,0,21,23,27,0},{1,7,28,28,6,12,13,15,16,18,0,0,0,0,0,0,0,0},{1,7,13,16,4,10,15,18,28,28,2,8,27,0,0,0,0,0},{4,10,15,18,1,7,13,16,2,8,28,28,19,20,21,23,27,0},{1,7,0,0,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,29,29,3,9,27,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,29,29,0,0,22,24,27,0},{1,7,29,29,5,11,14,17,0,0,0,0,0,0,0,0,0,0},{1,7,13,16,6,12,15,18,29,29,3,9,0,0,0,0,0,0},{6,12,15,18,1,7,13,16,3,9,29,29,22,24,0,0,26,25}};

// Initialising function:
void setup() {
  // Initialiee the pushbuttons as inputs:
  // pinMode(UP_Button_Pin, INPUT);      //--------------TEST MODE ONLY----------------
  // pinMode(DOWN_Button_Pin, INPUT);    //--------------TEST MODE ONLY----------------
  // pinMode(OK_Button_Pin, INPUT);      //--------------TEST MODE ONLY----------------
 
  // Initialise LCD and print Home screen:
  lcd.begin(16, 2);                             // set up the LCD's number of columns (16) and rows (2):
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press OK to load");
  lcd.setCursor(0, 1);
  lcd.print(Config_name[cnt]); 

  // Join I2C bus (address optional for master) and set GPIO extender IC I/O direction to outputs
  Wire.begin();
  for ( wire_addr = 32; wire_addr < 40; ++wire_addr ) {
    Wire.beginTransmission(wire_addr);
    Wire.write(IODIRA_B,3);
    Wire.endTransmission(); 
  }
}

void loop() {
  // Read the state of the pushbuttons:
  Buttons_State = analogRead (0);
  // UP_Button_State = digitalRead(UP_Button_Pin);       //--------------TEST MODE ONLY---------------
  // DOWN_Button_State = digitalRead(DOWN_Button_Pin);   //--------------TEST MODE ONLY---------------
  // OK_Button_State = digitalRead(OK_Button_Pin);       //--------------TEST MODE ONLY---------------
  
  // Check if the pushbuttons are pressed, one by one. If any set the button state HIGH and execute routine:
  // UP_Button routine:
  // if (UP_Button_State == HIGH) {    //--------------TEST MODE ONLY---------------
  if (Buttons_State > 0 && Buttons_State < 200) {
    // Update the current configuration number
    if (cnt == 50) {
      cnt = 0;
    }
    else {
      cnt++;
    }
  // Print current configuration number on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press OK to load");
  lcd.setCursor(0, 1);
  lcd.print(Config_name[cnt]);
  delay(300);  
  }
  
  // DOWN_Button routine:
  // if (DOWN_Button_State == HIGH) {    //--------------TEST MODE ONLY---------------
  if (Buttons_State > 200 && Buttons_State < 600) {
    // Update the current configuration number
    if (cnt == 0) {
      cnt = 50;
    }
    else {
      cnt--;
    }
  // Print current configuration number on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press OK to load");
  lcd.setCursor(0, 1);
  lcd.print(Config_name[cnt]);
  delay(300);
  }

  // OK_Button routine:
  // if (OK_Button_State == HIGH) {    //--------------TEST MODE ONLY---------------
  if (Buttons_State > 600 && Buttons_State < 800) {
    // Indicate the configurion is being loaded...
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Loading Config:");
    lcd.setCursor(0, 1);
    lcd.print(Config_name[cnt]);
    delay(300);
    // Clear the latch variable
    for ( i = 0; i < 8; ++i ) {
      latch[i] = 0;
    }
    // Write 0s (clear) to the output latches
    for ( wire_addr = 32; wire_addr < 40; ++wire_addr ) {
      Wire.beginTransmission(wire_addr);
      Wire.write(latch_clear,3);
      Wire.endTransmission(); 
    }
    // Set outputs according to current config
    for ( i = 0; i < 18; ++i ) {                              // Loop through all amp channels
      Config_buffer = pgm_read_byte(&(Config_data[cnt][i]));  // Retrieve data from Flash memory for the current config
      if (Config_buffer != 0) {                               // Analyse non 0 values only
        wr_addr = (Config_buffer - 1) / 16;                   // Determine which GPIO extender IC to write to
        wr_bit = (Config_buffer - 1) % 16;                    // Determine which GPIO extender IC bit needs to be set
        bitSet(latch[wr_addr], wr_bit);                       // Set bit
        latch_send[0] = byte (latch[wr_addr]);                // Break into bytes for the two GPIO extender IC banks
        latch_send[1] = byte (latch[wr_addr] >> 8);
        wire_addr = 32 + wr_addr;                             // Set GPIO extender IC physical address for write operation (32 = 0x20)
        Wire.beginTransmission(wire_addr);                    // Transmit to device address
        Wire.write(byte(0x14));                               // IODIRA (bank 1 output latch address)
        Wire.write(latch_send,2);                             // Write to outputs
        Wire.endTransmission();                               // Stop transmitting       
        wire_addr = 0;                                        // Clear physical address for next loop cycle
        delay(100);                                           // Add a delay to limit relays switching current
      }
    }
    // Clear the latch variable
    for ( i = 0; i < 8; ++i ) {
      latch[i] = 0;
    }
    // Set LEDs according to current config
    for ( i = 0; i < 18; ++i ) {                              // Loop through all amp channels
      Config_buffer = pgm_read_byte(&(LED_data[cnt][i]));     // Retrieve data from Flash memory for the current config
      if (Config_buffer != 0) {                               // Analyse non 0 values only
        wr_addr = (Config_buffer - 1) / 16;                   // Determine which GPIO extender IC to write to
        wr_bit = (Config_buffer - 1) % 16;                    // Determine which GPIO extender IC bit needs to be set
        bitSet(latch[wr_addr], wr_bit);                       // Set bit
        latch_send[0] = byte (latch[wr_addr]);                // Break into bytes for the two GPIO extender IC banks
        latch_send[1] = byte (latch[wr_addr] >> 8);
        wire_addr = 37 + wr_addr;                             // Set GPIO extender IC physical address for write operation (37 = 0x25)
        Wire.beginTransmission(wire_addr);                    // Transmit to device address
        Wire.write(byte(0x14));                               // IODIRA (bank 1 output latch address)
        Wire.write(latch_send,2);                             // Write to outputs
        Wire.endTransmission();                               // Stop transmitting       
        wire_addr = 0;                                        // Clear physical address for next loop cycle
      }
    }
    // Print current configuration number on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Running Config:");
    lcd.setCursor(0, 1);
    lcd.print(Config_name[cnt]);
    delay(300);
  }
}