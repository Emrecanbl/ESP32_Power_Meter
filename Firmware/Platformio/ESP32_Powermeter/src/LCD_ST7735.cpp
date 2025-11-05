#include <main.h>
#include <LCD_ST7735.h>

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

float p = 3.1415926;

// Numeric field positions (x,y,width) per channel and field
// field index: 0=V, 1=I, 2=P, 3=E
struct ValuePos { int16_t x; int16_t y; int16_t w; };
static ValuePos s_valPos[3][4];
static int16_t s_midX = 0;
static int16_t s_sectionH = 0;
static const uint16_t s_accent[3] = { ST77XX_RED, ST77XX_GREEN, ST77XX_CYAN };

void LCD_ST7735_init(){
  Serial.print(F("LCD_init_Started"));
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);  
  tft.setSPISpeed(160000000);
  Serial.println(F("Initialized"));
  tft.fillScreen(ST77XX_BLACK);
  testdrawtext("ESP32-POWERMETER", ST77XX_WHITE);
  delay(1000);
  // Draw static dashboard frame once (borders, labels)
  TFT_DrawStaticFrame();
}
void testdrawtext(const char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void drawChannelSection(int index, PowerSample &d, uint16_t accentColor){
  int16_t w = tft.width();
  int16_t h = tft.height();
  const int sections = 3;

  int16_t sectionH = h / sections;
  int16_t y0 = index * sectionH;
  int16_t y1 = y0 + sectionH - 1;
  int16_t midX = w / 2;

  tft.fillRect(0, y0, w, sectionH, ST77XX_BLACK);

  tft.drawLine(0, y0,     w - 1, y0,     ST77XX_BLUE);
  tft.drawLine(0, y1,     w - 1, y1,     ST77XX_BLUE);
  tft.drawLine(midX, y0+1, midX,  y1-1,  ST77XX_BLUE);

  tft.setTextSize(1);
  tft.setTextWrap(false);
  tft.setCursor(4, y0 + 2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("CH"); tft.print(index + 1);

  const int16_t leftX  = 6;
  const int16_t rightX = midX + 6;
  const int16_t lineH  = 12;
  int16_t ty = y0 + 16;

  // Wh/kWh seçimi
  float eVal = d.energyWh[index];
  const char* eUnit = "Wh";
  if (eVal >= 1000.0f) { eVal = eVal / 1000.0f; eUnit = "kWh"; }

  // Sol: V, I
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(leftX, ty);
  tft.print("V: ");
  tft.setTextColor(accentColor);
  tft.print(d.Voltage[index], 2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" V");

  ty += lineH;
  tft.setCursor(leftX, ty);
  tft.print("I: ");
  tft.setTextColor(accentColor);
  tft.print(d.Current[index], 3);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" A");

  // Sağ: P, E
  ty = y0 + 16;
  tft.setCursor(rightX, ty);
  tft.print("P: ");
  tft.setTextColor(accentColor);
  tft.print(d.Power[index], 1);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" W");

  ty += lineH;
  tft.setCursor(rightX, ty);
  tft.print("E: ");
  tft.setTextColor(accentColor);
  tft.print(eVal, 2);
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" ");
  tft.print(eUnit);
}

void TFT_Data_Show(PowerSample &d) {
  drawChannelSection(0, d, ST77XX_RED);
  drawChannelSection(1, d, ST77XX_GREEN);
  drawChannelSection(2, d, ST77XX_CYAN);
}

static inline void tftPrintFloat(Adafruit_ST7735 &t, double v, uint8_t digits) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%.*f", digits, v);
  t.print(buf);
}

// Render static labels, borders and cache numeric field rectangles.
void TFT_DrawStaticFrame() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(false);

  int16_t w = tft.width();
  int16_t h = tft.height();
  s_midX = w / 2;
  s_sectionH = h / 3;

  // Outer border
  tft.drawLine(0, 0,     w - 1, 0,     ST77XX_BLUE);
  tft.drawLine(0, h - 1, w - 1, h - 1, ST77XX_BLUE);
  tft.drawLine(0, 0,     0,     h - 1, ST77XX_BLUE);
  tft.drawLine(w - 1, 0, w - 1, h - 1, ST77XX_BLUE);

  for (int index = 0; index < 3; ++index) {
    int16_t y0 = index * s_sectionH;
    int16_t y1 = y0 + s_sectionH - 1;

    // Section borders
    tft.drawLine(0, y0,     w - 1, y0,     ST77XX_BLUE);
    tft.drawLine(0, y1,     w - 1, y1,     ST77XX_BLUE);
    tft.drawLine(s_midX, y0+1, s_midX,  y1-1,  ST77XX_BLUE);

    // Title
    tft.setCursor(4, y0 + 2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.print("CH"); tft.print(index + 1);

    // Layout constants
    const int16_t leftX  = 6;
    const int16_t rightX = s_midX + 6;
    const int16_t lineH  = 12;
    int16_t tyL = y0 + 16;
    int16_t tyR = y0 + 16;

    // Left column labels and units, cache value areas
    tft.setTextColor(ST77XX_WHITE);
    // V
    tft.setCursor(leftX, tyL); tft.print("V:");
    int16_t unitVL = s_midX - 14; tft.setCursor(unitVL, tyL); tft.print("V");
    s_valPos[index][0] = { (int16_t)(leftX + 18), tyL, (int16_t)(unitVL - (leftX + 18) - 1) };
    // I
    tyL += lineH;
    tft.setCursor(leftX, tyL); tft.print("I:");
    int16_t unitIL = s_midX - 14; tft.setCursor(unitIL, tyL); tft.print("A");
    s_valPos[index][1] = { (int16_t)(leftX + 18), tyL, (int16_t)(unitIL - (leftX + 18) - 1) };

    // Right column labels and units, cache value areas
    // P
    tft.setCursor(rightX, tyR); tft.print("P:");
    int16_t unitPR = w - 18; tft.setCursor(unitPR, tyR); tft.print("W");
    s_valPos[index][2] = { (int16_t)(rightX + 18), tyR, (int16_t)(unitPR - (rightX + 18) - 1) };
    // E unit label kept static as Wh for responsiveness
    tyR += lineH;
    tft.setCursor(rightX, tyR); tft.print("E:");
    int16_t unitER = w - 24; tft.setCursor(unitER, tyR); tft.print("Wh");
    s_valPos[index][3] = { (int16_t)(rightX + 18), tyR, (int16_t)(unitER - (rightX + 18) - 1) };
  }
}

// Update only numeric areas without redrawing labels/lines.
void TFT_UpdateValues(const PowerSample &d) {
  tft.setTextSize(1);
  tft.setTextWrap(false);

  for (int i = 0; i < 3; ++i) {
    uint16_t col = s_accent[i];

    // V
    const auto &pv = s_valPos[i][0];
    tft.fillRect(pv.x, pv.y - 1, pv.w, 10, ST77XX_BLACK);
    tft.setCursor(pv.x, pv.y); tft.setTextColor(col);
    tftPrintFloat(tft, d.Voltage[i], 2);

    // I
    const auto &pi = s_valPos[i][1];
    tft.fillRect(pi.x, pi.y - 1, pi.w, 10, ST77XX_BLACK);
    tft.setCursor(pi.x, pi.y); tft.setTextColor(col);
    tftPrintFloat(tft, d.Current[i], 3);

    // P
    const auto &pp = s_valPos[i][2];
    tft.fillRect(pp.x, pp.y - 1, pp.w, 10, ST77XX_BLACK);
    tft.setCursor(pp.x, pp.y); tft.setTextColor(col);
    tftPrintFloat(tft, d.Power[i], 1);

    // E in Wh (unit label is static)
    const auto &pe = s_valPos[i][3];
    tft.fillRect(pe.x, pe.y - 1, pe.w, 10, ST77XX_BLACK);
    tft.setCursor(pe.x, pe.y); tft.setTextColor(col);
    tftPrintFloat(tft, d.energyWh[i], 3);
  }
  // restore default text color if needed
  tft.setTextColor(ST77XX_WHITE);
}
