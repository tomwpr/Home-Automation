/* 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */
 /*
  Hack.lenotta.com
  Modified code of Getting Started RF24 Library
  It will switch a relay on if receive a message with text 1, 
  turn it off otherwise.
  Edo
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

int relay = 8;	            // the number of the relay pin
int trigger = 5;	    // the number of the trigger pin
int relayStatus = LOW;	    // the current state of the relay
int triggerStatus = 0;      // variable for trigger (button, motion sensor etc.) status
int lastUser = 0;           // variable for last user that have changed relay status (0 = doesn't matter, 1 = motion, 2 = remote)
unsigned long time;         // variable for delay time

//
// Hardware conf
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0F2LL, 0xF0F0F0F0D2LL };

void setup(void)
{

  Serial.begin(57600);
  pinMode(relay, OUTPUT);              // relay pin work mode
  pinMode(trigger, INPUT);	       // trigger pin work mode
  digitalWrite(relay, LOW);	
  printf_begin();
  printf("\nLight Switch Arduino\n\r");

  // Setup and configure RF radio
  radio.begin();
  radio.setRetries(15,15);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();
}

void loop(void)
{

    // read the state of the pushbutton value:
    triggerStatus = digitalRead(trigger);
    relayStatus = digitalRead(relay);

    // check if the pushbutton is pressed.
    if (triggerStatus == HIGH && relayStatus == LOW) {
        relayStatus = HIGH;
        lastUser = 1;
        time = millis();
    } 
    
    else if (triggerStatus == HIGH && relayStatus == HIGH && lastUser == 1 ) {
        time = millis();
    }
    
    else if (triggerStatus == LOW && relayStatus == HIGH && lastUser != 2 && millis() - time >= 180000 ) {
        relayStatus = LOW;
        lastUser = 1;
    }
      
    digitalWrite(relay, relayStatus);
    
    
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long message;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &message, sizeof(unsigned long) );

        // Spew it
        printf("Got message %lu...",message);
        if (message == 11) {
			digitalWrite(relay, HIGH);
			relayStatus = HIGH;
                        lastUser = 2;
	}
			        
	if (message == 10) {
			digitalWrite(relay, LOW);
			relayStatus = LOW;
                        lastUser = 2;
	}
				
        if (message == 0) {
			digitalWrite(relay, LOW);
			relayStatus = LOW;
                        lastUser = 2;
        }

		// Delay just a little bit to let the other unit
		// make the transition to receiver
		delay(20);
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &message, sizeof(unsigned long) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }

}
