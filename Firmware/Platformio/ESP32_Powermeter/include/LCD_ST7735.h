#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// Pin Definations
#define TFT_CS        5
#define TFT_RST        7 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8
#define TFT_MOSI 13  // Data out
#define TFT_SCLK 14  // Clock out

void LCD_ST7735_init();
void testlines(uint16_t color);
void testdrawtext(const char *text, uint16_t color);
// Static frame (labels, lines) once; later only numbers update
void TFT_DrawStaticFrame();
void TFT_UpdateValues(const PowerSample &d);
// Legacy per-section and full redraw (kept for compatibility)
void TFT_Data_Show( PowerSample &d);
