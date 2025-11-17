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
void Sensor_Uart_In(PowerSample& Power_Values) {
 String payload = Serial.readStringUntil('\n');
  payload.trim();
  if (payload.isEmpty()) {
    return;
  }

  // Expected format: CH(C),Status(S),Reset(R) -> Cx,Sy,Rz or simply x,y,z
  const int firstComma = payload.indexOf(',');
  const int secondComma = payload.indexOf(',', firstComma + 1);
  if (firstComma < 0 || secondComma < 0) {
    Serial.print("Invalid frame (comma): ");
    Serial.println(payload);
    return;
  }

  auto extractValue = [](const String& token) -> int {
    for (size_t i = 0; i < token.length(); ++i) {
      if (isDigit(token[i])) {
        return token.substring(i).toInt();
      }
    }
    return -1;
  };

  const String chToken = payload.substring(0, firstComma);
  const String statusToken = payload.substring(firstComma + 1, secondComma);
  const String resetToken = payload.substring(secondComma + 1);

  const int channel = extractValue(chToken);
  const bool status = extractValue(statusToken);
  const bool reset = extractValue(resetToken);

  if (channel < 0 || channel >= 3 || status < 0 || reset < 0) {
    Serial.print("Invalid frame (value): ");
    Serial.println(payload);
    return;
  }

  Power_Values.counter_state[channel] = status != 0;
  Power_Values.reset_state[channel] = reset != 0;
}