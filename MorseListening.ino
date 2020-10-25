/*
 * This file is part of the MorseListening distribution.
 * (https://github.com/JJ1SLR/MorseListening)
 * Copyright (c) 2020 JJ1SLR.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * MorseListening: a Morse listening practice device.
 * Version 0.1.1
 *
*/
#include <LiquidCrystal.h>
#include <IRremote.h>
#include <setjmp.h>

//******************************************************************************
// Hardware Defination
//******************************************************************************
#define BUZZER_PIN 9
#define LED_PIN 8

// LCD defination
LiquidCrystal g_lcd(12, 11, 7, 6, 5, 4);
#define LCD_COLS 16
#define LCD_ROWS 2

// remote control global variables
volatile IRrecv g_irrecv(2);
volatile decode_results g_results;

// remote key defination
#define KEY_CH_MINUS  0xFFA25D
#define KEY_CH        0xFF629D
#define KEY_CH_PLUS   0xFFE21D

#define KEY_PREV      0xFF22DD
#define KEY_NEXT      0xFF02FD
#define KEY_PLAY      0xFFC23D

#define KEY_MINUS     0xFFE01F
#define KEY_PLUS      0xFFA857
#define KEY_EQ        0xFF906F

#define KEY_0         0xFF6897
#define KEY_100       0xFF9867
#define KEY_200       0xFFB04F

#define KEY_1         0xFF30CF
#define KEY_2         0xFF18E7
#define KEY_3         0xFF7A85

#define KEY_4         0xFF10EF
#define KEY_5         0xFF38C7
#define KEY_6         0xFF5AA5

#define KEY_7         0xFF42BD
#define KEY_8         0xFF4AB5
#define KEY_9         0xFF52AD

#define KEY_LAST      0xFFFFFFFF
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
#define DBG_MSG(msg) Serial.print(F("DBG: ")); Serial.println(msg)
#define DBG_MSG_FMT(msg, fmt) Serial.print(F("DBG: ")); Serial.println(msg, fmt)
#else
#define DBG_MSG(msg)
#define DBG_MSG_FMT(msg, fmt)
#endif

#ifdef DEBUG_ERR
#define DBG_ERR(msg) Serial.print(F("ERR: ")); Serial.println(msg)
#define DBG_ERR_FMT(msg, fmt) Serial.print(F("ERR: ")); Serial.println(msg, fmt)
#else
#define DBG_ERR(msg)
#define DBG_ERR_FMT(msg, fmt)
#endif

#define IRQ  // just for comment
//------------------------------------------------------------------------------


//******************************************************************************
// Morse data structure
//******************************************************************************
// offset
#define MTABLE_CHAR_OFFSET  0x20
// space charactor code
#define M_SPACE             0x00
// invalid charactor code
#define M_INVLD             0x80
// morse data table
//  example of the item data structure (charactor 'Q') :
//  =========================================================
//  | B | 1    | 1    | 0   | 1    | 1          | 0 | 0 | 0 |
//  |   | ^    | ^    | ^   | ^    | ^          |   |   |   |
//  |   | dash | dash | dot | dash | terminator |   |   |   |
//  =========================================================
const byte mTable[] = {
  M_SPACE,      // 0x20, SPACE
  M_INVLD,      // 0x21, ! |
  B01001010,    // 0x22, " | ・－・・－・
  M_INVLD,      // 0x23, # |
  M_INVLD,      // 0x24, $ |
  M_INVLD,      // 0x25, % |
  M_INVLD,      // 0x26, & |
  B01111010,    // 0x27, ' | ・－－－－・
  B10110100,    // 0x28, ( | －・－－・
  B10110110,    // 0x29, ) | －・－－・－
  M_INVLD,      // 0x2A, * |
  B01010100,    // 0x2B, + | ・－・－・
  B11001110,    // 0x2C, , | －－・・－－
  B10000110,    // 0x2D, - | －・・・・－
  B01010110,    // 0x2E, . | ・－・－・－
  B10010100,    // 0x2F, / | －・・－・
  B11111100,    // 0x30, 0 | －－－－－
  B01111100,    // 0x31, 1 | ・－－－－
  B00111100,    // 0x32, 2 | ・・－－－
  B00011100,    // 0x33, 3 | ・・・－－
  B00001100,    // 0x34, 4 | ・・・・－
  B00000100,    // 0x35, 5 | ・・・・・
  B10000100,    // 0x36, 6 | －・・・・
  B11000100,    // 0x37, 7 | －－・・・
  B11100100,    // 0x38, 8 | －－－・・
  B11110100,    // 0x39, 9 | －－－－・
  B11100010,    // 0x3A, : | －－－・・・
  M_INVLD,      // 0x3B, ; |
  M_INVLD,      // 0x3C, < |
  B10001100,    // 0x3D, = | －・・・－
  M_INVLD,      // 0x3E, > |
  B00110010,    // 0x3F, ? | ・・－－・・
  B01101010,    // 0x40, @ | ・－－・－・
  B01100000,    // 0x41, A | ・－
  B10001000,    // 0x42, B | －・・・
  B10101000,    // 0x43, C | －・－・
  B10010000,    // 0x44, D | －・・
  B01000000,    // 0x45, E | ・
  B00101000,    // 0x46, F | ・・－・
  B11010000,    // 0x47, G | －－・
  B00001000,    // 0x48, H | ・・・・
  B00100000,    // 0x49, I | ・・
  B01111000,    // 0x4A, J | ・－－－
  B10110000,    // 0x4B, K | －・－
  B01001000,    // 0x4C, L | ・－・・
  B11100000,    // 0x4D, M | －－
  B10100000,    // 0x4E, N | －・
  B11110000,    // 0x4F, O | －－－
  B01101000,    // 0x50, P | ・－－・
  B11011000,    // 0x51, Q | －－・－
  B01010000,    // 0x52, R | ・－・
  B00010000,    // 0x53, S | ・・・
  B11000000,    // 0x54, T | －
  B00110000,    // 0x55, U | ・・－
  B00011000,    // 0x56, V | ・・・－
  B01110000,    // 0x57, W | ・－－
  B10011000,    // 0x58, X | －・・－
  B10111000,    // 0x59, Y | －・－－
  B11001000,    // 0x5A, Z | －－・・
};
//------------------------------------------------------------------------------


//******************************************************************************
// Macros
//******************************************************************************
// default sound last time ( = one dot time)
#define DEFAULT_DURATION 80
// default sound frequency
#define DEFAULT_FREQUENCY 1000
// default delay time in the sequence play
#define DEFAULT_SEQ_DELAY 1000
// array length macro function
#define ARY_LEN(ary) (sizeof(ary)/sizeof(ary[0]))
//------------------------------------------------------------------------------


//******************************************************************************
// Data types
//******************************************************************************
// run status
enum RunStatus {
  RS_READY = 0,                   // do nothing
  RS_SEQUENCE = 1,                // play the morse code on sequence
  RS_RANDOM_ALL = 2,              // play the morse code randomly
  RS_RANDOM_ALPHA_NUM = 3,        // (only include alphabet and number)
  RS_RANDOM_ALPHA_ONLY = 4,       // (only include alphabet
  RS_CALLSIGN = 9,                // play my callsign JJ1SLR
};
// rundom number creator function pointer type defination
typedef byte (*RandomFuncPtr)();
//------------------------------------------------------------------------------


//******************************************************************************
// Global variables
//******************************************************************************
// jump buffer for longjmp
jmp_buf g_jmpBuf;
// sound last time ( = one dot time)
volatile unsigned long g_duration = DEFAULT_DURATION;
// sound frequency
volatile unsigned int g_frequency = DEFAULT_FREQUENCY;
// delay time in the sequence play
volatile unsigned int g_seqDelay = DEFAULT_SEQ_DELAY;
// current remote key function pointer
volatile void (*keyFunc)() = NULL;
// current run status
volatile RunStatus g_runStatus = RS_READY;
// run status changed flag
volatile bool g_bRunStatusChanged = false;
//------------------------------------------------------------------------------


//******************************************************************************
// Basic functions
//******************************************************************************
// Delay milliseconds, non-blocking function.
// It calls eventChecker() function in the delay loop.
void delayWithChk(unsigned int mils) {
  unsigned long startMillis = millis();
  while (millis() - startMillis < mils) {
    eventChecker();
  }
}

// Play tone with given frequency and duration time.
// It uses Arduino tone() function so the hardware time2 is used.
void playTone(int freq, int dura) {
  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, freq, dura);
  delayWithChk(dura);
  digitalWrite(LED_PIN, LOW);
}

// Play morse dot.
void playDot() {
  playTone(g_frequency, g_duration);
  delayWithChk(g_duration);
}

// Play morse dash
void playDash() {
  playTone(g_frequency, g_duration * 3);
  delayWithChk(g_duration);
}

// Play separator between dot/dash.
void playSep() {
  delayWithChk(g_duration * 2);
}

// Play separator between words.
void playWordSep() {
  delayWithChk(g_duration * 6);
}

// Get the bit value of the given byte data.
bool getBit(byte dat, byte i) {
  return (bool)((0x01 << (7 - i)) & dat);
}

// Get the valid bit length of the data.
// The data's bit structure is the same as an item of mTable.
byte getLen(byte dat) {
  byte i;
  for (i = 7; i > 0; --i) {
    if (getBit(dat, i)) {
      break;
    }
  }
  // It doesn't mind if the highest bit is 1 or 0 if the loop is not breaked.
  return i;
}
//------------------------------------------------------------------------------


//******************************************************************************
// Module functions
//******************************************************************************
// Play sound, and display on lcd if the disp parameter is true.
bool playChar(char ch, bool disp) {
  //  Serial.println(ch);
  byte ic;  // index of mTable
  // invalid char
  if (ch < MTABLE_CHAR_OFFSET || ch >= MTABLE_CHAR_OFFSET + ARY_LEN(mTable)) {
    DBG_ERR(F("playChar(): Invalid char!"));
    DBG_ERR_FMT(ch, HEX);
    return false;
  }
  // Caculate the index of mTable of the given charactor.
  ic = ch - MTABLE_CHAR_OFFSET;

  // space
  if (M_SPACE == mTable[ic]) {
    playWordSep();
    return true;
  }
  // chars not in Morse
  if (M_INVLD == mTable[ic]) {
    DBG_ERR(F("playChar(): Char not in Morse table!"));
    DBG_ERR_FMT(ch, HEX);
    return false;
  }

  // get the length of the item
  byte len = getLen(mTable[ic]);

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
    for (byte i = 0; i < len; ++i) {
      if (getBit(mTable[ic], i)) {
        g_lcd.write('-');
      } else {
        g_lcd.write('.');
      }
    }
  }

  // valid char
  for (byte i = 0; i < len; ++i) {
    if (getBit(mTable[ic], i)) {
      playDash();
    } else {
      playDot();
    }
  }
  // play a separator
  playSep();
  return true;
}

// Play sound with no display. (overload)
bool playChar(char ch) {
  return playChar(ch, false);
}

// Play callsign and display them on the LCD.
void playCQCallSign() {
  g_lcd.cursor();
  g_lcd.blink();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  char *pc = CQ_CALL_SIGN;
  byte len = strlen(pc);
  for (byte i = 0; i < len; ++i) {
    g_lcd.write(pc[i]);
    if (i == LCD_COLS - 1) {
      g_lcd.setCursor(0, 1);
    }
    playChar(pc[i]);
  }
}

// Play sequence and display them on the LCD,
// with the same order in the Japanese law.
void playSequence() {
  g_lcd.noCursor();
  g_lcd.noBlink();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  // play 'A'-'Z'
  for (char c = 'A'; c <= 'Z'; ++c) {
    playChar(c, true);
    playWordSep();
    delayWithChk(g_seqDelay);
  }
  // play '1'-'9'
  for (char c = '1'; c <= '9'; ++c) {
    playChar(c, true);
    playWordSep();
    delayWithChk(g_seqDelay);
  }
  playChar('0', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('.', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar(',', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar(':', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('?', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('\'', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('-', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('(', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar(')', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('/', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('=', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('+', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('"', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('X', true);
  playWordSep();
  delayWithChk(g_seqDelay);
  playChar('@', true);
  playWordSep();
  delayWithChk(g_seqDelay);
}

// Play "Ready." on the LCD.
void playReady() {
  g_lcd.noCursor();
  g_lcd.noBlink();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print(F("Ready."));
}
// Play ramdomly and display them on the LCD.
// It stops when the screen is full.
// The random data is generated by the given function randomFunc.
void playRandom(RandomFuncPtr randomFunc) {
  // Cread random seed
  randomSeed(analogRead(A0));
  // Clear lcd
  g_lcd.cursor();
  g_lcd.blink();
  g_lcd.clear();
  // play random chars until the LCD is full
  for (byte i = 0; i < LCD_ROWS; ++i) {
    // Set cursor position
    g_lcd.setCursor(0, i);
    for (byte j = 0; j < LCD_COLS; ++j) {
      byte ic = randomFunc();
      char ch = ic + MTABLE_CHAR_OFFSET;  // calculate the charactor
      // treat space or invalid charactor as word separator
      if (M_SPACE == mTable[ic] || M_INVLD == mTable[ic]) {
        playWordSep();
        // display a space on the LCD
        g_lcd.write(' ');
        continue;
      }
      // play the charactor
      playChar(ch);
      // display it on the LCD
      g_lcd.write(ch);
    }
  }
}

//*******************************
// Remote key handling
//*******************************
// Increase frequency when the key PLUS is pressed.
void IRQ onKeyPlus() {
  if (g_frequency < 2000) {
    g_frequency += 100;
  }
}

// Decrease frequency when the key MINUS is pressed.
void IRQ onKeyMinus() {
  if (g_frequency > 500) {
    g_frequency -= 100;
  }
}

// Set frequency to default when the key EQ is pressed.
void IRQ onKeyEq() {
  g_frequency = DEFAULT_FREQUENCY;
}

// Decrease duration when the key NEXT is pressed.
void IRQ onKeyNext() {
  if (g_duration > 40) {
    g_duration -= 5;
  }
}

// Increase duration when the key PREV is pressed.
void IRQ onKeyPrev() {
  if (g_duration < 120) {
    g_duration += 5;
  }
}

// Set duration to default when the key PLAY is pressed.
void IRQ onKeyPlay() {
  g_duration = DEFAULT_DURATION;
}

// Decrease delay when the key CHPLUS is pressed.
void IRQ onKeyChPlus() {
  if (g_seqDelay >= 100) {
    g_seqDelay -= 100;
  }
}

// Increase delay when the key CHMINUS is pressed.
void IRQ onKeyChMinus() {
  if (g_seqDelay < 2000) {
    g_seqDelay += 100;
  }
}

// Set delay to default when the key CH is pressed.
void IRQ onKeyCh() {
  g_seqDelay = DEFAULT_SEQ_DELAY;
}

// Change the mode to RS_READY when the key 0 is pressed.
void IRQ onKey0() {
  g_runStatus = RS_READY;
  g_bRunStatusChanged = true;
}

// Change the mode to RS_SEQUENCE when the key 1 is pressed.
void IRQ onKey1() {
  g_runStatus = RS_SEQUENCE;
  g_bRunStatusChanged = true;
}

// Change the mode to RS_RANDOM_ALL when the key 2 is pressed.
void IRQ onKey2() {
  g_runStatus = RS_RANDOM_ALL;
  g_bRunStatusChanged = true;
}

// Change the mode to RS_RANDOM_ALPHA_NUM when the key 3 is pressed.
void IRQ onKey3() {
  g_runStatus = RS_RANDOM_ALPHA_NUM;
  g_bRunStatusChanged = true;
}

// Change the mode to RS_RANDOM_ALPHA_ONLY when the key 4 is pressed.
void IRQ onKey4() {
  g_runStatus = RS_RANDOM_ALPHA_ONLY;
  g_bRunStatusChanged = true;
}

// Change the mode to RS_CALLSIGN when the key 9 is pressed.
void IRQ onKey9() {
  g_runStatus = RS_CALLSIGN;
  g_bRunStatusChanged = true;
}

// It will runs here when the remote control pressed.
// Distribute it to the event function.
void IRQ onKeyReceived(decode_results *results) {
  switch (results->value) {
    case KEY_PLUS:
      keyFunc = onKeyPlus;
      break;
    case KEY_MINUS:
      keyFunc = onKeyMinus;
      break;
    case KEY_EQ:
      keyFunc = onKeyEq;
      break;

    case KEY_NEXT:
      keyFunc = onKeyNext;
      break;
    case KEY_PREV:
      keyFunc = onKeyPrev;
      break;
    case KEY_PLAY:
      keyFunc = onKeyPlay;
      break;

    case KEY_CH_PLUS:
      keyFunc = onKeyChPlus;
      break;
    case KEY_CH_MINUS:
      keyFunc = onKeyChMinus;
      break;
    case KEY_CH:
      keyFunc = onKeyCh;
      break;

    case KEY_0:
      keyFunc = onKey0;
      break;
    case KEY_1:
      keyFunc = onKey1;
      break;
    case KEY_2:
      keyFunc = onKey2;
      break;
    case KEY_3:
      keyFunc = onKey3;
      break;
    case KEY_4:
      keyFunc = onKey4;
      break;
    case KEY_9:
      keyFunc = onKey9;
      break;

    case KEY_LAST:
      // do the same as the last pressed key
      break;
    default:
      // invalid key
      keyFunc = NULL;
      break;
  }
  if (keyFunc != NULL) {
    keyFunc();
  }
}

// This function is called by delayWithChk() function.
// When run status changed, it will do a long jump to the
// start of loop() function.
void eventChecker() {
  if (g_bRunStatusChanged) {
    DBG_MSG("eventChecker: status changed");
    g_bRunStatusChanged = false;
    longjmp(g_jmpBuf, 1);
  }
}

// Random number creator to create a random index of mTable.
byte randomFuncAll() {
  return random(ARY_LEN(mTable));  // create a random index
}

// Random number creator to create a number in the range of
// alphabet or number. It may also create 0 for the space charactor.
byte randomFuncAlphaNum() {
  byte idx = 0;  // index
  // create a random number (0 ~ 37)
  // 0 and 37 is used to add space
  byte num = random(38);  
  DBG_MSG("randomFuncAlphaNum");
  DBG_MSG(num);
  if (0 < num && num <= 10) {
    // the index of '0' is 0x10. 
    // e.g. when num is 1, idx will be 0x10
    idx = num + 0x0F;  
  } else if (10 < num && num <= 36) {
    // the index of 'A' is 0x21.
    // e.g. when num is 11, idx will be 0x21
    //      when num is 36, idx will be 0x3A
    idx = num + 0x16;  
  } else {
    // space here
    idx = 0;
  }
  return idx;
}

// Random number creator to create a number in the range of
// alphabet. It may also create 0 for the space charactor.
byte randomFuncAlphaOnly() {
  byte idx = 0;  // index
  // create a random number (0 ~ 27)
  // 0 and 27 is used to add space
  byte num = random(28);
  DBG_MSG("randomFuncAlphaOnly");
  DBG_MSG(num);
  if (0 < num && num <= 26) {
    // the index of 'A' is 0x21.
    // e.g. when num is 1, idx will be 0x21
    //      when num is 26, idx will be 0x3A
    idx = num + 0x20;  
  } else {
    // space here
    idx = 0;
  }
  return idx;
}
//------------------------------------------------------------------------------


//******************************************************************************
// Public functions
//******************************************************************************
// IRQ0 handler.
void IRQ ir0_handler() {
  if (g_irrecv.decode(&g_results)) {
    onKeyReceived(&g_results);
    g_irrecv.resume(); // Receive the next value
  }
}

// The setup() function of the Arduino framework.
void setup() {
  // start serial output
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
  // Attatch interrupt
  attachInterrupt(0, ir0_handler, CHANGE);
}

// The loop() function of the Arduino framework.
void loop() {
  setjmp(g_jmpBuf);
  switch (g_runStatus) {
    case RS_READY:
      playReady();
      break;
    case RS_SEQUENCE:
      playSequence();
      break;
    case RS_RANDOM_ALL:
      playRandom(randomFuncAll);
      break;
    case RS_RANDOM_ALPHA_NUM:
      playRandom(randomFuncAlphaNum);
      break;
    case RS_RANDOM_ALPHA_ONLY:
      playRandom(randomFuncAlphaOnly);
      break;
    case RS_CALLSIGN:
      playCQCallSign();
      break;
    default:
      break;
  }
  // wait for remote control input
  for (;;) {
    delayWithChk(1);
  }
}
//------------------------------------------------------------------------------
