#include <main.h>

PowerSample power_Values;

// ---- Sync primitives ----
static SemaphoreHandle_t g_Sensor_read_Mutex = nullptr; // protects Sensor_read and Uart_out

// --- Tasks (empty bodies) ---
static void taskSensor(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, 100) == pdTRUE){
    Sensor_Read(power_Values);
    xSemaphoreGive(g_Sensor_read_Mutex);
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
  }
}


static void taskScreen(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, 100) == pdTRUE){
      TFT_UpdateValues(power_Values);
      xSemaphoreGive(g_Sensor_read_Mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(SCREEN_PERIOD_MS));
  }
}


static void taskWeb(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, 1000) == pdTRUE){
      WEB_UI_Stream(power_Values);
      xSemaphoreGive(g_Sensor_read_Mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(WEB_LOOP_MS));
  }
}


static void taskUart(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, 100) == pdTRUE){
      Sensor_Uart_Out(power_Values);
      xSemaphoreGive(g_Sensor_read_Mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(UART_PERIOD_MS));
  }
}


// --- Setup: only creates tasks ---
void setup() {
Serial.begin(115200);
Sensor_init();
WEB_UI_init();
LCD_ST7735_init();
g_Sensor_read_Mutex = xSemaphoreCreateMutex();
// Create tasks (adjust stack/prio/cores)
xTaskCreatePinnedToCore(taskSensor, "sensor", 4096, nullptr, 3, nullptr, 1);
xTaskCreatePinnedToCore(taskScreen, "screen", 4096, nullptr, 2, nullptr, 1);
xTaskCreatePinnedToCore(taskUart, "uart", 4096, nullptr, 1, nullptr, 0);
xTaskCreatePinnedToCore(taskWeb, "web", 4096, nullptr, 2, nullptr, 0);

g_buttonQueue = xQueueCreate(6, sizeof(ButtonEvent));
pinMode(BUTTON_PINS[0], INPUT_PULLUP);
pinMode(BUTTON_PINS[1], INPUT_PULLUP);
pinMode(BUTTON_PINS[2], INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[0]), button0ISR, FALLING);
attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[1]), button1ISR, FALLING);
attachInterrupt(digitalPinToInterrupt(BUTTON_PINS[2]), button2ISR, FALLING);
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}


static void IRAM_ATTR handleButtonFromISR(uint8_t index) {
  const bool isLow = digitalRead(BUTTON_PINS[index]) == LOW;
  BaseType_t hpTaskWoken = pdFALSE;
  const TickType_t now = xTaskGetTickCountFromISR();

  if (isLow && !g_buttonStates[index].pressed) {
    g_buttonStates[index].pressed = true;
    g_buttonStates[index].pressTick = now;
  } else if (!isLow && g_buttonStates[index].pressed) {
    const TickType_t delta = now - g_buttonStates[index].pressTick;
    const bool isLong = delta >= LONG_PRESS_TICKS;
    ButtonEvent evt{index, isLong};
    g_buttonStates[index].pressed = false;
    xQueueSendFromISR(g_buttonQueue, &evt, &hpTaskWoken);
  }
  if (hpTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

static void IRAM_ATTR button0ISR() { handleButtonFromISR(0); }
static void IRAM_ATTR button1ISR() { handleButtonFromISR(1); }
static void IRAM_ATTR button2ISR() { handleButtonFromISR(2); }


static void taskButtons(void* arg) {
  ButtonEvent evt;
  for (;;) {
    if (xQueueReceive(g_buttonQueue, &evt, portMAX_DELAY) == pdPASS) {
      if (evt.longPress) {
        power_Values.reset_state[evt.index] = !power_Values.reset_state[evt.index]; 
      }
      else {
        power_Values.counter_state[evt.index] = !power_Values.counter_state[evt.index];
      }
    }
  }
}

