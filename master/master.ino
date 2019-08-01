//radio
#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include <printf.h>
byte addresses[][7] = {"MASTER","node01"};
enum menu{ def, igt_ready, igt_stby, igt_fire } state;
enum gotdata { nrf, serial };
enum command_payloads { failed,
                        defa, 
                        get_state_all, 
                        get_time, 
                        get_temp, 
                        get_pwr_volt, 
                        get_exp_volt,
                        get_igt_state,
                        set_igt_stby,
                        set_igt_ready,
                        set_igt_fire,
                        exp_seq,
                        igt_start,
                        igt_stop};
command_payloads sendByte;
command_payloads gotByte;
byte pipeNo;
RF24 radio(7,8);
bool send_cmd(command_payloads sb){
  Serial.println("sending command");
  if(!radio.write( &sb, sizeof(sb) ){
    Serial.println("failed");
    return false;
  }
  return true;
}

//timer
#include <SeqTimer.h>
SeqTimer ping_timer;  

//definitions
#define BLUE_LED
#define BLUE_ON digitalWrite(BLUE, HIGH);
#define BLUE_OFF digitalWrite(BLUE, LOW);

#define GREEN_LED
#define RED_LED

//Serial
byte j = 0; //licznik na ilosc bajtow w buforze i do dlugosci napisu
boolean GotString = false;
void setup() {
  Serial.begin(115200);
  Serial.println("starting radio..");
  
  //radio
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,addresses[0]);
  radio.startListening();                 // Start listening
  radio.printDetails();  
  Serial.println("done");
  //timer
  ping_timer.init(100);
}

void loop() {
  if(ping_timer.getTimer()){
    unsigned long time = micros(); 
    if (!send_cmd(defa)){
      Serial.println(F("failed."));      
    }else{

      if(!radio.available()){ 
        Serial.println(F("Blank Payload Received.")); 
      }else{
        while(radio.available() ){
          unsigned long tim = micros();
          radio.read( &gotByte, sizeof(gotByte) );
          printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,tim-time);
        }
      }

    }    
  }
  while(Serial.available()){
    if(j == BUFFSIZE-1){
      Serial.println("przepelnienie bufora");
      j = 0;
    }
    input[j] = Serial.read();
    if(input[j] == '\n'){
      input[j+1] = '\0';
      GotString = true;
      PrintBuff(input);
    }else j++; 
  }
  if(GotString == true){  
    if(strcmp(input, "set_igt_ready\n") == 0){
      send_cmd(set_igt_ready);
    }else if( strcmp(input, "set_igt_fire")){
      send_cmd(set_igt_fire);
    }
  }


}
