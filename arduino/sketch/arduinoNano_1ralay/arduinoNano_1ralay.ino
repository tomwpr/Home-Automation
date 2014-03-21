sketch/* 
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

int relay1 = 8;			// the number of the relay1 pin
int button1Pin = 2;		// the number of the pushbutton1 pin
int state1 = LOW;		// the current state of the relay1
int reading1;			// variable for reading the pushbutton1 status
int previous1 = LOW;	        // the previous reading from the pushbutton1 

long time = 0;			// the last time the output pin was toggled
long debounce = 300;	        // the debounce time, increase if the output flickers

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
  pinMode(relay1, OUTPUT);              // pin for relay1 work mode
  pinMode(button1Pin, INPUT);	        // pin for button1 work mode
  digitalWrite(relay1, LOW);	
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
			digitalWrite(relay1, HIGH);
			state1 = HIGH;
	}
			        
	if (message == 10) {
			digitalWrite(relay1, LOW);
			state1 = LOW;
	}
				
        if (message == 0) {
			digitalWrite(relay1, LOW);
			state1 = LOW;
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

    // read the state of the pushbutton value:
    reading1 = digitalRead(button1Pin);

    // check if the pushbutton is pressed.
    if (reading1 == HIGH) {

      if (reading1 == HIGH && previous1 == LOW && millis() - time > debounce ) {
  
            if (state1 == LOW) {
      	            state1 = HIGH; 
            } else {
                    state1 = LOW;
            }
      	 
            time = millis();
      
      }
      
      digitalWrite(relay1, state1);      
  
    }

    previous1 = reading1;	

}
