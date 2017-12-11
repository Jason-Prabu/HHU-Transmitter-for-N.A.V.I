#include <Keypad.h>               // FOR KEYPAD
#include <SPI.h>                  // FOR NRF24LO1
#include <nRF24L01.h>
#include <RF24.h>                 //RF LIBRARY
#include <RF24_config.h>          // FOR DEBUG INFO
#include <printf.h>

//PIEZO-BUZZER TONE FREQUENCIES

#define SOS_TONE_FREQ   3000
#define CLEAR_TONE_FREQ 2000
#define SEND_TONE_FREQ  1500
#define PAGE_TONE_FREQ  1200
#define NUM_TONE_FREQ   600
#define ALPHA_TONE_FREQ 200


RF24 radio(7, 8);                                         // CE AND CSN PINS CONNECTED TO D7 AND D8
const uint64_t pipes[1] = {0xF0F0F0F0E1LL};               // TRANSMITTER ADDRESS

const int buzzer = 9;               // +VE TERMINAL OF BUZZER CONNECTED TO D9 THROUGHN RESISTOR

char bus_arr[6];
int k = 0, send_flag = 0, page_flag = 1;

//KEYPAD ROWS FIRST THEN COLUMN - LEFT TO RIGHT

const byte ROWS = 4;          // 4 ROWS
const byte COLS = 4;          // 4 COLUMNS

//KEY MAP PAGE 1
char keyspage1[ROWS][COLS] = {
  {'1', '2', '3', '4'},
  {'5', '6', '7', '8'},
  {'9', '0', 'A', 'B'},
  {'*', '#', '.', '/'}
};              
//KEY MAP PAGE 2                  
char keyspage2[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'G', 'H', 'I', 'J'},
  {'K', 'L', 'M', 'N'},
  {'*', '#', '.', '/'}
};          
//KEY MAP PAGE 3                      
char keyspage3[ROWS][COLS] = {
  {'O', 'P', 'Q', 'R'},
  {'S', 'T', 'U', 'V'},
  {'W', 'X', 'Y', 'Z'},
  {'*', '3', '.', '/'}
};                                

byte rowPins[ROWS] = {A0, A1, A2, A3}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keyspage1), rowPins, colPins, ROWS, COLS );


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
            if (kpd.key[i].kchar == '/')
              tone(buzzer, SOS_TONE_FREQ);     
            else if (kpd.key[i].kchar == '*')
              tone(buzzer, CLEAR_TONE_FREQ);                 
            else if (kpd.key[i].kchar == '#')
              tone(buzzer, SEND_TONE_FREQ);  
            else if (kpd.key[i].kchar == '.')
              tone(buzzer, PAGE_TONE_FREQ);                   
            else if (kpd.key[i].kchar >= '0' && kpd.key[i].kchar <= '9')
              tone(buzzer, NUM_TONE_FREQ);                 
            else if (kpd.key[i].kchar >= 'A' && kpd.key[i].kchar <= 'Z')
              tone(buzzer, ALPHA_TONE_FREQ);                 
            break;

          case RELEASED:
            noTone(buzzer);                    // SOUND OFF BUZZER
            // CLEAR ARRAY AND SET TO PAGE 1
            if (kpd.key[i].kchar == '*')       
            {
              for (int i = 0; i < 6; i++)
                bus_arr[i] = '\0';
              k = 0;
              send_flag = 0;
            }
            // SEND ARRAY
            else if (kpd.key[i].kchar == '#')  
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
            // SOS
            else if (kpd.key[i].kchar == '/')
            {
              
            }
            // CHANGE KEYS PAGE
            else if (kpd.key[i].kchar == '.')  
            {
              page_flag++;
              if (page_flag == 1 || page_flag > 3)
              {
                page_flag = 1;
                kpd = Keypad( makeKeymap(keyspage1), rowPins, colPins, ROWS, COLS );
              }
              else if (page_flag == 2)
                kpd = Keypad( makeKeymap(keyspage2), rowPins, colPins, ROWS, COLS );
              else if (page_flag == 3)
                kpd = Keypad( makeKeymap(keyspage3), rowPins, colPins, ROWS, COLS );
            }
            //  ADD TO ARRAY
            else              
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
