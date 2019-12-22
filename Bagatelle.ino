// Ryan Burnside 2019
// Scoring system for physical bagatelle game

#include <LiquidCrystal.h>

// Pin constants
const unsigned char SHIFT_PIN = 2;
const unsigned char CLOCK_PIN = 3;
const unsigned char SERIAL_INPUT_PIN = 4;
const unsigned char CLOCK_INHIBIT_PIN = 5;

const unsigned char LCD_REGISTER_SELECT = 6;
const unsigned char LCD_READWRITE_PIN = 7;
const unsigned char LCD_ENABLE_PIN = 8;

const unsigned char LCD_DATA_4 = 15;
const unsigned char LCD_DATA_5 = 14;
const unsigned char LCD_DATA_6 = 16;
const unsigned char LCD_DATA_7 = 10;

class BallSwitch
// Each cycle feed it the switch value, it will ignore new inputs until after the switch is read open and x millisecs have passed
{
  public:
    unsigned int WAIT_TICKS = 100;
    unsigned int ticks_remaining = 0;
    byte point_value = 0;

    byte update(bool read_value)
    // Updates the internals and returns a point value if need be.
    {
      if(ticks_remaining == 0 && last_read_value == LOW && read_value == HIGH)
      { 
        // Reset counter
        ticks_remaining = WAIT_TICKS;
        last_read_value = read_value;
        return point_value;
      }
      
      if (ticks_remaining > 0)
      {
        // Reduce
        ticks_remaining -= 1;
      }
      
      last_read_value = read_value;
      return 0;
    }
    
  private:
    bool last_read_value = HIGH;
};

// Global instance for 2x16 LCD display
LiquidCrystal lcd(LCD_REGISTER_SELECT, LCD_ENABLE_PIN, LCD_DATA_4, LCD_DATA_5, LCD_DATA_6, LCD_DATA_7);

// Global score variable
unsigned int score = 0;

// This is a buffer to hold our 8 inputs
bool buf[8] = {0,0,0,0,0,0,0,0};

// This is a buffer to hold our 8 BallSwitches
BallSwitch switches[8];

void update_lcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.setCursor(0, 1);
  lcd.print(score);
}

void setup() 
{
  // Shift register stuff
  pinMode(SHIFT_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(CLOCK_INHIBIT_PIN, OUTPUT);
  pinMode(SERIAL_INPUT_PIN, INPUT);
  
  // CLKIHB low, CLK low, SL/_LD high 
  digitalWrite(CLOCK_INHIBIT_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(SHIFT_PIN, HIGH);

  // LCD
  pinMode(LCD_REGISTER_SELECT, OUTPUT);
  pinMode(LCD_READWRITE_PIN, OUTPUT);
  pinMode(LCD_ENABLE_PIN, OUTPUT);

  pinMode(LCD_DATA_4, OUTPUT);
  pinMode(LCD_DATA_5, OUTPUT);
  pinMode(LCD_DATA_6, OUTPUT);
  pinMode(LCD_DATA_7, OUTPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  // Priming score display
  update_lcd();

  unsigned char point_table[8] = {30, 27, 24, 21, 19, 16, 13, 10};
  
  for(char i = 0; i < 8; ++i)
  {
    switches[i].WAIT_TICKS = 1000; // Heavy delay (ball bearings bounce!)
    switches[i].point_value = point_table[i];
  }
}

void poll_inputs()
{
  // We collect the data from an 8 bit shift register below...
  // Follow Figure 4 closer. - psilord
 
  // prepare the clock for when we want to read.
  digitalWrite(CLOCK_INHIBIT_PIN, HIGH);
  digitalWrite(CLOCK_PIN, LOW);
  
  // CLK is inhibited, so we don't need to pulse it.
  digitalWrite(SHIFT_PIN, LOW);
  digitalWrite(SHIFT_PIN, HIGH);
  
  // allow the clock to function.
  digitalWrite(CLOCK_INHIBIT_PIN, LOW);
  for(unsigned char i = 0; i < 8; ++i)
  {
    // Read the newly shifted bit
    buf[7 - i] = digitalRead(SERIAL_INPUT_PIN);
    // pulse the clock....
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CLOCK_PIN, LOW);
  }
}

void loop() 
{
  poll_inputs();

  for(unsigned int i = 0; i < 8; ++i)
  {
    unsigned int old_score = score;
    score += switches[i].update(buf[i]);
    if(old_score != score){update_lcd();}
  }
  
  delay(1); // 1 Millisec resolution for the BallSwitch array.
}
