/*
 * TODO
 * add switch to detect open doors
 * Add button to disarm screen to arm the system
 */

#include "Adafruit_Keypad.h"
#include <TouchScreen.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include "pitches.h"
#define KEYPAD_PID3844
#define R1    30 //orig 2
#define R2    32 // orig 3
#define R3    34 // orig 4
#define R4    36 // orig 5
#define C1    22 // orig 8
#define C2    24 // orig 9
#define C3    26 // orig 10
#define C4    28 // orig 11
#include "keypad_config.h"
int redLed       = 25;
int greenLed     = 23;
int buzzer       = 27;
int doorSwitch   = 29;
int windowSwitch = 31;
int armSwitch    = 33;
int systemStatus = 0; // 0=alarm not armed (off) 1= alarm armed (on)
char correctPwd[5] = {'1','1','1','1','\0'};
char pwd[5];
int loopCycle = 1;
int i = 0;
int invalidPwdCount = 0;
int touch = 0;
//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 120
#define TS_MAXX 900
#define TS_MINY 70
#define TS_MAXY 920

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 40
#define PENRADIUS 3
#define MINPRESSURE 10
#define MAXPRESSURE 1000

void setup() {
  Serial.begin(9600);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(doorSwitch, INPUT);
  pinMode(windowSwitch, INPUT);
  pinMode(armSwitch, INPUT);
  pinMode(13, OUTPUT);
  customKeypad.begin();
  Serial.println(F("TFT LCD test"));

  #ifdef USE_Elegoo_SHIELD_PINOUT
    Serial.println(F("Using Elegoo 2.4\" TFT Arduino Shield Pinout"));
  #else
    Serial.println(F("Using Elegoo 2.4\" TFT Breakout Board Pinout"));
  #endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

   uint16_t identifier = tft.readID();
   if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }
  else if(identifier==0x1111)
  {     
      identifier=0x9328;
       Serial.println(F("Found 0x9328 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9328;
  
  }
  tft.begin(identifier);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setRotation(1);
  
}

void loop() {
  if (loopCycle == 1){
    systemStatus = turnAlarmOn();
    loopCycle ++;
  }

//  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
//  digitalWrite(13, LOW);

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    if (p.x > 200 && p.x < 320) {
      if (p.y > 40 && p.y < 220) {
        touch = 1;
      }
    }
    if (p.x > 120 && p.x < 240) {
      if (p.y > 0 && p.y < 120) {
        touch = 2;
      }
    }
    
  }
  if (touch == 1) {
    systemStatus = turnAlarmOn();
    touch = 0;
  }

  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(200, 100);
  
  // Maximum bad attempt count that will pause system or ??
  if (invalidPwdCount >2){  //check invalid pwd count to pause logic if needed
  // TODO fix the counter logic to pause system    
      loopCycle = 1;
      invalidPwdCount = 0;
      Serial.println("Next time I'm notifying the security company");
  }  
  customKeypad.tick();
  // save four key pad entries
  while(customKeypad.available() & i < 4){
     keypadEvent e = customKeypad.read();
     if(e.bit.EVENT == KEY_JUST_PRESSED) {
       Serial.println((char)e.bit.KEY);
       pwd[i] = (char)e.bit.KEY;
       i++;
     }
  }
    delay(10);
  // When pwd matches, process
  if(String(pwd) == correctPwd){
    systemStatus = turnAlarmOff();
  }

  // process alarm turn on switch
  if(digitalRead(armSwitch) == HIGH){
    //delay(10000);
    systemStatus = turnAlarmOn();
  }
}

void soundAlarm(){
  
}

int turnAlarmOff(){
  Serial.println("in turn alarm off");
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  tft.fillScreen(GREEN);
  tft.setCursor(10, 20);
  tft.setTextSize(3);
  tft.println(" ");
  tft.println(" Disarmed");  
  tft.println(" ");
  tft.setTextColor(BLACK);  
  tft.setTextSize(2);
  tft.println("  Please wait for door      to unlock");
  
  tft.fillRect(40, 150, 250, 50, WHITE);
  tft.drawRoundRect(40, 150, 250, 50, 6, RED);
  tft.drawRoundRect(39, 149, 252, 52, 6, RED);
  tft.drawRoundRect(38, 148, 254, 54, 6, RED);
  tft.setCursor(80, 165);
  tft.setTextSize(3);
  tft.println("SET ALARM");  
  
  for(int z = 0; z <4; z++){
   pwd[z] = {'z'};
  }
  int melody2[] = {262, 262, 262, 262, 262, 262, 262, 262};
  int noteDurations2[] = {4, 4, 4, 4, 4, 4, 4, 4};
 
  // play tone to indicate alarm is off
  for (int thisNote2 = 0; thisNote2 < 8; thisNote2++) {
    int noteDuration2 = 1000 / noteDurations2[thisNote2];
    tone(buzzer, melody2[thisNote2], noteDuration2);
    int pauseBetweenNotes2 = noteDuration2 * 1.30;
    delay(pauseBetweenNotes2);
    noTone(buzzer);
  }
  i=0;
    return 0;
}

int turnAlarmOn(){
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);
  tft.fillRect(0, 0, 320, 240, BLUE);
  tft.fillRect(10, 10, 300, 220, RED);
  tft.setTextColor(BLACK);  
  tft.setTextSize(3);
  tft.setCursor(25,40);
  tft.println("System is Armed");
  tft.println(" ");
  tft.setTextColor(WHITE);
  tft.println("  Enter Password");
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);
  for(int z = 0; z <4; z++){
   pwd[z] = {'z'};
  }
  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
  int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
  // play tone to indicate alarm is set
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzer, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(buzzer);
  }  
  return 1;
}
