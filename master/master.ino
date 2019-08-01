//radio
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
byte addresses[][7] = {"MASTER","node01"};
enum menu{ def, igt_ready, igt_stby, igt_fire, wait_igt_state } state;
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
  radio.stopListening();
  Serial.println("sending command");
  if(!radio.write( &sb, sizeof(sb) )){
    Serial.println("failed");
    radio.startListening();
    return false;
  }
  radio.startListening();
  return true;
}

//timer
#include <SeqTimer.h>
SeqTimer ping_timer;  

//definitions
#define BLUE_LED 9
#define BLUE_LED_ON digitalWrite(BLUE_LED, HIGH);
#define BLUE_LED_OFF digitalWrite(BLUE_LED, LOW);

#define GREEN_LED X
#define RED_LED X

//Serial
byte j = 0; //licznik na ilosc bajtow w buforze i do dlugosci napisu
boolean GotString = false;
#define BUFFSIZE 50
char input[BUFFSIZE];
void PrintBuff(char arr[]){
  for(int i = 0; i < BUFFSIZE; i++){
    Serial.print(arr[i]);
    if(arr[i] == '\n') return;
  }
}


bool ping_verbose = true;
void setup() {
  //serial
  Serial.begin(9600);
  Serial.println("starting radio..");
  printf_begin();

  //pins
  pinMode(BLUE_LED, OUTPUT);
  BLUE_LED_OFF
   
  //radio
  radio.begin();
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openWritingPipe(addresses[0]);        // Both radios listen on the same pipes by default, and switch when writing
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();                 // Start listening
  radio.printDetails();  
  Serial.println("done");
  //timer
  ping_timer.init(1000);
}

void loop() {
    while(radio.available(&pipeNo)){


    radio.read( &gotByte, 1);
    radio.writeAckPayload(pipeNo,&gotByte, sizeof(gotByte) );  
    Serial.println("got byte:");
    Serial.print(gotByte);
    if(state == def){
          if(gotByte == def){
            
          }else if(gotByte == get_state_all){
      
          }else if(gotByte == get_temp){
            
          }else if(gotByte == get_time){
            
          }else if(gotByte == get_pwr_volt){
            
          }else if(gotByte == get_exp_volt){
            
          }else if(gotByte == get_igt_state){
          }
    }else if(state == wait_igt_state){
      Serial.println("igt_state: ");
      Serial.print(gotByte);
      state = def;
    }
  }
  if(ping_timer.getTimer()){
    radio.stopListening();
    sendByte = defa;
    unsigned long time = micros(); 
    if(ping_verbose) Serial.println(F(" pingpong..."));  
    if (!radio.write( &sendByte, sizeof(sendByte))){
      if(ping_verbose) Serial.println(F(" pingpong failed."));      
      BLUE_LED_OFF
    }else{
      radio.startListening();
      if(!radio.available()){ 
        BLUE_LED_OFF
        if(ping_verbose) Serial.println(F("Blank Payload Received.")); 
      }else{
        while(radio.available() ){
          BLUE_LED_ON
          unsigned long tim = micros();
          radio.read( &gotByte, sizeof(gotByte) );
          if(ping_verbose) printf("Got response %d, round-trip delay: %lu microseconds\n\r",gotByte,tim-time);
        }
      }

    }    
  }
  if(Serial.available()){
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
    }else if( strcmp(input, "set_igt_fire\n") == 0){
      send_cmd(set_igt_fire);
    }
    else if( strcmp(input, "pingpong_v_off\n") == 0){
      ping_verbose = false;
    }
    else if( strcmp(input, "pingpong_v_on\n") == 0){
      ping_verbose = true;
    }
    else if( strcmp(input, "get_igt_state\n") == 0){
      send_cmd(get_igt_state);
      state = wait_igt_state;
      
    }
    //zerowanie flag
    GotString = false;
    j = 0;
  }


}
