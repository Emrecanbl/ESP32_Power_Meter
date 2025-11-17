#include "Adafruit_INA3221.h"
#include <Wire.h>

// put function declarations here:
#define I2C_SDA 11
#define I2C_SCL 10
// ---------- Shared data model ----------
struct PowerSample {
    uint32_t ms; // uptime ms
    float Voltage[3]={0.0,0.0,0.0}; // Channel voltage
    float Current[3]={0.0,0.0,0.0}; // Channel current
    double Power[3]={0.0,0.0,0.0}; // Channel power
    double energyWh[3]={0.0,0.0,0.0}; // Channel Total Energie
    bool counter_state[3]={false,false,false}; // Channel Counter State run/stop
    bool reset_state[3]={false,false,false};   // Channel Counter Reset Value
};

extern Adafruit_INA3221 ina3221;

void Sensor_init();
void Sensor_Voltage_Read();
void Sensor_Current_Read();
void Sensor_Read(PowerSample &Power_Values);
void Sensor_Uart_Out(PowerSample &Power_Values);
