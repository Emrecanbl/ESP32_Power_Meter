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

static inline int16_t textWidthPx(const String &s, uint8_t size){
  return (int16_t)(s.length() * 6 * size);
}

static void TFT_DrawSplash(){
  int16_t w = tft.width();
  int16_t h = tft.height();
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  // Title centered (slightly smaller)
  const uint8_t titleSize = 1;
  String title = F("ESP32-POWERMETER");
  int16_t tx = (w - textWidthPx(title, titleSize)) / 2; if (tx < 2) tx = 2;
  tft.setTextSize(titleSize);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(tx, 8);
  tft.print(title);

  // Divider
  int16_t yDiv = 8 + (8 * titleSize) + 4; // title height + margin
  tft.drawLine(4, yDiv, w - 4, yDiv, ST77XX_BLUE);

  // Lines layout (one step smaller)
  const uint8_t labelSize = 1;
  const uint8_t valueSize = 1;
  const int16_t charH = 8; // base char height
  const int16_t lineH = (int16_t)(charH * valueSize + 4);
  int16_t y = yDiv + 8;

  // Sensor line
  {
    String lbl = F("Sensor");
    String val = Check.Sensor_Started ? F("OK") : F("FAIL");
    uint16_t vcol = Check.Sensor_Started ? ST77XX_GREEN : ST77XX_RED;
    tft.setTextSize(labelSize);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(6, y);
    tft.print(lbl);
    // value right-aligned
    int16_t vx = w - textWidthPx(val, valueSize) - 6;
    if (vx < 6) vx = 6;
    tft.setTextSize(valueSize);
    tft.setTextColor(vcol);
    tft.setCursor(vx, y);
    tft.print(val);
  }
  y += lineH;

  // WiFi line
  {
    String lbl = F("WiFi");
    String val = Check.Wifi_Connected ? F("Connected") : F("Not Conn.");
    uint16_t vcol = Check.Wifi_Connected ? ST77XX_GREEN : ST77XX_RED;
    tft.setTextSize(labelSize);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(6, y);
    tft.print(lbl);
    int16_t vx = w - textWidthPx(val, valueSize) - 6;
    if (vx < 6) vx = 6;
    tft.setTextSize(valueSize);
    tft.setTextColor(vcol);
    tft.setCursor(vx, y);
    tft.print(val);
  }
  y += lineH;

  // IP line (smaller size to fit long IPv4/IPv6)
  {
    String lbl = F("IP");
    String val = Check.Wifi_ip;
    tft.setTextSize(labelSize);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(6, y);
    tft.print(lbl);
    const uint8_t ipSize = 1;
    int16_t vx = w - textWidthPx(val, ipSize) - 6;
    if (vx < 6) vx = 6; // clamp left if too long
    tft.setTextSize(ipSize);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(vx, y + (8 * (labelSize - ipSize)) / 2); // vertically align to label row
    tft.print(val);
  }
}

void LCD_ST7735_init(){
  Serial.print(F("LCD_init_Started"));
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);  
  tft.setSPISpeed(160000000);
  Serial.println(F("Initialized"));
  tft.fillScreen(ST77XX_BLACK);
  // Full-screen splash for readability
  TFT_DrawSplash();
  delay(5000);
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
  const int16_t headerH = 10; // slimmer top status bar for more space
  s_midX = w / 2;
  s_sectionH = (h - headerH) / 3; // leave space for header

  // Header line (top-right IP)
  {
    String ip = Check.Wifi_Connected ? Check.Wifi_ip : String("No WiFi");
    String line = String("IP: ") + ip;
    const int16_t charW = 6; // default font width at text size 1
    int16_t textW = line.length() * charW;
    int16_t x = w - textW - 2;
    if (x < 2) x = 2;
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(x, 2);
    tft.print(line);
  }

  // Outer border
  tft.drawLine(0, 0,     w - 1, 0,     ST77XX_BLUE);
  tft.drawLine(0, h - 1, w - 1, h - 1, ST77XX_BLUE);
  tft.drawLine(0, 0,     0,     h - 1, ST77XX_BLUE);
  tft.drawLine(w - 1, 0, w - 1, h - 1, ST77XX_BLUE);

  for (int index = 0; index < 3; ++index) {
    int16_t y0 = headerH + index * s_sectionH;
    int16_t y1 = y0 + s_sectionH - 1;

    // Section borders
    tft.drawLine(0, y0,     w - 1, y0,     ST77XX_BLUE);
    tft.drawLine(0, y1,     w - 1, y1,     ST77XX_BLUE);
    tft.drawLine(s_midX, y0+1, s_midX,  y1-1,  ST77XX_BLUE);

    // Title (smaller per request)
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.setCursor(4, y0 + 2);
    tft.print("CH"); tft.print(index + 1);
    const int16_t titleH = 8 * 1; // text size 1 -> 8px

    // Layout constants
    const int16_t leftX  = 6;
    const int16_t rightX = s_midX + 6;
    const int16_t charW  = 6; // base font metrics
    const int16_t charH  = 8;
    const int16_t labelSize = 1;
    const int16_t unitSize  = 1;
    const int16_t lineH  = (charH * labelSize) + 2; // tighter line spacing for fit
    int16_t tyL = y0 + 2 + titleH + 2; // below title
    int16_t tyR = y0 + 2 + titleH + 2;

    // Back to normal text size for labels/units
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    // Left column labels and units, cache value areas
    // V
    int16_t labelW_V = (int16_t)(2 * charW * labelSize); // "V:" 2 chars
    int16_t valueVX  = (int16_t)(leftX + labelW_V + 3);
    int16_t unitWV   = (int16_t)(1 * charW * unitSize);  // "V"
    int16_t unitVL   = (int16_t)(s_midX - unitWV - 3);
    tft.setCursor(leftX, tyL); tft.print("V:");
    tft.setCursor(unitVL, tyL); tft.print("V");
    s_valPos[index][0] = { valueVX, tyL, (int16_t)(unitVL - valueVX - 1) };

    // I
    tyL += lineH;
    int16_t labelW_I = (int16_t)(2 * charW * labelSize); // "I:"
    int16_t valueIX  = (int16_t)(leftX + labelW_I + 3);
    int16_t unitWI   = (int16_t)(1 * charW * unitSize);  // "A"
    int16_t unitIL   = (int16_t)(s_midX - unitWI - 3);
    tft.setCursor(leftX, tyL); tft.print("I:");
    tft.setCursor(unitIL, tyL); tft.print("A");
    s_valPos[index][1] = { valueIX, tyL, (int16_t)(unitIL - valueIX - 1) };

    // Right column labels and units, cache value areas
    // P
    tft.setCursor(rightX, tyR); tft.print("P:");
    int16_t labelW_P = (int16_t)(2 * charW * labelSize); // "P:"
    int16_t valuePX  = (int16_t)(rightX + labelW_P + 3);
    int16_t unitWP   = (int16_t)(1 * charW * unitSize);  // "W"
    int16_t unitPR   = (int16_t)(w - unitWP - 3);
    tft.setCursor(unitPR, tyR); tft.print("W");
    s_valPos[index][2] = { valuePX, tyR, (int16_t)(unitPR - valuePX - 1) };

    // E (Wh)
    tyR += lineH;
    tft.setCursor(rightX, tyR); tft.print("E:");
    int16_t labelW_E = (int16_t)(2 * charW * labelSize); // "E:"
    int16_t valueEX  = (int16_t)(rightX + labelW_E + 3);
    int16_t unitWE   = (int16_t)(2 * charW * unitSize);  // "Wh"
    int16_t unitER   = (int16_t)(w - unitWE - 3);
    tft.setCursor(unitER, tyR); tft.print("Wh");
    s_valPos[index][3] = { valueEX, tyR, (int16_t)(unitER - valueEX - 1) };
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
