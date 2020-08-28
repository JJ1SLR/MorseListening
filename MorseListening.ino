#include <LiquidCrystal.h>
#include <IRremote.h>

// Remote key defination
#define KEY_CH_MINUS 0xFFA25D
#define KEY_CH 0xFF629D
#define KEY_CH_PLUS 0xFFE21D

#define KEY_PREV 0xFF22DD
#define KEY_NEXT 0xFF02FD
#define KEY_PLAY 0xFFC23D

#define KEY_MINUS 0xFFE01F
#define KEY_PLUS 0xFFA857
#define KEY_EQ 0xFF906F

#define KEY_0 0xFF6897
#define KEY_100 0xFF9867
#define KEY_200 0xFFB04F

#define KEY_1 0xFF30CF
#define KEY_2 0xFF18E7
#define KEY_3 0xFF7A85

#define KEY_4 0xFF10EF
#define KEY_5 0xFF38C7
#define KEY_6 0xFF5AA5

#define KEY_7 0xFF42BD
#define KEY_8 0xFF4AB5
#define KEY_9 0xFF52AD

#define KEY_LAST 0xFFFFFFFF

// Macro functions
#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

// Pins defination
#define BUZZER_PIN 9
#define REMOTE_PIN 2
#define LED_PIN 8

#define MTABLE_CHAR_OFFSET 0x20

typedef struct {
  byte len;
  byte dat;
} MorseCode;

const PROGMEM MorseCode mTable[] = {
  { 0, B00000000 },       // 0x20, SPACE
  { 0, B11111111 },       // 0x21, ! | 
  { 6, B01001000 },       // 0x22, " | ・－・・－・
  { 0, B11111111 },       // 0x23, # | 
  { 0, B11111111 },       // 0x24, $ | 
  { 0, B11111111 },       // 0x25, % | 
  { 0, B11111111 },       // 0x26, & | 
  { 6, B01111000 },       // 0x27, ' | ・－－－－・
  { 5, B10110000 },       // 0x28, ( | －・－－・
  { 6, B10110100 },       // 0x29, ) | －・－－・－
  { 0, B11111111 },       // 0x2A, * | 
  { 5, B01010000 },       // 0x2B, + | ・－・－・
  { 6, B11001100 },       // 0x2C, , | －－・・－－
  { 6, B10000100 },       // 0x2D, - | －・・・・－
  { 6, B01010100 },       // 0x2E, . | ・－・－・－
  { 5, B10010000 },       // 0x2F, / | －・・－・
  { 5, B11111000 },       // 0x30, 0 | －－－－－
  { 5, B01111000 },       // 0x31, 1 | ・－－－－
  { 5, B00111000 },       // 0x32, 2 | ・・－－－
  { 5, B00011000 },       // 0x33, 3 | ・・・－－
  { 5, B00001000 },       // 0x34, 4 | ・・・・－
  { 5, B00000000 },       // 0x35, 5 | ・・・・・
  { 5, B10000000 },       // 0x36, 6 | －・・・・
  { 5, B11000000 },       // 0x37, 7 | －－・・・
  { 5, B11100000 },       // 0x38, 8 | －－－・・
  { 5, B11110000 },       // 0x39, 9 | －－－－・
  { 6, B11100000 },       // 0x3A, : | －－－・・・
  { 0, B11111111 },       // 0x3B, ; | 
  { 0, B11111111 },       // 0x3C, < | 
  { 5, B10001000 },       // 0x3D, = | －・・・－
  { 0, B11111111 },       // 0x3E, > | 
  { 6, B00110000 },       // 0x3F, ? | ・・－－・・
  { 6, B01101000 },       // 0x40, @ | ・－－・－・
  { 2, B01000000 },       // 0x41, A | ・－
  { 4, B10000000 },       // 0x42, B | －・・・
  { 4, B10100000 },       // 0x43, C | －・－・
  { 3, B10000000 },       // 0x44, D | －・・
  { 1, B00000000 },       // 0x45, E | ・
  { 4, B00100000 },       // 0x46, F | ・・－・
  { 3, B11000000 },       // 0x47, G | －－・
  { 4, B00000000 },       // 0x48, H | ・・・・
  { 2, B00000000 },       // 0x49, I | ・・
  { 4, B01110000 },       // 0x4A, J | ・－－－
  { 3, B10100000 },       // 0x4B, K | －・－
  { 4, B01000000 },       // 0x4C, L | ・－・・
  { 2, B11000000 },       // 0x4D, M | －－
  { 2, B10000000 },       // 0x4E, N | －・
  { 3, B11100000 },       // 0x4F, O | －－－
  { 4, B01100000 },       // 0x50, P | ・－－・
  { 4, B11010000 },       // 0x51, Q | －－・－
  { 3, B01000000 },       // 0x52, R | ・－・
  { 3, B00000000 },       // 0x53, S | ・・・
  { 1, B10000000 },       // 0x54, T | －
  { 3, B00100000 },       // 0x55, U | ・・－
  { 4, B00010000 },       // 0x56, V | ・・・－
  { 3, B01100000 },       // 0x57, W | ・－－
  { 4, B10010000 },       // 0x58, X | －・・－
  { 4, B10110000 },       // 0x59, Y | －・－－
  { 4, B11000000 },       // 0x5A, Z | －－・・
};

// Lcd defination
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

unsigned long duration = 80;
unsigned int frequency = 1000;

// Remote control global variables
volatile IRrecv irrecv(REMOTE_PIN);
volatile decode_results results;

void playTone(int freq, int dura) {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, freq, dura);
  delay(dura);
  digitalWrite(LED_PIN, LOW);
}

void playDot() {
  playTone(frequency, duration);
  delay(duration);
}

void playDash() {
  playTone(frequency, duration*3);
  delay(duration);
}

void playSep() {
  delay(duration*2);
}

void playWordSep() {
  delay(duration*6);
}

void ir0_handler() {
  //Serial.println("Enter Interrupt");
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}

void setup() {
  // Start Serial output
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize digital pin LED_PIN
  pinMode(LED_PIN, OUTPUT);
  // initialize buzzer output pin
  pinMode(BUZZER_PIN, OUTPUT);
  // initialize lcd
  lcd.begin(16, 2);
  lcd.cursor();
  lcd.blink();
  // Start the receiver
  irrecv.blink13(true);
  irrecv.enableIRIn(); 
  attachInterrupt(0, ir0_handler, CHANGE);
}

void loop() {
    playDash();
    playDot();
    playDash();
    playDot();
    playSep();
    lcd.print("C");
    
    playDash();
    playDash();
    playDot();
    playDash();
    playSep();
    lcd.print("Q");

    playWordSep();
    lcd.print(" ");

    playDash();
    playDot();
    playDash();
    playDot();
    playSep();
    lcd.print("C");
    
    playDash();
    playDash();
    playDot();
    playDash();
    playSep();
    lcd.print("Q");

    playWordSep();
    lcd.print(" ");

    playDash();
    playDot();
    playDash();
    playDot();
    playSep();
    lcd.print("C");
    
    playDash();
    playDash();
    playDot();
    playDash();
    playSep();
    lcd.print("Q");

    playWordSep();
    lcd.print(" ");
    
    playDash();
    playDot();
    playDot();
    playSep();
    lcd.print("D");
    
    playDot();
    playSep();
    lcd.print("E");

    playWordSep();
    lcd.print(" ");

    lcd.setCursor(0, 1);

    playDot();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("J");

    playDot();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("J");

    playDot();
    playDash();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("1");

    playDot();
    playDot();
    playDot();
    playSep();
    lcd.print("S");

    playDot();
    playDash();
    playDot();
    playDot();
    playSep();
    lcd.print("L");

    playDot();
    playDash();
    playDot();
    playSep();
    lcd.print("R"); 
    

    playWordSep();
    lcd.print(" ");

    playDot();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("J");

    playDot();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("J");

    playDot();
    playDash();
    playDash();
    playDash();
    playDash();
    playSep();
    lcd.print("1");

    playDot();
    playDot();
    playDot();
    playSep();
    lcd.print("S");

    playDot();
    playDash();
    playDot();
    playDot();
    playSep();
    lcd.print("L");

    playDot();
    playDash();
    playDot();
    playSep();
    lcd.print("R");

    playWordSep();
    lcd.print(" ");
    
    playDash();
    playDot();
    playDash();
    playSep();
    lcd.print("K");

//    digitalWrite(LED_BUILTIN, LOW);
    delay(5000);
    
//  }
  lcd.clear();
  lcd.setCursor(0, 0);
}
