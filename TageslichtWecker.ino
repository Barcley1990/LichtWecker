// include the library code:
#include "include.h"
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

// initialize the librarys
LiquidCrystal lcd(DISPRS, DISPEN, DISPD4, DISPD5, DISPD6, DISPD7);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);

typedef enum
{
  Format24 = 0,
  Format12 = 1
}hourFormatType;

typedef enum 
{
  AM = 0,
  PM = 1
}timeFormatType;

typedef enum
{
  Montag      = 1,
  Dienstag    = 2,
  Mittwoch    = 3,
  Donnerstag  = 4,
  Freitag     = 5,
  Samstag     = 6,
  Sonntag     = 7
}wkdayType;

typedef enum
{
  Jan = 1,
  Feb = 2,
  Mar = 3,
  Apr = 4,
  Mai = 5,
  Jun = 6,
  Jul = 7,
  Aug = 8,
  Sep = 9,
  Okt = 10,
  Nov = 11,
  Dez = 12
}monthType;

typedef enum
{
  ON,
  OFF
} LED_State;

struct
{
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  wkdayType wkday;
  uint8_t day;
  uint8_t month;
  uint16_t year;
  hourFormatType hourFormat;
  timeFormatType timeFormat;
}timeData;

struct 
{
  
}alarmData;

/* Static time variables */
static uint8_t clockHours, clockMin, clockSec;
static uint16_t dateDay = 1, dateMonth = 1, dateYear = 2018;
static uint8_t alarmHours, alarmMin;

/* Global variables */
boolean flagTaskCyclic100ms = FALSE;
boolean flagTaskCyclic10ms = FALSE;
uint8_t menuInc, menuDec;
uint8_t buttonStates = 0;
uint16_t menuID = 0;
uint16_t menuSelector = 0;
char displayTextBuffer[LCD_COL];
boolean LedMode1Flag;
boolean LedMode2Flag;
uint8_t timeOutCnt = 0;

/* Function declaration */
void switchAllLeds(LED_State state);
void mcp7940nReadTimeAndDate(uint8_t *hour, uint8_t *minute, uint8_t *sec, wkdayType *wkday, uint8_t *day, uint8_t *month, uint16_t *year);
void mcp7940nWriteTimeAndDate(uint8_t hour, uint8_t minute, uint8_t sec, wkdayType wkday, uint8_t day, uint8_t month, uint16_t year, hourFormatType hourFormat=Format24, timeFormatType timeFormat=AM);
void mcp7940nDisableExtCrystal(void);
void mcp7940nEnableExtCrystal(void);

void setup()
{
  DDRC |= (1<<0);
  PORTC &= ~(1<<PC0); 
  
  /* Setup external libraries */
  lcd.begin(LCD_COL, LCD_ROW);
  PORTC |= (1<<PC0); 
  pixels.begin();
  switchAllLeds(OFF);
  pixels.setBrightness(20);

  Serial.begin(9600);
  Wire.begin(MCP7940N_ADDRES);

  /* TODO: For Testing... */
  mcp7940nWriteTimeAndDate(23,52,59,Dienstag,22,9,2018);

  /* Setup I/Os */
  pinMode(BUTTON_0, INPUT);
  pinMode(BUTTON_1, INPUT);

  // Setup Timer2
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A = (1 << WGM21);
  TCCR2B = (1 << CS20) | (1 << CS21) | (1 << CS22);
  TIMSK2 = (1 << OCIE2A);
  OCR2A  = 157;

  // Setup Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  TIMSK1 = (1 << OCIE1A);
  OCR1A  = 1600;

  sei();
}

void loop()
{
  /* Handle 100ms Task */
  if (flagTaskCyclic100ms == TRUE)
  {
    TaskCyclic100ms();
    flagTaskCyclic100ms = FALSE;
  }
  /* Handle 10ms Task */
  if (flagTaskCyclic10ms == TRUE)
  {
    TaskCyclic10ms();
    flagTaskCyclic10ms = FALSE;
  }
  handleButtons();
}

void TaskCyclic100ms()
{
  if(menuID==UIMENUE_ID_CLOCK)
  {
    uiClock();
  }
  else
  {
    timeOutCnt++;
    if(timeOutCnt >= 100) // 10s
    {
      lcd.clear();
      timeOutCnt = 0;
      menuID=UIMENUE_ID_CLOCK;
    }
  }
}

void TaskCyclic10ms()
{
  static uint16_t i = 0, j = 0;
  static uint8_t hight = 0;

  /* rainbow */
  if (LedMode2Flag)
  {
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    j++;
    if (j > 256) j = 0;
  }
  /* volcano */
  if(LedMode1Flag)
  {  
    
    if(i<hight)
    {
      pixels.setPixelColor(i, RED);
      pixels.show();
      i++;
    }
    else if(i>0)
    {
      hight = 0;
      pixels.setPixelColor(i, 0,0,0);
      pixels.show();
      i--;  
    }
    else
    {
      hight = random(NUMPIXELS);
    }
  }

}


void updateDisplay()
{
  lcd.clear();

  /* Set cursor only if not in clock menu */
  if (menuID != UIMENUE_ID_CLOCK)
    uiSetSelector(menuSelector);

  switch (menuID)
  {
    /* Main menues */
    case UIMENUE_ID_CLOCK:  uiClock(); break;
    case UIMENUE_ID_HOME:   uiShowMenu("Licht", "Zeit", "Alarm", "<--"); break;
    case UIMENUE_ID_LIGHT:  uiShowMenu("Ein  ", "Aus ", "Mode1", "Mode2"); break;
    case UIMENUE_ID_TIME:   uiShowMenu("Set  ", "    ", "     ", "<--"); break;
    case UIMENUE_ID_ALARM:  uiShowMenu("     ", "    ", "     ", "<--"); break;
    /* Time and Date menues */
    case UISUBMENUE_ID_SETHOUR:   uiShowMenu("Hour: ", itoa(clockHours, displayTextBuffer, 10), "", ""); break;
    case UISUBMENUE_ID_SETMIN:    uiShowMenu("Min:  ", itoa(clockMin, displayTextBuffer, 10), "", "");   break;
    case UISUBMENUE_ID_SETSEC:    uiShowMenu("Sec:  ", itoa(clockSec, displayTextBuffer, 10), "", "");   break;
    case UISUBMENUE_ID_SETYEAR:   uiShowMenu("Year: ", itoa(dateYear, displayTextBuffer, 10), "", "");   break;
    case UISUBMENUE_ID_SETMONTH:  uiShowMenu("Month:", itoa(dateMonth, displayTextBuffer, 10), "", "");  break;
    case UISUBMENUE_ID_SETDAY:    uiShowMenu("Day:  ", itoa(dateDay, displayTextBuffer, 10), "", "");    break;
    case UISUBMENUE_ID_SAFEDATE:  mcp7940nWriteTimeAndDate(clockHours,clockMin,clockSec,Montag,dateDay,dateMonth,dateYear); menuID = UIMENUE_ID_CLOCK; break;
    /* Light menues */
    case UISUBMENUE_ID_LIGHTON:  switchAllLeds(ON); menuID = UIMENUE_ID_CLOCK; break;
    case UISUBMENUE_ID_LIGHTOFF: switchAllLeds(OFF); LedMode1Flag = FALSE; LedMode2Flag = FALSE; menuID = UIMENUE_ID_CLOCK; break;
    case UISUBMENUE_ID_MODE1: LedMode1Flag = TRUE; LedMode2Flag = FALSE; menuID = UIMENUE_ID_CLOCK; break;
    case UISUBMENUE_ID_MODE2: LedMode2Flag = TRUE; LedMode1Flag = FALSE; menuID = UIMENUE_ID_CLOCK; break;
    default: menuID = UIMENUE_ID_CLOCK; break;
  }
}

/*************************************************************/
/**********************HANDLE BUTTONS*************************/
/*************************************************************/
void handleButtons()
{
    /* Check button for toggle further */
  if (isButtonToggled(NEXTBUTTON) == TRUE)
  {
    /* Only for setting the time */
    if (menuID == UISUBMENUE_ID_SETHOUR)
    {
      incHour();
    }
    else if (menuID == UISUBMENUE_ID_SETMIN)
    {
      incMin();
    }
    else if (menuID == UISUBMENUE_ID_SETSEC)
    {
      incSec();
    }
    else if(menuID == UISUBMENUE_ID_SETYEAR)
    {
      incYear();
    }
    else if(menuID == UISUBMENUE_ID_SETMONTH)
    {
      incMonth();
    }
    else if(menuID == UISUBMENUE_ID_SETDAY)
    {
      incDay();
    }
    else
    {
      /* First entry from clock to main menu? */
      if (menuID == 0)
      {
        menuID = UIMENUE_ID_HOME;
        uiShowMenu("Licht", "Zeit", "Alarm", "<--");
      }
      /* ...else increment selector  */
      else
      {
        menuSelector++;
        if (menuSelector > 3)
          menuSelector = 0;
      }
    }
    //Serial.print("SelectorID: "); Serial.println(menuSelector);
    timeOutCnt = 0; // reset timeout counter
    updateDisplay();
  }


  /* Check button for selection */
  if (isButtonToggled(SELECTBUTTON) == TRUE)
  {
    /* Main Menu ? */
    if (menuID % UIMENUE_ID_HOME == 0)
    {
      menuInc = 0x10;
      menuDec = menuID % UIMENUE_ID_HOME;
    }
    /* Or SubMenu? */
    else
    {
      menuInc = 0x1;
      menuDec = menuID % UIMENUE_ID_HOME;
    }
    //Serial.print("MenuID: "); Serial.println(menuID, HEX);
    switch (menuSelector)
    {
      case 0: menuID += menuInc * 0x1; break;
      case 1: menuID += menuInc * 0x2; break;
      case 2: menuID += menuInc * 0x3; break;
      case 3: menuID -= menuDec * 0x1; break;
      default: break;
    }
    /* Reset Menu Selector*/
    menuSelector = 0;
    timeOutCnt = 0; // reset timeout counter
    updateDisplay();
  }
}

/*************************************************************/
/**********************DISPLAY FUNCTIONS**********************/
/*************************************************************/
void incHour()
{
  clockHours++;
  if (clockHours >= 24)
    clockHours = 0;
}
void incMin()
{
  clockMin++;
  if (clockMin >= 60)
    clockMin = 0;
}
void incSec()
{
  clockSec++;
  if (clockSec >= 60)
    clockSec = 0;
}
void incYear()
{
  dateYear++;
  if (dateYear >= 2099)
    dateYear = 2000;
}
void incMonth()
{
  dateMonth++;
  if (dateMonth >= 13)
    dateMonth = 1;
}
void incDay()
{
  dateDay++;
  if (dateDay >= 32)
    dateDay = 1;
}

void uiClock()
{
  static uint8_t tmp;
  static uint8_t loopCnt = 0;

  /* Update Time Data via I2C */
  mcp7940nReadTimeAndDate(&timeData.hour, 
                          &timeData.minute, 
                          &timeData.second,
                          &timeData.wkday, 
                          &timeData.day, 
                          &timeData.month, 
                          &timeData.year);
                          
  loopCnt++;
  if (loopCnt == 10)
  {
    tmp ^= 0x1;
    loopCnt = 0;
  }
  /* Print Time */
  lcd.setCursor(4, 0);
  lcd.print(itoa(timeData.hour, displayTextBuffer, 10));
  if (tmp)
    lcd.print(":");
  else
    lcd.print(" ");
  lcd.print(itoa(timeData.minute, displayTextBuffer, 10));
  lcd.print(" Uhr");

  /* Print Date */
  lcd.setCursor(3, 1);
  lcd.print(itoa(timeData.day, displayTextBuffer, 10));
  lcd.print(".");
  switch(timeData.month)
  {
    case 1: lcd.print("Jan"); break;
    case 2: lcd.print("Feb"); break;
    case 3: lcd.print("Mar"); break;
    case 4: lcd.print("Apr"); break;
    case 5: lcd.print("Mai"); break;
    case 6: lcd.print("Jun"); break;
    case 7: lcd.print("Jul"); break;
    case 8: lcd.print("Aug"); break;
    case 9: lcd.print("Sep"); break;
    case 10: lcd.print("Okt"); break;
    case 11: lcd.print("Nov"); break;
    case 12: lcd.print("Dez"); break;
  }
  lcd.print(".");
  lcd.print("20");
  lcd.print(itoa(timeData.year, displayTextBuffer, 10));
}

void uiShowMenu(const char subMenu0[6], const char subMenu1[6], const char subMenu2[6], const char subMenu3[6])
{
  lcd.setCursor(LCD_UIPOS1_COL + 1, LCD_UIPOS1_ROW);
  lcd.print(subMenu0);
  lcd.setCursor(LCD_UIPOS2_COL + 1, LCD_UIPOS2_ROW);
  lcd.print(subMenu1);
  lcd.setCursor(LCD_UIPOS3_COL + 1, LCD_UIPOS3_ROW);
  lcd.print(subMenu2);
  lcd.setCursor(LCD_UIPOS4_COL + 1, LCD_UIPOS4_ROW);
  lcd.print(subMenu3);
}

void uiSetSelector(uint8_t pos)
{
  switch (pos)
  {
    case 0: lcd.setCursor(LCD_UIPOS1_COL, LCD_UIPOS1_ROW); break;
    case 1: lcd.setCursor(LCD_UIPOS2_COL, LCD_UIPOS2_ROW); break;
    case 2: lcd.setCursor(LCD_UIPOS3_COL, LCD_UIPOS3_ROW); break;
    case 3: lcd.setCursor(LCD_UIPOS4_COL, LCD_UIPOS4_ROW); break;
    default: lcd.setCursor(LCD_UIPOS1_COL, LCD_UIPOS1_ROW); break;
  }
  lcd.print(">");
}

void uiClearSelector(int8_t pos)
{
  if (pos < 0)
    pos = 4;
  switch (pos)
  {
    case 0: lcd.setCursor(LCD_UIPOS1_COL, LCD_UIPOS1_ROW); break;
    case 1: lcd.setCursor(LCD_UIPOS2_COL, LCD_UIPOS2_ROW); break;
    case 2: lcd.setCursor(LCD_UIPOS3_COL, LCD_UIPOS3_ROW); break;
    case 3: lcd.setCursor(LCD_UIPOS4_COL, LCD_UIPOS4_ROW); break;
    default: lcd.setCursor(LCD_UIPOS1_COL, LCD_UIPOS1_ROW); break;
  }
  lcd.print(" ");
}

uint8_t isButtonPressed(uint8_t buttonNum)
{
  uint8_t ret;
  if ((buttonStates & (1 << buttonNum)))
  {
    ret = TRUE;
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

uint8_t isButtonReleased(uint8_t buttonNum)
{
  uint8_t ret;
  if ((buttonStates & (1 << buttonNum)) == FALSE)
  {
    ret = TRUE;
  }
  else
  {
    ret = FALSE;
  }
  return ret;
}

uint8_t isButtonToggled(uint8_t buttonNum)
{
  uint8_t ret = FALSE;
  if ((buttonStates & (1 << (buttonNum + 4))))
  {
    buttonStates &= ~(1 << (buttonNum + 4));
    ret = TRUE;
  }
  return ret;
}


/*************************************************************/
/**********************LED FUNCTIONS**************************/
/*************************************************************/
void switchAllLeds(LED_State state)
{
  for (uint8_t i = 0; i < NUMPIXELS; i++)
  {
    if (state == ON)
      pixels.setPixelColor(i, RED);
    else
      pixels.setPixelColor(i, 0, 0, 0);
  }
  pixels.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



/*************************************************************/
/**********************INTERRUPTS*****************************/
/*************************************************************/
ISR(TIMER2_COMPA_vect)
{
  uint8_t buttonState = 1;
  uint8_t buttonNum;

  /* Readout buttons */
  for (buttonNum = 0; buttonNum < 2; buttonNum++)
  {
    switch (buttonNum)
    {
      case 0: buttonState = digitalRead(BUTTON_0); break;
      case 1: buttonState = digitalRead(BUTTON_1); break;
    }
    if (buttonState == FALSE) // Button is pressed
    {
      /* Set Bit */
      buttonStates |= (1 << buttonNum);
    }
    else
    {
      /* Check for toggle: Was button set before? */
      if ((buttonStates & (1 << buttonNum)))
      {
        /* Set toggle flag */
        buttonStates |= (1 << (buttonNum + 4));
      }
      /* Clear Bit */
      buttonStates &= ~(1 << buttonNum);
    }
  }


  flagTaskCyclic10ms = TRUE;
}

ISR(TIMER1_COMPA_vect)
{
  flagTaskCyclic100ms = TRUE;
}
