#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
// This library is exported from Edge Impulse Studio
#include <Smart_Appliance_Identification_inferencing.h> 

// Hardware and Network configurations
const int sensorPin = A0;
const float vRms = 230.0; // 230V AC
const float acs712_sensitivity = 0.185; // 185 mV/A for 5A model

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* server = "http://api.thingspeak.com/update";
const String apiKey = "YOUR_THINGSPEAK_API_KEY";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected!");
}

void loop() {
    // ---------------------------------------------------------
    // STAGE 1: Offset Calibration
    // ---------------------------------------------------------
    long offsetSum = 0;
    for (int i = 0; i < 200; i++) {
        offsetSum += analogRead(sensorPin);
        delayMicroseconds(1000); // 1 kHz sampling
    }
    float dcOffset = offsetSum / 200.0;

    // ---------------------------------------------------------
    // STAGE 2: Feature Acquisition
    // ---------------------------------------------------------
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };
    for (int i = 0; i < 200; i++) {
        // Centered ADC sample
        buffer[i] = analogRead(sensorPin) - dcOffset; 
        delayMicroseconds(1000); // 1000 us delay for 1kHz logical rate
    }

    // ---------------------------------------------------------
    // STAGE 3: TinyML Inference
    // ---------------------------------------------------------
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    
    ei_impulse_result_t result = { 0 };
    err = run_classifier(&signal, &result, false);

    // Find the highest confidence class
    int maxIndex = 0;
    float maxConfidence = 0.0;
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > maxConfidence) {
            maxConfidence = result.classification[i].value;
            maxIndex = i;
        }
    }
    
    // Appliance ID mapping (0=noload, 1=phone, 2=laptop, 3=fan, 4=kettle)
    int applianceId = maxIndex; 

    // ---------------------------------------------------------
    // STAGE 4: Energy Computation and Upload
    // ---------------------------------------------------------
    float currentSumSq = 0;
    for (int i = 0; i < 200; i++) {
        float centeredVoltage = (analogRead(sensorPin) - dcOffset) * (3.3 / 1023.0);
        float current = centeredVoltage / acs712_sensitivity;
        currentSumSq += (current * current);
        delayMicroseconds(1000);
    }
    
    float iRms = sqrt(currentSumSq / 200.0);
    float apparentPower = vRms * iRms;

    // Transmit to ThingSpeak via HTTP GET
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        
        String url = String(server) + "?api_key=" + apiKey + 
                     "&field1=" + String(applianceId) +
                     "&field2=" + String(maxConfidence, 3) +
                     "&field3=" + String(iRms, 3) +
                     "&field4=" + String(apparentPower, 2);
                     
        http.begin(client, url);
        int httpCode = http.GET();
        http.end();
        
        Serial.println("Data sent to ThingSpeak. HTTP Code: " + String(httpCode));
    }
    
    // 16-second delay to comply with ThingSpeak free-tier rate limits
    delay(16000); 
}