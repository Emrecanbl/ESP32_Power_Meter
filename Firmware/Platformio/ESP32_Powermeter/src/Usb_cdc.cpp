#include <main.h>

Adafruit_INA3221 ina3221;

void Sensor_Uart_Out(PowerSample& Power_Values) {
  // UART table output: CH,V,A,P,Wh
  Serial.println("CH,V,A,P");
  for (uint8_t i = 0; i < 3; i++) {
    Serial.print(i);
    Serial.print(',');
    Serial.print(Power_Values.Voltage[i], 2);
    Serial.print(',');
    Serial.print(Power_Values.Current[i], 3);
    Serial.print(',');
    Serial.print(Power_Values.Power[i], 2);
    Serial.print(',');
    Serial.println(Power_Values.energyWh[i], 3);
  }
}