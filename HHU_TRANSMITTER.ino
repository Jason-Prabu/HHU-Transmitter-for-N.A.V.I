#include <Keypad.h>               // FOR KEYPAD
#include <SPI.h>                  // FOR NRF24LO1
#include <nRF24L01.h>
#include <RF24.h>                 //RF LIBRARY
#include <RF24_config.h>          // FOR DEBUG INFO
#include <printf.h>

//PIEZO-BUZZER TONE FREQUENCIES

#define clear_tone_freq 2000
#define send_tone_freq  1500
#define num_tone_freq   600
#define alpha_tone_freq 200


RF24 radio(7, 8);                                         // CE AND CSN PINS CONNECTED TO D7 AND D8
const uint64_t pipes[1] = {0xF0F0F0F0E1LL};               // TRANSMITTER ADDRESS

const int buzzer = 9;               // +VE TERMINAL OF BUZZER CONNECTED TO D9 THROUGHN RESISTOR

char bus_arr[6];
int k = 0, send_flag = 0;

//KEYPAD ROWS FIRST THEN COLUMN - LEFT TO RIGHT

const byte ROWS = 4;          // 4 ROWS
const byte COLS = 4;          // 3 COLUMNS
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};                                //KEY MAP

byte rowPins[ROWS] = {A0, A1, A2, A3}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


void setup() {

  Serial.begin(9600);

  radio.begin();                          // START RADIO  
  radio.setPALevel(RF24_PA_MIN);          // SET TO LOW POWER CONSUMPTION
  radio.setRetries(15, 15);               // RETRY EVERY 350 milliseconds FOR 15 TIMES
  radio.openWritingPipe(pipes[0]);        // CREATE PIPE TO RECEIVER ADDRESS
  radio.stopListening();                  // ENTER WRITING MODE

  pinMode(buzzer, OUTPUT); // SET BUZZER - D9 AS OUTPUT

}


void loop() {

  if (kpd.getKeys())   // IF KEY AVAILABLE
  {
    for (int i = 0; i < LIST_MAX; i++) // SCAN WHOLE KEY LIST
    {
      if ( kpd.key[i].stateChanged )   // ONLY FIND CHANGED STATE KEYS
      {
        switch (kpd.key[i].kstate)
        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED

          case PRESSED:
          // SOUND BUZZER ....TURN OFF WHEN KEY RELEASED
            if (kpd.key[i].kchar == '*')
              tone(buzzer, clear_tone_freq);                 
            else if (kpd.key[i].kchar == '#')
              tone(buzzer, send_tone_freq);                 
            else if (kpd.key[i].kchar >= '0' && kpd.key[i].kchar <= '9')
              tone(buzzer, num_tone_freq);                 
            else if (kpd.key[i].kchar >= 'A' && kpd.key[i].kchar <= 'D')
              tone(buzzer, alpha_tone_freq);                 
            break;

          case RELEASED:
            noTone(buzzer);                    // SOUND OFF BUZZER
            if (kpd.key[i].kchar == '*')       // CLEAR ARRAY
            {
              for (int i = 0; i < 6; i++)
                bus_arr[i] = '\0';
              k = 0;
              send_flag = 0;
            }
            else if (kpd.key[i].kchar == '#')  // SEND ARRAY
            {
              if (send_flag == 1)
              {
                bus_arr[k] = '\0';
                send_flag = 0;
                k = 0;
                Serial.println(bus_arr);
                radio.write(&bus_arr, sizeof(bus_arr));     //SEND USING NRF TO RECEIVER
//                radio.printDetails();                       // RADIO DEBUG DETAILS
                delay(500);                                 // WAIT 0.5 SECONDS
              }
            }
            else              //  ADD TO ARRAY
            {
              if (k < 5)
              {
                bus_arr[k++] = kpd.key[i].kchar;
                send_flag = 1;
              }
            }
            break;
        }
      }
    }
  }
}




