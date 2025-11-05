#include <main.h>

PowerSample power_Values;

// ---- Sync primitives ----
static SemaphoreHandle_t g_Sensor_read_Mutex = nullptr; // protects Sensor_read and Uart_out

// --- Tasks (empty bodies) ---
static void taskSensor(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, portMAX_DELAY) == pdTRUE){
    Sensor_Read(power_Values);
    xSemaphoreGive(g_Sensor_read_Mutex);
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
  }
}


static void taskScreen(void* arg) {
  (void)arg;
  for (;;) {
    TFT_UpdateValues(power_Values);
    vTaskDelay(pdMS_TO_TICKS(SCREEN_PERIOD_MS));
  }
}


static void taskWeb(void* arg) {
  (void)arg;
  for (;;) {
    WEB_UI_Stream(power_Values);
    vTaskDelay(pdMS_TO_TICKS(WEB_LOOP_MS));
  }
}


static void taskUart(void* arg) {
  (void)arg;
  for (;;) {
    if (xSemaphoreTake(g_Sensor_read_Mutex, 1000) == pdTRUE){
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
xTaskCreatePinnedToCore(taskUart, "uart", 3072, nullptr, 1, nullptr, 0);
xTaskCreatePinnedToCore(taskWeb, "web", 4096, nullptr, 2, nullptr, 0);
}


void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
