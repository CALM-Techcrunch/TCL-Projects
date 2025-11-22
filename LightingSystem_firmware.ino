#include "LS_config.h"
#include <ESP8266WebServer.h>
extern ESP8266WebServer server;

// Definitions for shared globals declared as extern in LS_config.h
bool relay_states[3] = {false, false, false};
bool server_running = false;

// --- Lighting and Sensor Pins ---
const uint8_t RELAY1_PIN = D0;
const uint8_t RELAY2_PIN = D4;
const uint8_t RELAY3_PIN = D2;
const uint8_t ULTRASONIC_TRIG_PIN = D6;
const uint8_t ULTRASONIC_ECHO_PIN = D7;
const uint8_t PIR_PIN = A0;
const uint8_t Buzzer_PIN = D8;

struct tm time_date;

// For non-blocking intruder alarm check
unsigned long lastIntruderCheck = 0;
const unsigned long intruderInterval = 100; // ms

void setRelay(uint8_t relay, bool state) {
    switch(relay){
        case 1:
         digitalWrite(RELAY1_PIN,state ? HIGH : LOW);
         string1(state ? "ON" : "OFF");
         relay_states[0] = state;
         break;
        case 2:
         digitalWrite(RELAY2_PIN,state ? HIGH : LOW);
         string2(state ? "ON" : "OFF");
         relay_states[1] = state;
         break;
        case 3:
         digitalWrite(RELAY3_PIN,state ? HIGH : LOW);
         string3(state ? "ON" : "OFF");
         relay_states[2] = state;
         break; 
        default:
          Rly_status("OFF","OFF","OFF");


    }
      

    
}


void setup()      
{
    Serial.begin(115200); delay(100);
    Serial.println("\n\n=== SYSTEM STARTUP ===");
    Serial.println("Starting initialization...");
    
    Serial.println("[SETUP] Initializing I2C on D1(SCL) and D3(SDA)...");
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("[SETUP] I2C initialized");
    
    // Initialize lighting and sensor pins
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(Buzzer_PIN,OUTPUT);
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RELAY3_PIN, LOW);
    IR_init();
    Serial.println("[SETUP] IR initialized");
    
    LS_inputSetup();
    Serial.println("[SETUP] Input setup complete");
    
    LS_timeInit();
    Serial.println("[SETUP] RTC initialized");
    
    LS_serverInit();
    Serial.println("[SETUP] Server initialized");
    
    lcd_init();
    Serial.println("[SETUP] LCD initialized");
    
    Serial.println("=== INITIALIZATION COMPLETE ===\n");
}

/// @brief keep it going
void loop()
{
    
    server.handleClient();
    
    handleRelayClicks();
    
    // Check for time-based toggle schedules
    checkAndToggleSchedules();
    
    // Non-blocking intruder alarm check
    if (millis() - lastIntruderCheck >= intruderInterval) {
        lastIntruderCheck = millis();
        intruderAlarm();
    }
    
    handleRemote();
    
}

void intruderAlarm(){
    int sensorValue = analogRead(A0);
    // Read distance from ultrasonic sensor
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
    int distance = duration * 0.034 / 2;

    static bool sirenToggle = false;
    static unsigned long lastSirenTone = 0;
    const unsigned long sirenToneInterval = 150; // ms
    if(sensorValue > 1020 && distance > 150){
        unsigned long now = millis();
        if (now - lastSirenTone > sirenToneInterval) {
            sirenToggle = !sirenToggle;
            lastSirenTone = now;
            if (sirenToggle) {
                tone(Buzzer_PIN, 1000, sirenToneInterval);
            } else {
                tone(Buzzer_PIN, 2000, sirenToneInterval);
            }
        }
    } else {
        noTone(Buzzer_PIN);
    }
}


/* ----------------------------------- EOF ---------------------------------- */