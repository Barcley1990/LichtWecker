#define TRUE          0x1
#define FALSE         0x0

/* MCP7940N Macros */
#define MCP7940N_ADDRES           0x6F
/* Timekeeping */
#define MCP7940N_RTCSEC_REG       0x00
#define MCP7940N_RTCMIN_REG       0x01
#define MCP7940N_RTCHOUR_REG      0x02
#define MCP7940N_RTCWKDAY_REG     0x03
#define MCP7940N_RTCDATE_REG      0x04
#define MCP7940N_RTCMTH_REG       0x05
#define MCP7940N_RTCYEAR_REG      0x06
#define MCP7940N_CONTROL_REG      0x07
#define MCP7940N_OSCTRIM_REG      0x08

#define OSC_DISABLE_MASK  0x0
#define OSC_ENABLE_MASK   0x128

/* Arduino Pins */
#define BUTTON_0      7
#define BUTTON_1      8
#define NEOPIXELPIN   10
#define DISPRS        12
#define DISPEN        11
#define DISPD4        6
#define DISPD5        5
#define DISPD6        4
#define DISPD7        3

/* LED Macros */
#define NUMPIXELS     192
#define RED     Adafruit_NeoPixel::Color(255, 0, 0)
#define GREEN  Adafruit_NeoPixel::Color(0, 255, 0)
#define BLUE  Adafruit_NeoPixel::Color(0, 0, 255)
#define LIGHTBLUE  Adafruit_NeoPixel::Color(0, 155, 155)
#define PINK  Adafruit_NeoPixel::Color(155, 0, 255)
#define YELLOW  Adafruit_NeoPixel::Color(155, 155, 0)
#define ORANGE  Adafruit_NeoPixel::Color(200, 80, 0)
#define WHITE Adafruit_NeoPixel::Color(200, 255, 255)

/* LCD macros */
#define LCD_COL         16
#define LCD_ROW         2
#define LCD_UIPOS1_COL  0
#define LCD_UIPOS1_ROW  0
#define LCD_UIPOS2_COL  8
#define LCD_UIPOS2_ROW  0
#define LCD_UIPOS3_COL  0
#define LCD_UIPOS3_ROW  1
#define LCD_UIPOS4_COL  8
#define LCD_UIPOS4_ROW  1

/* Button Macros */
#define NEXTBUTTON      1
#define SELECTBUTTON    0

/* Menu IDs */
#define UIMENUE_ID_CLOCK        0x000
#define UIMENUE_ID_HOME         0x100

#define UIMENUE_ID_LIGHT        0x110
#define UISUBMENUE_ID_LIGHTON   0x111
#define UISUBMENUE_ID_LIGHTOFF  0x112
#define UISUBMENUE_ID_MODE1     0x113
#define UISUBMENUE_ID_MODE2     0x114

#define UIMENUE_ID_TIME         0x120
#define UISUBMENUE_ID_SETHOUR   0x121
#define UISUBMENUE_ID_SETMIN    0x122
#define UISUBMENUE_ID_SETSEC    0x123
#define UISUBMENUE_ID_SETYEAR   0x124
#define UISUBMENUE_ID_SETMONTH  0x125
#define UISUBMENUE_ID_SETDAY    0x126
#define UISUBMENUE_ID_SAFEDATE  0x127

#define UIMENUE_ID_ALARM        0x130
#define UISUBMENUE_ID_SETALHOUR 0x131
#define UISUBMENUE_ID_SETALMIN  0x132
