#include <stdio.h>

/*
This code is under CC-BY-SA license, created by Edoardo Odorico and Lorenzo Farnararo
http://hack.lenotta.com
edo@lenotta.com
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include "../RF24.h";

using namespace std;
RF24 radio("/dev/spidev0.0",8000000 , 25);  //spi device, speed and CSN,only CSN is NEEDED in RPI
const int role_pin = 7;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup(void){
  //Prepare the radio module
  printf("\nPreparing interface\n");
  radio.begin();
  radio.setRetries( 15, 15);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);
  radio.setPALevel(RF24_PA_MAX);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.startListening();
  radio.printDetails();

}

bool switchLight(int action){
  //This function send a message, the 'action', to the arduino and wait for answer
  //Returns true if ACK package is received
  //Stop listening
  radio.stopListening();
  unsigned long message = action;
  printf("Now sending %lu...", message);

  //Send the message
  bool ok = radio.write( &message, sizeof(unsigned long) );
  if (ok)
    printf("ok...");
  else
    printf("failed.\n\r");
  //Listen for ACK
  radio.startListening();
  //Let's take the time while we listen
  unsigned long started_waiting_at = __millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout ) {
    __msleep(10);
    if (__millis() - started_waiting_at > 1000 )
      timeout = true;

  }

  if( timeout ){
    //If we waited too long the transmission failed
      printf("Failed, response timed out.\n\r");
      return false;
  }else{
    //If we received the message in time, let's read it and print it
    unsigned long got_time;
    radio.read( &got_time, sizeof(unsigned long) );
    printf("Got response %lu, round-trip delay: %lu\n\r",got_time,__millis()-got_time);
    return true;
  }

}  

int main( int argc, char ** argv){

    char choice;
    setup();
    bool switched = false;
    int counter = 0;

    //Define the options

    while(( choice = getopt( argc, argv, "f:")) != -1){

        if (choice == 'f'){

           	if( strcmp( optarg, "bedroom1on" ) == 0 ){

              printf("\nTurn 1 ooooon!\n");
              while(switched == false && counter < 5){

                // the switch light return true if the ACK package is received, 
                // If we do not receive the ACK package for 5 times in a row, then the transmission will stop.

                switched = switchLight(1);
                counter++;
              }
            }
            
        	if( strcmp( optarg, "bedroom2on" ) == 0 ){

              printf("\nTurn 2 ooooon!\n");
              while(switched == false && counter < 5){

                // the switch light return true if the ACK package is received, 
                // If we do not receive the ACK package for 5 times in a row, then the transmission will stop.

                switched = switchLight(2);
                counter++;
              }
			}
			
			if( strcmp( optarg, "bedroom1off" ) == 0 ){

              printf("\nTurn 1 oooooff!\n");
              while(switched == false && counter < 5){

                // the switch light return true if the ACK package is received, 
                // If we do not receive the ACK package for 5 times in a row, then the transmission will stop.

                switched = switchLight(3);
                counter++;
              }
			}
			
			if( strcmp( optarg, "bedroom2off" ) == 0 ){

              printf("\nTurn 2 oooooff!\n");
              while(switched == false && counter < 5){

                // the switch light return true if the ACK package is received, 
                // If we do not receive the ACK package for 5 times in a row, then the transmission will stop.

                switched = switchLight(4);
                counter++;
              }
           }else{

              printf("\nKill it without mercy!\n");
              while(switched == false && counter < 5){

                // the switch light return true if the ACK package is received, 
                // If we do not receive the ACK package for 5 times in a row, then the transmission will stop.

                switched = switchLight(0);
                counter++;
              }
           }

          }else{
            // A little help:
                printf("\n\rIt's time to make some choices...\n");
                printf("\n\rUse -f option: ");
                printf("\n[on1|On1|ON1] - will turn the light on.");
                printf("\n[Off1|*] -  guess what? It will turns the light off.\n ");
                printf("\n\rExample: ");
                printf("\nsudo ./switch -f on1\n");
          }

      //return 0 if everything went good, 2 otherwise
       if (counter < 5)
         return 0;
       else
         return 2;
   }
}