/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define WARM 0
#define BREW 1
#define STOP 2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
int i = 0;
char count [5];
unsigned long current;
unsigned long last_millis;
int state;
bool prev_state;
int reading;
int stop_count = 0;
int brew_count = 0;

unsigned long warmup = 30*60*1000; // 30 minutes in milliseconds

void setup() {
  Serial.begin(9600);
  
  pinMode(8,OUTPUT);
  pinMode(2,OUTPUT);
  digitalWrite(8,LOW);  //ground for current sensor
  digitalWrite(2,HIGH); //power for display

  state = WARM;
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  u8g2_for_adafruit_gfx.begin(display);
  display.clearDisplay();                                 // clear the graphcis buffer  
  u8g2_for_adafruit_gfx.setFontMode(1);                   // use u8g2 transparent mode (this is default)
  u8g2_for_adafruit_gfx.setFontDirection(0);              // left to right (this is default)
  u8g2_for_adafruit_gfx.setForegroundColor(WHITE);        // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso58_tn); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  display.display();                                    // make everything visible
}

void loop() {

  current = millis();

  prev_state = state;

  //smoothing of signal to ignore oscillations
  reading = (reading*9 + analogRead(A1))/10;
  Serial.println(reading);
  //count the instances of zero reading
  if (reading == 0) {
    stop_count++;
    brew_count = 0;
  } else {
    brew_count++;
    stop_count = 0;
 }

  //change state if the zero reading is repeated
  if (state == WARM && current >= warmup) {
    display.clearDisplay();
    display.display();
    state = STOP;
  } else if (state == BREW && stop_count > 10) {
    state = STOP;
  } else if (state == STOP && brew_count > 10) {
    state = BREW;
  }

  //detect timer start and reset
  if (state == BREW && prev_state == STOP) {
    i = 0;  
  }

  //continue timing
  if ( state == WARM && current-last_millis >=60*1000) {
    sprintf(count, "%02d", (warmup-current)/(60*1000)); //convert to minutes and format timer number to have leading zeros
    u8g2_for_adafruit_gfx.setCursor(28,61);
    u8g2_for_adafruit_gfx.print(count);
    display.display();
     
  } else if (state == BREW && current-last_millis >= 1000) {

    last_millis = current;
    display.clearDisplay();
  
    if (i>999) {  // max time
      i=0;
    }
    
    if (i<=99) {                              //write two digits with digits centered on screen
      sprintf(count, "%02d", i);              //format timer number to have leading zeros
      u8g2_for_adafruit_gfx.setCursor(28,61);
    } else {                                  //write three digits with digits centered on screen
      sprintf(count, "%03d", i);              //format timer number to have leading zeros
      u8g2_for_adafruit_gfx.setCursor(10,61);
    }

    //update timer and increment
    u8g2_for_adafruit_gfx.print(count);
    display.display();
    i+=1;
    
  } else if (state == STOP && current - last_millis >= 5*60*1000){ //go to sleep after 5 min
    display.clearDisplay();
    display.display();
  }
}

