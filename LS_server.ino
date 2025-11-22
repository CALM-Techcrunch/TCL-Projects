
#include "LS_config.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
// #include <WiFiClient.h>

#define LS_SERVER_NAME    "Smart_Light"
#define LS_USERNAME       "admin"
#define CLIENT_TIMEOUT_MINS   5

// --- Lighting and Sensor Pins ---
extern const uint8_t RELAY1_PIN;
extern const uint8_t RELAY2_PIN;
extern const uint8_t RELAY3_PIN;
extern const uint8_t ULTRASONIC_TRIG_PIN;
extern const uint8_t ULTRASONIC_ECHO_PIN;
extern const uint8_t PIR_PIN;


ESP8266WebServer server(80);
LS_COMMAND server_command = NO_COMMAND;

// --- Time-based toggle scheduling ---
struct ToggleSchedule {
    char time[6];  // Format: "HH:MM"
    bool enabled;
    bool oneShot; // If true, disable schedule after first trigger
    char lastTriggeredTime[6];  // To prevent multiple toggles at same minute
};

ToggleSchedule toggleSchedules[3] = {
    {"", false, false, ""},
    {"", false, false, ""},
    {"", false, false, ""}
};

void setToggleSchedule(uint8_t relay, const char* time, bool oneShot) {
    if (relay < 1 || relay > 3) return;
    if (!time) time = "";
    strncpy(toggleSchedules[relay-1].time, time, 5);
    toggleSchedules[relay-1].time[5] = '\0';
    toggleSchedules[relay-1].enabled = (strlen(time) > 0);
    toggleSchedules[relay-1].oneShot = oneShot;
    toggleSchedules[relay-1].lastTriggeredTime[0] = '\0';
    Serial.print("[Toggle Schedule] Light ");
    Serial.print(relay);
    Serial.print(" set to toggle at ");
    Serial.print(toggleSchedules[relay-1].time);
    Serial.print(" oneShot=");
    Serial.println(toggleSchedules[relay-1].oneShot ? "true" : "false");
}

void checkAndToggleSchedules() {
    // Get current time from RTC
    bool ok = LS_timeRead();
    if (!ok) return;
    
    char currentTime[6];
    snprintf(currentTime, sizeof(currentTime), "%02d:%02d", time_date.tm_hour, time_date.tm_min);
    
    for (int i = 0; i < 3; i++) {
        ToggleSchedule* sched = &toggleSchedules[i];
        
        // Check if schedule is enabled and time matches
        if (sched->enabled && strcmp(sched->time, currentTime) == 0) {
            // Check if we haven't already toggled at this time
            if (strcmp(sched->lastTriggeredTime, currentTime) != 0) {
                // Toggle the relay
                uint8_t relay = i + 1;
                bool currentState = getRelay(relay);
                setRelay(relay, !currentState);
                
                // Mark as triggered at this time
                strncpy(sched->lastTriggeredTime, currentTime, 5);
                sched->lastTriggeredTime[5] = '\0';
                
                Serial.print("[Auto Toggle] Light ");
                Serial.print(relay);
                Serial.print(" toggled at ");
                Serial.println(currentTime);
                
                // If this is a one-shot schedule, disable it after triggering
                if (sched->oneShot) {
                    sched->enabled = false;
                    sched->time[0] = '\0';
                    sched->oneShot = false;
                    Serial.print("[Auto Toggle] One-shot schedule for light ");
                    Serial.print(relay);
                    Serial.println(" disabled after trigger");
                }
            }
        }
        // Reset trigger time when we move past the scheduled minute
        else if (strcmp(sched->lastTriggeredTime, currentTime) != 0) {
            sched->lastTriggeredTime[0] = '\0';
        }
    }
}

bool getRelay(uint8_t relay) {
    return relay_states[relay-1];
}

bool appendFile(String dir, String text)
{
    File file = LittleFS.open(dir, "a");

    if (!file)
    {
        // LS_errorHandler( FILE_ERROR);
        return false;
    }

    text += "\n";
    file.print(text);
    file.close();
    return true;
}

bool writeFile(String dir, String text)
{
    File file = LittleFS.open(dir, "w");

    if (!file)
    {
        // LS_errorHandler( FILE_ERROR);
        return false;
    }

    file.print(text);
    delay(1);
    file.close();
    return true;
}

String getContentType(String filename)
{
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".jpeg"))
        return "image/jpeg";
    return "text/plain";
}

bool handleFileRequests( String path )
{
    if (path.endsWith("/")){ path += "index.html"; }

    String contentType = getContentType(path);
    path = "/webapp" + path;
    Serial.print("URL: "); Serial.println(path);

    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }

    Serial.println("Not found");
    return false;
}


bool LS_serverStop( void )
{
    server.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);

    // LS_params.server_running = false;
// 
    return true;
}
void handlePir() {
    int pirValue = analogRead(A0);
    bool motion = pirValue > 1020;
    String res = "{\"motion\":" + String(motion ? "true" : "false") + "}";
    server.send(200, "application/json", res);
}
void handleUltrasonic() {
        digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
        long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
        int distance = duration * 0.034 / 2;
        String res = "{\"distance\":" + String(distance) + "}";
        server.send(200, "application/json", res);
    }
void handleRelayStatus() {
        String uri = server.uri();
        uint8_t relay = uri.charAt(8) - '0';
        if (relay < 1 || relay > 3) {
            server.send(400, "application/json", "{\"error\":\"Invalid relay\"}");
            return;
        }
        bool state = getRelay(relay);
        String res = "{\"status\":\"" + String(state ? "on" : "off") + "\"}";
        server.send(200, "application/json", res);
    }
void handleRelay() {
        String uri = server.uri();
        uint8_t relay = uri.charAt(7) - '0';
        String statusStr = uri.substring(9);
        bool state = (statusStr == "on");
        if (relay < 1 || relay > 3) {
            server.send(400, "application/json", "{\"error\":\"Invalid relay\"}");
            return;
        }
        setRelay(relay, state);
        String res = "{\"status\":\"" + String(state ? "on" : "off") + "\"}";
        server.send(200, "application/json", res);
 }

void handleRTC() {
    bool ok = LS_timeRead();
    if (!ok) {
        server.send(500, "application/json", "{\"error\":\"RTC not available\"}");
        return;
    }
    
    char buf[64];
    int year = 2000 + time_date.tm_year;
    const char daysOfTheWeek[7][10] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    
    // Format: YYYY-MM-DD DayName HH:MM:SS
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %s %02d:%02d:%02d", 
             year, time_date.tm_mon, time_date.tm_mday, 
             daysOfTheWeek[time_date.tm_wday],
             time_date.tm_hour, time_date.tm_min, time_date.tm_sec);
    
    String res = String("{\"time\":\"") + String(buf) + String("\"}");
    server.send(200, "application/json", res);
}

void handleSetTime() {
    if (server.method() != HTTP_POST) {
        server.send(400, "application/json", "{\"error\":\"POST only\"}");
        return;
    }
    
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }
    
    String body = server.arg("plain");
    
    // Parse JSON: {"year":2025,"month":11,"day":22,"hour":14,"minute":30,"second":45}
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    
    // Simple JSON parsing
    if (sscanf(body.c_str(), "{\"year\":%d,\"month\":%d,\"day\":%d,\"hour\":%d,\"minute\":%d,\"second\":%d}", 
               &year, &month, &day, &hour, &minute, &second) != 6) {
        server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
        return;
    }
    
    // Validate ranges
    if (year < 2000 || year > 2099 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        server.send(400, "application/json", "{\"error\":\"Invalid time values\"}");
        return;
    }
    
    // Create struct tm and set RTC
    struct tm newTime;
    newTime.tm_year = year - 2000;
    newTime.tm_mon = month;
    newTime.tm_mday = day;
    newTime.tm_hour = hour;
    newTime.tm_min = minute;
    newTime.tm_sec = second;
    
    bool success = LS_timeSet(&newTime);
    
    if (success) {
        Serial.println("[RTC] Time synced from web interface");
        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(500, "application/json", "{\"error\":\"Failed to set time\"}");
    }
}

void handleSetToggleSchedule() {
    if (server.method() != HTTP_POST) {
        server.send(400, "application/json", "{\"error\":\"POST only\"}");
        return;
    }
    
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"error\":\"No body\"}");
        return;
    }
    
    String body = server.arg("plain");

    // Robust JSON parsing by searching for keys so order doesn't matter
    int light = -1;
    String timeStr = "";
    bool oneShot = true; // default to one-shot

    int idx = body.indexOf("\"light\"");
    if (idx != -1) {
        int colon = body.indexOf(':', idx);
        if (colon != -1) {
            // parse integer after colon
            light = body.substring(colon + 1).toInt();
        }
    }

    idx = body.indexOf("\"time\"");
    if (idx != -1) {
        int tstart = body.indexOf("\"time\":\"");
        if (tstart != -1) {
            tstart += 8; // move to first char of time value
            int tend = body.indexOf('"', tstart);
            if (tend != -1) {
                timeStr = body.substring(tstart, tend);
            }
        }
    }

    // oneShot flag
    if (body.indexOf("\"oneShot\":false") != -1) oneShot = false;
    if (body.indexOf("\"oneShot\":true") != -1) oneShot = true;

    if (light < 1 || light > 3) {
        server.send(400, "application/json", "{\"error\":\"Invalid light number\"}");
        return;
    }

    // Normalize timeStr to c-string for setToggleSchedule
    char timeBuf[6] = "";
    if (timeStr.length() > 0) {
        if (timeStr.length() > 5) timeStr = timeStr.substring(0,5);
        timeStr.toCharArray(timeBuf, sizeof(timeBuf));
        // Validate HH:MM
        int h = -1, m = -1;
        if (sscanf(timeBuf, "%d:%d", &h, &m) != 2 || h < 0 || h > 23 || m < 0 || m > 59) {
            server.send(400, "application/json", "{\"error\":\"Invalid time values\"}");
            return;
        }
    }

    // Set the toggle schedule for this light only
    setToggleSchedule(light, timeBuf, oneShot);

    if (strlen(timeBuf) > 0) {
        Serial.print("[Toggle] Light ");
        Serial.print(light);
        Serial.print(" scheduled to toggle at ");
        Serial.println(timeBuf);
    } else {
        Serial.print("[Toggle] Light ");
        Serial.print(light);
        Serial.println(" toggle schedule disabled");
    }
    
    server.send(200, "application/json", "{\"success\":true}");
}

    

    

    

bool LS_serverStart( void )
{
    WiFi.forceSleepWake();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(LS_SERVER_NAME, ""); // Start the access point with no password
    MDNS.begin("Smart_Light");     // Start the mDNS responder for Ahome.local
    
        // Lighting and sensor endpoints for webapp
    server.on("/relay/1/on", HTTP_POST, handleRelay);
    server.on("/relay/1/off", HTTP_POST, handleRelay);
    server.on("/relay/2/on", HTTP_POST, handleRelay);
    server.on("/relay/2/off", HTTP_POST, handleRelay);
    server.on("/relay/3/on", HTTP_POST, handleRelay);
    server.on("/relay/3/off", HTTP_POST, handleRelay);
    server.on("/status/1", HTTP_GET, handleRelayStatus);
    server.on("/status/2", HTTP_GET, handleRelayStatus);
    server.on("/status/3", HTTP_GET, handleRelayStatus);
    server.on("/pir", HTTP_GET, handlePir);
    server.on("/ultrasonic", HTTP_GET, handleUltrasonic);
    server.on("/rtc", HTTP_GET, handleRTC);
    server.on("/set-time", HTTP_POST, handleSetTime);
    server.on("/set-toggle-schedule", HTTP_POST, handleSetToggleSchedule);
    

    server.onNotFound([]() {                              // If the client requests any URI
        if (!handleFileRequests(server.uri()))                  // send it if it exists
        server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    server.begin();
    IPAddress ip = WiFi.softAPIP();
    return true;
}

void LS_serverInit( void )
{
    LittleFS.begin();
    LS_serverStart();
}

LS_COMMAND LS_serverGetUpdates()
{
    server.handleClient();
    LS_COMMAND new_command = server_command;
    server_command = NO_COMMAND;
    return new_command;
}

/* ----------------------------------- EOF ---------------------------------- */
