


void mcp7940nReadTimeAndDate(uint8_t *hour, 
                             uint8_t *minute, 
                             uint8_t *sec, 
                             wkdayType *wkday, 
                             uint8_t *day, 
                             uint8_t *month, 
                             uint16_t *year)
{
  uint8_t temp1, temp2;
  Wire.beginTransmission(MCP7940N_ADDRES);
  Wire.write(MCP7940N_RTCSEC_REG);
  Wire.endTransmission();
  
  Wire.requestFrom(MCP7940N_ADDRES,7);
  while(Wire.available() < 7);
  /* Time data */
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *sec    = temp2 +((temp1>>4)&0x7)*10;
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *minute = temp2 + ((temp1>>4)&0x7)*10;
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *hour   = temp2 + ((temp1>>4)&0x3)*10;

  /* Date data */
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0x7;
  *wkday  = (wkdayType)temp2;
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *day    = temp2 +((temp1>>4)&0x3)*10;
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *month  = temp2 +((temp1>>4)&0x1)*10;
  temp1   = (uint8_t)Wire.read();
  temp2   = temp1&0xF;
  *year   = temp2 +((temp1>>4)&0xF)*10;
}

void mcp7940nWriteTimeAndDate(uint8_t hour, 
                              uint8_t minute, 
                              uint8_t sec, 
                              wkdayType wkday, 
                              uint8_t day, 
                              uint8_t month, 
                              uint16_t year,
                              hourFormatType hourFormat, 
                              timeFormatType timeFormat)
{
  uint8_t LPYR = FALSE;
  if((year % 4) == 0)
    LPYR = TRUE;
    
  sec     = (((sec/10)&0x7)<<4) | (((sec%10)&0xF)<<0); 
  minute  = (((minute/10)&0x7)<<4) | (((minute%10)&0xF)<<0);
  hour    = ((hourFormat<<6)&0x1) | ((timeFormat<<5)&0x1) | (((hour/10)&0x3)<<4) | (((hour%10)&0xF)<<0);
  wkday   = (wkdayType)(((wkday%10)&0x7)<<0);
  day     = (((day/10)&0x3)<<4) | (((day%10)&0xF)<<0);
  month   = ((LPYR&0x1)<<5) | (((month/10)&0x1)<<4) | (((month%10)&0xF)<<0);
  year    = (((year/10)&0x7)<<4) | (((year%10)&0xF)<<0);
  
  //mcp7940nDiableExtCrystal();
  Wire.beginTransmission(MCP7940N_ADDRES);
  Wire.write(MCP7940N_RTCSEC_REG);
  Wire.write(sec);
  Wire.write(minute);
  Wire.write(hour);
  Wire.write(wkday);
  Wire.write(day);
  Wire.write(month);
  Wire.write(year);
  Wire.endTransmission();
  //mcp7940nEnableExtCrystal();
}

void mcp7940nDiableExtCrystal()
{
  uint8_t oscRunFlag = TRUE;
  
  /* Set ST Bit to zero */
  Wire.beginTransmission(MCP7940N_ADDRES);
  Wire.write(MCP7940N_RTCSEC_REG);
  Wire.write(OSC_DISABLE_MASK);
  Wire.endTransmission();

  /* Await OSCRUN flag to be cleared */
  while(oscRunFlag)
  {
    Wire.beginTransmission(MCP7940N_ADDRES);
    Wire.write(MCP7940N_RTCWKDAY_REG);
    Wire.endTransmission(); 
      
    Wire.requestFrom(MCP7940N_ADDRES,1);
    while(Wire.available() < 1)
    oscRunFlag = ((uint8_t)Wire.read()>>5)&0x1;
  }
}

void mcp7940nEnableExtCrystal()
{
  uint8_t oscRunFlag = FALSE;
  
  /* Set ST Bit to zero */
  Wire.beginTransmission(MCP7940N_ADDRES);
  Wire.write(MCP7940N_RTCSEC_REG);
  Wire.write(OSC_ENABLE_MASK);
  Wire.endTransmission();

  /* Await OSCRUN flag to be cleared */
  while(!oscRunFlag)
  {
    Wire.beginTransmission(MCP7940N_ADDRES);
    Wire.write(MCP7940N_RTCWKDAY_REG);
    Wire.endTransmission(); 
      
    Wire.requestFrom(MCP7940N_ADDRES,1);
    while(Wire.available() < 1)
    oscRunFlag = ((uint8_t)Wire.read()>>5)&0x1;
  }
}