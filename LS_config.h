#ifndef LS_H
#define LS_H

#include <Wire.h>
#include <LittleFS.h>
#include <time.h>

const uint8_t I2C_SDA = D3;  
const uint8_t I2C_SCL = D1;

typedef enum LS_COMMAND
{
    FACTORY_RESET = -2,
    NO_COMMAND = 0,
    LS_SCREEN,
    SWITCH_MAIN_ALARM,
    SWITCH_SERVER,
    SWITCH_LS
} LS_COMMAND;

extern bool server_running;
extern struct tm time_date;
extern bool relay_states[3];

// Input handling
void LS_inputSetup(void);
LS_COMMAND LS_inputRead(void);
void handleRelayClicks(void);

// Server
void LS_serverInit(void);
bool LS_serverStart(void);
bool LS_serverStop(void);
LS_COMMAND LS_serverGetUpdates(void);

// LCD display
void lcd_init(void);
void Row1(char secondPrint[]);
void Row2(char secondPrint[]);
void Rly_status(char one[], char two[], char three[]);
void string1(const char one[]);
void string2(const char two[]);
void string3(const char three[]);

// Relay control
void setRelay(uint8_t relay, bool state);

// IR remote
void IR_init(void);
int getRemoteKey(char value);
void handleRemote(void);

#endif

/* ------------------------------- END OF FILE ------------------------------ */
