#include <SoftwareSerial.h>
#include "edf_reader.h"
#include "Akeru.h"

Akeru_ akeru;

Reading reading = {
  10,{
    0,0,0,0,0,0,0,0,0,0      }
};

int position = 0;
int counter = 0;

unsigned long time;

void setup() {
  pinMode(13, OUTPUT);
  akeru.begin();
  Serial.begin(1200);
  time = millis();
  delay(1000);
  Serial.println("Ready to go...");
}

void loop() {
  sendData(updateReading(reading, position), position);
  position = tick(position, 1000);
}

// Send data to Sigfox network
void sendData(Reading reading, int position){
  if(position == 9)
  {
    digitalWrite(13, HIGH);
    Serial.println("Sending...");
    akeru.send(&reading, sizeof(reading));
    Serial.println("done");
    digitalWrite(13, LOW); 
  }
}

Reading updateReading(Reading reading, int position){

  char frame[512] = "";
  boolean isOffPeak = false;
  short counter = 0;

  while (!isValid(frame)){
    waitForFrameBeginning();
    receiveFrame(frame);
    isOffPeak = isOffPeakHours(frame);
    short counter = extractCount(frame, isOffPeak);
  }

  return copyInto(reading, position, isOffPeak,  counter);
}

boolean isOffPeakHours(char frame[]){
  return true;
}

short extractCount(char frame[], boolean isOffPeak)
{
  return 0;
}

Reading copyInto(Reading reading, int position, boolean isOffPeak, short counter){
  if(isOffPeak){
    reading.offPeakIndex = position;
  }
  reading.values[position] = counter;
  return reading;
}

boolean isValid(char frame[]){
  return true;
}

// Fast forward to the beginning of the next frame (0x02)
void waitForFrameBeginning(){
  char receivedChar ='\0';
  while(receivedChar != 0x02){
    if (Serial.available()) {
      receivedChar = Serial.read() & 0x7F;
    }
  }
}

void receiveFrame(char frame[]){
  int i=0;
  char receivedChar ='\0';

  while(receivedChar != 0x03) { 
    if (Serial.available()) {
      receivedChar = Serial.read() & 0x7F;
      frame[i++]=receivedChar;
    }	
  }
  frame[i++]='\0';
}

int tick(int position, int period){
  position = (position+1) % 10;
  unsigned long cycleTime = millis() - time;
  delay(period-cycleTime);
  time = millis();
  return position; 
}
