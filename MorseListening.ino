#include <LiquidCrystal.h>
#include <IRremote.h>


//******************************************************************************
// Hardware Defination
//******************************************************************************
#define BUZZER_PIN 9
#define LED_PIN 8

// LCD defination
LiquidCrystal g_lcd(12, 11, 7, 6, 5, 4);
#define LCD_COLS 16
#define LCD_ROWS 2

// Remote control global variables
volatile IRrecv g_irrecv(2);
volatile decode_results g_results;

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
//------------------------------------------------------------------------------


//******************************************************************************
// Software configure Defination
//******************************************************************************
#define CQ_CALL_SIGN "CQ CQ CQ DE JJ1SLR JJ1SLR K"
//------------------------------------------------------------------------------


//******************************************************************************
// DEBUG Macros
//******************************************************************************
//#define DEBUG
//#define DEBUG_ERR

#ifdef DEBUG
#define DBG_MSG(msg) Serial.print(F("DBG: ")); Serial.println(msg);
#define DBG_MSG_FMT(msg, fmt) Serial.print(F("DBG: ")); Serial.println(msg, fmt)
#else
#define DBG_MSG(msg)
#define DBG_MSG_FMT(msg, fmt)
#endif

#ifdef DEBUG_ERR
#define DBG_ERR(msg) Serial.print(F("ERR: ")); Serial.print(msg)
#define DBG_ERR_FMT(msg, fmt) Serial.print(F("ERR: ")); Serial.print(msg, fmt)
#else
#define DBG_ERR(msg)
#define DBG_ERR_FMT(msg, fmt)
#endif
//------------------------------------------------------------------------------


//******************************************************************************
// Morse data structure
//******************************************************************************
// Offset
#define MTABLE_CHAR_OFFSET 0x20
// Morse storage structure
typedef struct {
  byte len;
  byte dat;
} MorseCode;
// Morse data table
const MorseCode mTable[] = {
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
//------------------------------------------------------------------------------


//******************************************************************************
// Macro functions
//******************************************************************************
#define ARY_LEN(ary) (sizeof(ary)/sizeof(ary[0]))
//------------------------------------------------------------------------------


//******************************************************************************
// Global variables
//******************************************************************************
// sound last time ( = one dot time)
#define DEFAULT_DURATION 80
volatile unsigned long g_duration = DEFAULT_DURATION;
// sound g_frequency
#define DEFAULT_FREQUENCY 1000
volatile unsigned int g_frequency = DEFAULT_FREQUENCY;
#define DEFAULT_SEQ_DELAY 1000
unsigned int g_seq_delay = DEFAULT_SEQ_DELAY;
//------------------------------------------------------------------------------


//******************************************************************************
// Basic functions
//******************************************************************************
void delayWithChk(unsigned int mils) {
  unsigned long startMillis = millis();
  for (;;) {
    eventChecker();
    if(millis() - startMillis >= mils) {
      break;
    }
  }
}

void playTone(int freq, int dura) {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, freq, dura);
  delayWithChk(dura);
  digitalWrite(LED_PIN, LOW);
}

void playDot() {
  playTone(g_frequency, g_duration);
  delayWithChk(g_duration);
}

void playDash() {
  playTone(g_frequency, g_duration*3);
  delayWithChk(g_duration);
}

void playSep() {
  delayWithChk(g_duration*2);
}

void playWordSep() {
  delayWithChk(g_duration*6);
}

bool getBit(byte dat, byte i) {
  return (bool)((0x01 << (7 - i)) & dat);
}
//------------------------------------------------------------------------------


//******************************************************************************
// Module functions
//******************************************************************************
bool playChar(char ch, bool disp) {
//  Serial.println(ch);
  byte ic;  // index of mTable
  // invalid char
  if (ch < MTABLE_CHAR_OFFSET || ch >= MTABLE_CHAR_OFFSET + ARY_LEN(mTable)) {
    DBG_ERR(F("playChar(): Invalid char!"));
    DBG_ERR_FMT(ch, HEX);
    return false;
  }
  ic = ch - MTABLE_CHAR_OFFSET;
  
  DBG_MSG(F("playChar(): mTable[ic].len"));
  DBG_MSG(F("playChar(): mTable[ic].dat"));

  // space
  if (mTable[ic].len == 0 && mTable[ic].dat == 0) {
    playWordSep();
    return true;
  }
  // chars not in Morse
  if (mTable[ic].len == 0 && mTable[ic].dat != 0) {
    DBG_ERR(F("playChar(): Char not in Morse table!"));
    DBG_ERR_FMT(ch, HEX);
    return false;
  }

  // display char on g_lcd
  if (disp) {
    // display char on row 0
    g_lcd.clear();
    g_lcd.setCursor(0, 0);
    g_lcd.print(F("Charactor: "));
    g_lcd.write(ch);
    // display Morse on row 1
    g_lcd.setCursor(0, 1);
    g_lcd.print(F("Morse: "));
    for (int i = 0; i < mTable[ic].len; ++i) {
      if (getBit(mTable[ic].dat, i)) {
        g_lcd.write('-');
      } else {
        g_lcd.write('.');
      }
    }
  }

  // valid char
  for (int i = 0; i < mTable[ic].len; ++i) {
    if (getBit(mTable[ic].dat, i)) {
      playDash();
    } else {
      playDot();
    }
  }
  playSep();
  return true;
}

bool playChar(char ch) {
  return playChar(ch, false);
}

void playCQCallSign() {
  g_lcd.cursor();
  g_lcd.blink();
  char *pc = CQ_CALL_SIGN;
  byte len = strlen(pc);
  for (byte i = 0; i < len; ++i) {
    playChar(pc[i]);
    g_lcd.write(pc[i]);
    if (i == LCD_COLS - 1) {
      g_lcd.setCursor(0, 1);
    }
  }
}

void playSequence() {
  g_lcd.noCursor();
  g_lcd.noBlink();
  // play A-Z
  for (int i = 0x21; i < ARY_LEN(mTable); ++i) {
    if (playChar(i + MTABLE_CHAR_OFFSET, true)) {
      delayWithChk(g_seq_delay); 
    }
  }
  // play 0-9
  for (int i = 0x11; i < 0x1A; ++i) {
    if (playChar(i + MTABLE_CHAR_OFFSET, true)) {
      delayWithChk(g_seq_delay); 
    }
  }
  playChar('0', true);
  delayWithChk(g_seq_delay);
  playChar('.', true);
  delayWithChk(g_seq_delay);
  playChar(',', true);
  delayWithChk(g_seq_delay);
  playChar(':', true);
  delayWithChk(g_seq_delay);
  playChar('?', true);
  delayWithChk(g_seq_delay);
  playChar('\'', true);
  delayWithChk(g_seq_delay);
  playChar('-', true);
  delayWithChk(g_seq_delay);
  playChar('(', true);
  delayWithChk(g_seq_delay);
  playChar(')', true);
  delayWithChk(g_seq_delay);
  playChar('/', true);
  delayWithChk(g_seq_delay);
  playChar('=', true);
  delayWithChk(g_seq_delay);
  playChar('+', true);
  delayWithChk(g_seq_delay);
  playChar('"', true);
  delayWithChk(g_seq_delay);
  playChar('X', true);
  delayWithChk(g_seq_delay);
  playChar('@', true);
  delayWithChk(g_seq_delay);
}
//------------------------------------------------------------------------------


//******************************************************************************
// Public functions
//******************************************************************************
void eventChecker() {
  
}

void ir0_handler() {
  DBG_MSG(F("ir0_handler: Enter Interrupt"));
  if (g_irrecv.decode(&g_results)) {
    Serial.println(g_results.value, HEX);
    g_irrecv.resume(); // Receive the next value
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
  // initialize g_lcd
  g_lcd.begin(LCD_COLS, LCD_ROWS);
  g_lcd.cursor();
  g_lcd.blink();
  // Start the receiver
  g_irrecv.blink13(true);
  g_irrecv.enableIRIn(); 
  attachInterrupt(0, ir0_handler, CHANGE);
}

void loop() {
  // playCQCallSign();
  playSequence();

  delayWithChk(5000);
 
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
}
