#include <DS3231.h>
#include <Wire.h>
#include <EEPROM.h>

//clock
DS3231 Clock;
byte Hour;
byte Minute;
byte Second;
bool Century = false;
bool h12;
bool PM;


//radio
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
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
                        igt_stop,
                        debug_on,
                        debug_off,
                        led_off};
enum command_payloads sendByte;
enum command_payloads gotByte;
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

bool send_data(const void *buf, uint8_t len){
  radio.stopListening();
  Serial.println("sending data");
  if(!radio.write( buf, len )){
    Serial.println("failed");
    radio.startListening();
    return false;
  }
  radio.startListening();
  return true;
}



//definicje
#define ZAPALNIK 5
#define ZAPALNIK_ON digitalWrite(ZAPALNIK, LOW);
#define ZAPALNIK_OFF digitalWrite(ZAPALNIK, HIGH);

#define PIEZZO 6
#define PIEZZO_ON digitalWrite(PIEZZO, HIGH);
#define PIEZZO_OFF digitalWrite(PIEZZO, LOW);

#define BLUE_LED 2
#define BLUE_LED_ON digitalWrite(BLUE_LED, HIGH);
#define BLUE_LED_OFF digitalWrite(BLUE_LED, LOW);

#define RED_LED 2
#define RED_LED_ON digitalWrite(RED_LED, HIGH);
#define RED_LED_OFF digitalWrite(RED_LED, LOW);

#define YELLOW_LED 2
#define YELLOW_LED_ON digitalWrite(YELLOW_LED, HIGH);
#define YELLOW_LED_OFF digitalWrite(YELLOW_LED, LOW);

#define OBWOD 9
#define A_SPLONKA A0
#define A_ZASILANIE A1
#define IRQ 2
#define DHT11 10
struct switches{
  byte debug:1,
       igt_ready:1,
       led_ind:1,
       silent:1;
}sw;
float adc;


void setup() {
  //other
  sw.debug = false;
  sw.led_ind = true;
  sw.silent = false;
  
  //pins conifg
  pinMode(ZAPALNIK, OUTPUT);
  pinMode(PIEZZO, OUTPUT);
  pinMode(OBWOD, INPUT);
  pinMode(A_SPLONKA, INPUT);
  pinMode(A_ZASILANIE, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  //def states
  ZAPALNIK_OFF
  PIEZZO_OFF
  BLUE_LED_OFF
  RED_LED_OFF
  YELLOW_LED_OFF
  
  //serial
  Serial.begin(9600);
  Serial.println("start....");
  printf_begin();

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
  YELLOW_LED_ON
  Serial.println("explosion sequenece started");
  send_cmd(exp_seq);
  for(int i = 0; i < period*10/2; i++){
    if(i%10 == 0) {
      Serial.print(i);
      radio.write(i, sizeof(i));
    }
    PIEZZO_ON
    delay(100);
    PIEZZO_OFF
    delay(100);
  }
  RED_LED_ON
  ZAPALNIK_ON
  Serial.println("ignition");
  send_cmd(igt_start);
  delay(3000);
  ZAPALNIK_OFF
  RED_LED_OFF
  YELLOW_LED_OFF
  send_cmd(igt_stop);
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
       radio.write( &sendByte, sizeof(sendByte));
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
    if(sw.debug) {
      PIEZZO_ON
      delay(50);
      PIEZZO_OFF
    }


    radio.read( &gotByte, 1);
    radio.writeAckPayload(pipeNo,&gotByte, sizeof(gotByte) );  
    Serial.println("got byte:");
    Serial.print(gotByte);
    if(gotByte == def){
      BLUE_LED_ON
      delay(50);
      BLUE_LED_OFF      
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
      Serial.println("exp_voltage: ");
      Serial.print(adc);
      adc = analogRead(A_SPLONKA) / 204.8 * 220.0 / 1220.0;
      send_data(adc, sizeof(adc));
    }else if(gotByte == get_igt_state){
      Serial.println("get_igt_state");
      digitalWrite(OBWOD, HIGH);
      send_data(digitalRead(OBWOD), 1);
      
    }else if(gotByte == set_igt_fire){
      Serial.println("set ig fire");
      if(state == igt_ready){
        explosion_seq(3);
      }
    }else if(gotByte == set_igt_ready){
      state = igt_ready;
      YELLOW_LED_ON
      Serial.println("set ig ready");
    }else if(gotByte == set_igt_stby){
      state = def;
      Serial.println("set def");
    }    else if(gotByte == debug_on){
      sw.debug = true;
      Serial.println("debug_on");
    }else if(gotByte == debug_off){
      sw.debug = false;
      Serial.println("debug_off");
    }else if(gotByte == set_led_off){
      pinMode(BLUE_LED, INPUT);
      pinMode(RED_LED, INPUT);
      pinMode(YELLOW_LED, INPUT);
      sw.led_ind = false;
      Serial.println("led_off");
    }else if(gotByte == led_on){
      pinMode(BLUE_LED, OUTPUT);
      pinMode(YELLOW_LED, OUTPUT);
      pinMode(RED_LED, OUTPUT);
      sw.led_ind = true;
      Serial.println("led_on");
      
    }
    
  }
  if ( state == def ) {
    
  }else if( state == igt_ready){

  }
}
