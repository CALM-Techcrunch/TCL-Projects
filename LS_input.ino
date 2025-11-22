#include <ClickButton.h>
#include "LS_config.h"

#define BUTTON_PIN D5
extern const uint8_t RELAY1_PIN;
extern const uint8_t RELAY2_PIN;
extern const uint8_t RELAY3_PIN;

// Track relay states locally for toggling
static bool relay1_state = false;
static bool relay2_state = false;
static bool relay3_state = false;

ClickButton button(BUTTON_PIN, HIGH, CLICKBTN_PULLUP);
void handleRelayClicks() {
    button.Update();
    if (button.clicks != 0) {
        // Long press (negative click count) toggles all lights
        if (button.clicks < 0) {
            relay1_state = !relay1_state;
            relay2_state = !relay2_state;
            relay3_state = !relay3_state;
            setRelay(1, relay1_state ? HIGH : LOW);
            setRelay(2, relay2_state ? HIGH : LOW);
            setRelay(3, relay3_state ? HIGH : LOW);
            #ifdef USING_NODEMCU
                Serial.println("=== ALL LIGHTS TOGGLED ===");
                Serial.print("Relay 1: ");
                Serial.println(relay1_state ? "ON" : "OFF");
                Serial.print("Relay 2: ");
                Serial.println(relay2_state ? "ON" : "OFF");
                Serial.print("Relay 3: ");
                Serial.println(relay3_state ? "ON" : "OFF");
            #endif
        } else {
            // Standard multi-click handling
            switch (button.clicks) {
                case 1:
                    relay1_state = !relay1_state;
                    setRelay(1, relay1_state ? HIGH : LOW);
                    
                    #ifdef USING_NODEMCU
                        Serial.println(relay1_state ? "Relay 1 ON" : "Relay 1 OFF");
                    #endif
                    break;
                case 2:
                    relay2_state = !relay2_state;
                    setRelay(2, relay2_state ? HIGH : LOW);
                    #ifdef USING_NODEMCU
                        Serial.println(relay2_state ? "Relay 2 ON" : "Relay 2 OFF");
                    #endif
                    break;
                case 3:
                    relay3_state = !relay3_state;
                    setRelay(3, relay3_state ? HIGH : LOW);
                    #ifdef USING_NODEMCU
                        Serial.println(relay3_state ? "Relay 3 ON" : "Relay 3 OFF");
                    #endif
                    break;
            }
        }
    }
    
}

/** @brief Debounce timer in ms */
#define  DEBOUNCE_TIME        20

/** @brief Time limit for multi clicks */
#define  MULTI_CLICK_TIME    500

/** @brief time until "held-down clicks" */
#define  LONG_CLICK_TIME    1500



void LS_inputSetup( void )
{
//    pinMode(BUTTON_PIN, FUNCTION_3);
    button.debounceTime   = DEBOUNCE_TIME;
    button.multiclickTime = MULTI_CLICK_TIME;
    button.longClickTime  = LONG_CLICK_TIME;
}



LS_COMMAND LS_inputRead(void)
{
    button.Update();
    if (button.clicks != 0)
    {
        switch (button.clicks)
        {
            case 1:
                #ifdef USING_NODEMCU
                    Serial.println("YOU CLICKED ONCE");
                #endif
                return LS_SCREEN;
                break;
            case 2:
                #ifdef USING_NODEMCU
                    Serial.println("YOU CLICKED TWICE");
                #endif
                return SWITCH_LS;
            break;
            case 3:
                #ifdef USING_NODEMCU
                    Serial.println("YOU CLICKED THRICE");
                #endif
                return SWITCH_SERVER;
            break;
            case -1:
                #ifdef USING_NODEMCU
                    Serial.println("YOU CLICKED LONGER 1"); 
                #endif
                return SWITCH_MAIN_ALARM;
            break;
            case -3:
                #ifdef USING_NODEMCU
                    Serial.println("YOU CLICKED LONGER 2");
                #endif
                return FACTORY_RESET;
            break;
        }
    }
    return NO_COMMAND;
}
