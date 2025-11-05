
#include "Adafruit_INA3221.h"
#include <Wire.h>

// put function declarations here:
#define I2C_SDA 11
#define I2C_SCL 10
// ---------- Shared data model ----------
struct PowerSample {
    uint32_t ms; // uptime ms
    float Voltage[3]; // Channel voltage
    float Current[3]; // Channel current
    double Power[3]; // Channel power
    double energyWh[3]; // Channel Total Energie
    bool counter_state[3]; // Channel Counter State run/stop
    bool reset_state[3];   // Channel Counter Reset Value
};

void Sensor_init();
void Sensor_Voltage_Read();
void Sensor_Current_Read();
void Sensor_Read(PowerSample &Power_Values);
void Sensor_Uart_Out(PowerSample &Power_Values);