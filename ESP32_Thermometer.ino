#include <WiFi.h>
#include <WebServer.h>
#include <esp_adc_cal.h>
#include "html.h"
 
const char* ssid = "FIDAA";
const char* password = "littleshark528";
 
WebServer server(80);
 
#define THERMOCOUPLE_PIN 33
#define ANALOG_REFERENCE_VOLTAGE 3.3
#define ADC_BIT_RESOLUTION 12
 
float calcVoltage, actTemperature;
 
float convertRawAdcToVoltage(uint32_t rawAdcValue)
{
  esp_adc_cal_characteristics_t adcCharacteristics;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adcCharacteristics);
  uint32_t voltageInMilliVolts = esp_adc_cal_raw_to_voltage(rawAdcValue, &adcCharacteristics);
  return voltageInMilliVolts / 1000.0;
}
 
float calculateTemperatureFromVoltage(float voltage)
{
  return (voltage - 1.25) / 0.005;
}
 
void setup()
{
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    Serial.println("Connecting to WiFi...");
  }
 
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 
  server.on("/", []()
  {
    server.send_P(200, "text/html", html_page);
  });
 
  server.on("/readTemp", HTTP_GET, []() {
    float celsius = actTemperature;
    float fahrenheit = celsius * 1.8 + 32;
    char json[70];
    snprintf(json, 70, "{\"celsius\": \"%.2f\", \"fahrenheit\": \"%.2f\"}", celsius, fahrenheit);
    server.send(200, "application/json", json);
  });
 
  server.begin();
}
 
void loop()
{
  uint32_t rawAdcReading = analogRead(THERMOCOUPLE_PIN);
  calcVoltage = convertRawAdcToVoltage(rawAdcReading);
  actTemperature = calculateTemperatureFromVoltage(calcVoltage);
 
  server.handleClient();
}
