#include <Arduino.h>
#include <INA3221.h>
#include <Web_Ui.h>
#include <LCD_ST7735.h>
#include <esp_system.h>
// ---------- Config ----------
static constexpr uint32_t SENSOR_PERIOD_MS = 100; // 10 Hz
static constexpr uint32_t SCREEN_PERIOD_MS = 8; // ~30 FPS (adjust as needed)
static constexpr uint32_t UART_PERIOD_MS = 1000; // 1 Hz
static constexpr uint32_t WEB_LOOP_MS = 10; // service HTTP frequently
static constexpr uint32_t Mutex_Max_Wait_MS = 1000;                 

struct Start_Check
{
 bool Sensor_Started = false;
 bool Wifi_Connected = false;
 String Wifi_ip;
};

extern Start_Check Check;

constexpr uint8_t BUTTON_PINS[3] = {6, 9, 15};      // adjust to your wiring
constexpr TickType_t LONG_PRESS_TICKS = pdMS_TO_TICKS(1500);
constexpr TickType_t ISR_QUEUE_TIMEOUT = 0;

static void IRAM_ATTR button0ISR();
static void IRAM_ATTR button1ISR();
static void IRAM_ATTR button2ISR();

struct ButtonEvent {
  uint8_t index;
  bool longPress;
};

struct ButtonState {
  TickType_t pressTick = 0;
  bool pressed = false;
};

static ButtonState g_buttonStates[3];
static QueueHandle_t g_buttonQueue;
static SemaphoreHandle_t g_buttonMux = xSemaphoreCreateBinary();
static bool g_isRunning = false;

void initButtons();
static void toggleRunState();
static void taskButtons(void* arg);
static void toggleResetState() ;

