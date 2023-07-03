#include <Arduino.h>
#include <Badge2020_TFT.h>
#include <Adafruit_GFX.h> 
#include "SPIFFS.h"

Badge2020_TFT tft;

// put function declarations here:
void initTFT(Badge2020_TFT &tft);
void printReadme();
void showBitmap();
void scroll(uint8_t pos);

void setup() {
  Serial.begin(115200);
  initTFT(tft);
  tft.setTextColor(ST77XX_WHITE);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //printReadme();
  showBitmap();
}

uint8_t scrollpos = 0;
void loop() {
  scroll(scrollpos);
  delay(10);
  scrollpos--;
  if (scrollpos > 240) scrollpos = 239;
}

void showBitmap()
{
  File bmp = SPIFFS.open("/grass.bmp");
  uint8_t buffer[2*240*10]; //enough for 1 line of pixels

  //first, read & ignore the BMP header:
  bmp.read((uint8_t *) buffer, 10);
  //last 4 bytes of the bmp header contain offset for pixel arreay:
  uint32_t offset = 0;
  bmp.readBytes((char *)&offset, 4);
  Serial.print("offset for pixel array = ");
  Serial.println(offset);
  //so, ignore everything until offset:
  bmp.seek(offset);

  for (uint16_t y=0; y<240; y+=10)
  {
    bmp.read((uint8_t *) buffer, 2*240*10);
    tft.drawRGBBitmap(0, y, (uint16_t*) buffer, 240, 10);
  }
  bmp.close();
}
void printReadme()
{
  File file = SPIFFS.open("/README.md"); //case sensitive
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  
  delay(3000);

  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

// put function definitions here:
void initTFT(Badge2020_TFT &tft) {
  tft.init(240, 240);
  tft.enableBacklight();
  tft.setRotation( 2 );
  // Anything from the Adafruit GFX library can go here, see
  // https://learn.adafruit.com/adafruit-gfx-graphics-library  
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 10);

  //send command to indicate that in future, we want to scroll a window of 240 lines (total screen)
  uint8_t verticalScrollParams[] = {
    0, 
    0, //no "stable" non-scrolling portion at top
    0,
    240, //240 lines in our display; all scrolling. 240 < 255 so fits in a single byte
    0,
    320-240 //chip has memory for 320 lines, but my display only has 240, so keep remaining lines non-scrolling
  };
  tft.sendCommand(0x33, verticalScrollParams, (uint8_t)6);

}

void scroll(uint8_t pos)
{
  uint8_t scrollparams[2] = {0, pos};
  tft.sendCommand(0x37, scrollparams, (uint8_t)2);
}
