#include <INA3221.h>

Adafruit_INA3221 ina3221;

void Sensor_init(){
  
  delay(10); // Wait for serial port to connect on some boards
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("INA3221 Test Start");
  // Initialize the INA3221
  bool init_ok = false;
  for (int attempt = 0; attempt < 10; ++attempt) { // retry 10 times
    if (ina3221.begin(0x40, &Wire)) { // can use other I2C addresses or buses
      init_ok = true;
      break;
    }
    delay(100); // wait 100 ms before next attempt
  }
  if (!init_ok) {
    Serial.println("INA3221 init FAIL after 10 attempts");
    return;
  }
  Serial.println("INA3221 Found!");

  ina3221.setAveragingMode(INA3221_AVG_16_SAMPLES);

  // Set shunt resistances for all channels to 0.05 ohms
  for (uint8_t i = 0; i < 3; i++) {
    ina3221.setShuntResistance(i, 0.02);
  }

  // Set a power valid alert to tell us if ALL channels are between the two
  // limits:
  ina3221.setPowerValidLimits(3.0 /* lower limit */, 15.0 /* upper limit */);
}
void Sensor_Voltage_Read(){
      for (uint8_t i = 0; i < 3; i++) {
    float voltage = ina3221.getBusVoltage(i); // V
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(" Voltage = ");
    Serial.print(voltage, 2);
    Serial.println(" V ");
  }
  Serial.println();
}
void Sensor_Current_Read(){
    for (uint8_t i = 0; i < 3; i++) {
    double current = ina3221.getCurrentAmps(i); // A
    Serial.print("Channel ");
    Serial.print(i);
    Serial.print(" Current = ");
    Serial.print(current, 2);
    Serial.println(" A");
  }
  Serial.println();
}
void Sensor_Read(PowerSample &Power_Values){
  static uint32_t lastMs = 0;

  float voltage[3];
  double current[3];
  double power[3];

  for (uint8_t i = 0; i < 3; i++) {
    Power_Values.Voltage[i] = ina3221.getBusVoltage(i); // V
    Power_Values.Current[i]  = ina3221.getCurrentAmps(i); // A
    Power_Values.Power[i]  = Power_Values.Voltage[i] * Power_Values.Current[i]; //P 

  }

  uint32_t now = millis();
  if (lastMs != 0) {
    double dt_h = (now - lastMs) / 3600000.0; // hours
    for (uint8_t i = 0; i < 3; i++) {
      if(Power_Values.reset_state[i] == false){
        if(Power_Values.counter_state[i] == true){
          Power_Values.energyWh[i] += Power_Values.Power[i] * dt_h; // Wh = W * h
        }
      }
      else{
          Power_Values.energyWh[i] = 0.0;
          Power_Values.reset_state[i] = false;
      } 
    }
  }
  lastMs = now;
}
void Sensor_Uart_Out(PowerSample &Power_Values){
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

