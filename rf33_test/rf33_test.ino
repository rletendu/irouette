#include "oregon.hpp"
#include <x10rf.h>

#define BP_PIN 2

#define TX_PIN 10
Oregon rf_sender = Oregon();
x10rf myx10 = x10rf(TX_PIN,0,1);
byte old=HIGH;

void setup()
{
  pinMode(BP_PIN,INPUT_PULLUP);
  Serial.begin(9600);
  rf_sender.begin(TX_PIN);
  myx10.begin();
  Serial.println("Tx Temperature");
  rf_sender.send_temperature(0x20, 0xCB, 11.2, 1);

}

void loop()
{
  byte state;
  delay(100);
  state = digitalRead(BP_PIN);
  if (state != old) {
    Serial.print("Tx BP:");Serial.println(state);
    if (state == HIGH) {
     myx10.x10Switch('B',4, 1); 
    } else {
      myx10.x10Switch('B',4, 0); 
    }
  }
  old = state;

 
  /*
  rf_sender.send_temperature_hum(0x20, 0xCB, 11.2, 52,1);
  delay(3000);
  */
}


#if 0
#include <x10rf.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define tx 0           // Pin for 433MHz transmitter
#define reps 1         // Repeats for RF-command
#define ledpin 4       // Pin for transmit led (0 = don't blink)

const int blinks_per_unit = 1000;       // Number of blinks per 1 unit (kW, m3 etc.)
const int transmit_interval = 31;       // Number of watchdog intervals between transmits (each interval is ~2 sec, needs adjustment for each chip)
const int current_multiplier = 6;       // Number to multiply the amount of blinks the last minute to get an hours consumption. (Normally 60, set to 6 here as I am counting 0.1 units).

x10rf x10 = x10rf(tx, ledpin, reps);    // x10 transmit object

unsigned long totalUnits = 0;           // Counter for total number of units used
int counterUnits = 0;                   // Temporary counter for calculation of total consumption

volatile long totalCounter = 0;         // Counter for current consumption
volatile int wdCounter = 0;             // Counter for current consumption
volatile int currentCounter = 0;        // Counter for current consumption

void setup() {
  resetWatchdog ();  // do this first in case WDT fires

  x10.begin();

  pinMode(0, OUTPUT);  // Set pins, all unused pins to input for power savings
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, OUTPUT);

  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off

  if(ledpin > 0) {
    for (int k = 0; k < 8; k = k + 1) {
      digitalWrite(ledpin, !digitalRead(ledpin));   // Run a blink sequence to show the circuit lives on boot
      delay(100);
    }
  }

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void transmit() {
  x10.RFXmeter(11, 0, totalUnits);                            // Transmit total units consumed
  x10.RFXmeter(12, 0, currentCounter * current_multiplier);   // Transmits current unit consumption
  currentCounter = 0;                                         // Resets counter for last interval
}

void sleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement
  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  MCUSR = 0;
  sleep_cpu();                            // sleep

  sleep_disable();                        // Clear SE bit
}

void resetWatchdog ()
{
  MCUSR = 0;                                                  // clear various "reset" flags
  WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);                // allow changes, disable reset, clear existing interrupt
                                               // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
  WDTCR = bit (WDIE) | bit (WDP2) | bit (WDP1) | bit (WDP0);  // set WDIE, and 2 seconds delay
  wdt_reset();                                                // pat the dog
}  // end of resetWatchdog

void loop() {
  if (wdCounter >= 	) {  // Check if we have run the required number of wakeups
    wdCounter -= transmit_interval;
    transmit();                          // Run radio transmission
  }

  if (totalCounter >= blinks_per_unit) {
    totalCounter -= blinks_per_unit;
    totalUnits++;                       // Increases total counter unit
  }

  sleep();
}

ISR(PCINT0_vect) {
  if (digitalRead(2) == HIGH) {
    totalCounter++;                        // Increase counter for total current consumption
    currentCounter++;
  }
}

ISR(WDT_vect) {
  resetWatchdog();
  wdCounter++;
}

#endif
