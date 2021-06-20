#ifndef Globals_h
#define Globals_h

typedef enum State {
  Run,
  SetHour,
  SetMinute,
  SetSecond
} State;

// Task scheduler
#define NOTASKS 3
#define TASKLOOPWAITMS 20

// Pins to HV5530 shift registers
#define PIN_HV_LE 10      // HV5530 Latch
#define PIN_HV_BL 12      // HV5530 Blank (PWM)
#define PIN_HV_DATA 11    // HV5530 Data
#define PIN_HV_CLK 13     // HV5530 Clock

#define PIN_DOT_THREE 2   // Transistor cathode dot driver on Nixie 3 (PWM)
#define PIN_DOT_FOUR 3    // Transistor cathode dot driver on Nixie 4 (PWM)

#define PIN_COLON A2      // Transistor LED colon driver (PWM)

#define PIN_SHDN A1       // Logic level control for 5V to 12V DC Booster, HIGH turns it on, turn it off when DC input voltage is 12V
#define PIN_HV_EN A6      // Logic level control for enabling the HV DC booster, HIGH = On, LOW = Off (The UCC3803D Booster is soft start), this connection is isolated using an opto isolator

#define PIN_PIR A0        // Logic input for PIR Sensor

// #define PIN_SW1 4      // Menu button PINs
// #define PIN_SW2 7
// #define PIN_SW3 8
#define PIN_SW1_LED1 5    // Menu button PWM LED control PINs
#define PIN_SW2_LED2 6
#define PIN_SW3_LED3 9

#define RandomSeed A7     // Pin used to generate random seed

//Button buttonA = Button(PIN_SW1, BUTTON_PULLUP);      // Menu button
//Button buttonB = Button(PIN_SW2, BUTTON_PULLUP);      // Display date / + button
//Button buttonC = Button(PIN_SW3, BUTTON_PULLUP);      // Temp/Humidity/Pressure / - button

#endif
