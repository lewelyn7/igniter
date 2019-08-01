#include <DS3231.h>
#include <Wire.h>
#include <EEPROM.h>
DS3231 Clock;
byte Hour;
byte Minute;
byte Second;
bool Century = false;
bool h12;
bool PM;


#include <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
byte addresses[][7] = {"MASTER","node01"};
enum menu{ def, igt_ready, igt_stby, igt_fire } state;
enum gotdata { nrf, serial };
enum command_payloads { failed,
                        def, 
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

#define ZAPALNIK 5
#define ZAPALNIK_ON digitalWrite(ZAPALNIK, LOW);
#define ZAPALNIK_OFF digitalWrite(ZAPALNIK, HIGH);

#define PIEZZO 6
#define PIEZZO_ON digitalWrite(PIEZZO, LOW);
#define PIEZZO_OFF digitalWrite(PIEZZO, HIGH);

#define OBWOD 9
#define A_SPLONKA A0
#define A_ZASILANIE A1
#define IRQ 2
#define DHT11 10


RF24 radio(7,8);

void setup() {
  //pins conifg
  pinMode(ZAPALNIK, OUTPUT);
  pinMode(PIEZZO, OUTPUT);
  pinMode(OBWOD, INPUT);
  pinMode(A_SPLONKA, INPUT);
  pinMode(A_ZASILANIE, INPUT);
  //def states
  
  //serial
  Serial.begin(115200);
  Serial.println("start....");

  //radio
  Serial.println("starting radio");
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.enableAckPayload();               // Allow optional ack payloads
  radio.setRetries(0,15);                 // Smallest time between retries, max no. of retries
  radio.setPayloadSize(1);                // Here we are sending 1-byte payloads to test the call-response speed
  radio.openReadingPipe(1,addresses[0]);
  radio.openWritingPipe(addresses[1]); 
  radio.startListening();
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
 
  Serial.println("done");
  

  // put your setup code here, to run once:

}

void explosion_seq(int period){
  Serial.println("explosion sequenece started");
  for(int i = 0; i < period*10; i++){
    if(i%10 == 0) {
      Serial.print(i);
      radio.write(i, sizeof(i));
    }
    PIEZZO_ON
    delay(100);
    PIEZZO_OFF
    delay(100);
  }
  ZAPALNIK_ON
  Serial.println("ignition");
  sendByte = igt_start;
  radio.write( &sendByte, sizeof(sendByte));
  delay(3000);
  ZAPALNIK_OFF
  sendByte = igt_stop;
  radio.write( &sendByte, sizeof(sendByte));
  Serial.println("ignition stop");

}
void get_times(byte& Hour, byte& Minute, byte& Second) {
  Hour = Clock.getHour(h12, PM);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();
}

bool send_time(){
      radio.stopListening();
      get_times( Hour, Minute, Second);
      sendByte = get_time;
      bool send_state = true;
      send_state &= radio.write( &sendByte, sizeof(sendByte) );
      send_state &= radio.write( &Hour, sizeof(Hour) );
      send_state &= radio.write( &Minute, sizeof(Minute) );
      send_state &= radio.write( &Second, sizeof(Second) );

      if (!send_state){
       Serial.println(F("time send failed"));
       sendByte = failed;
       radio.write( sendByte, sizeof(sendByte));
      }
      radio.startListening();
      return send_state;
}
void set_time(DS3231 &Clock, byte Hour, byte Minute, byte Second){
  
  Clock.setClockMode(false);
  Clock.setHour(Hour);
  Clock.setMinute(Minute);
  Clock.setSecond(Second);
}
void loop() {
  while(radio.available(&pipeNo)){

    radio.read( &gotByte, 1);
    radio.writeAckPayload(pipeNo,&gotByte, sizeof(gotByte) );  
    
    if(gotByte == def){
      
    }else if(gotByte == get_state_all){
      //send_time();
      //temp
      //czas
      //obwod
      //napiecie
      //napiecie zasilania
      //
      
    }else if(gotByte == get_temp){
      
    }else if(gotByte == get_time){
      
    }else if(gotByte == get_pwr_volt){
      
    }else if(gotByte == get_exp_volt){
      
    }else if(gotByte == get_igt_state){
      
    }else if(gotByte == set_igt_fire){
      state = igt_fire;
    }
    
  }
  if ( state == def ) {
  }else if( state == igt_ready){
    if(state == igt_fire){
       explosion_seq(3);
       state = igt_ready;
    }
  }
}
