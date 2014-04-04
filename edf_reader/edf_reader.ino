#include <SoftwareSerial.h>
#include "edf_reader.h"
#include "Akeru.h"

Akeru_ akeru;

Reading reading = {
  0, 0,{
    0,0,0,0,0,0,0,0,0,0        }
};

Values previousValues = {0,0,0};

int position = 0;

unsigned long time;

void setup() {
  pinMode(13, OUTPUT);
  akeru.begin();
  Serial.begin(1200);
  delay(1000);
  Serial.println("Ready to go...");
  time = millis();
}

void loop() {
  sendData(updateReading(reading, position), position);
  position = tick(position, 5000);
}

// Send data to Sigfox network
void sendData(Reading reading, int position){
  if(position == 9)
  {
    digitalWrite(13, HIGH);
    Serial.println("Sending...");
    //akeru.send(&reading, sizeof(reading));
    Serial.println("done");
    digitalWrite(13, LOW); 
  }
}

Reading updateReading(Reading reading, int position){

  boolean validFrame = false;
  char frame[512] = "";
  short counter = 0;
  Values values = {0, 0, false};

  Serial.println("update reading");

  while (!validFrame){
    waitForFrameBeginning();
    receiveFrame(frame);
    validFrame = extractValues(frame,values);
    printFrame(frame);
  }

  return copyInto(reading, position, values);
}

boolean extractValues(char frame[], Values values){

  
  
  return true;
}

void printFrame(char frame[]){
  for (int i=0; i < strlen(frame); i++){
    Serial.write(frame[i]);
  }
}

Reading copyInto(Reading reading, int position, Values currentValues){
  if (currentValues.isPeak != previousValues.isPeak)  {
    reading.transitionIndex = position;
    reading.transitionDirection = currentValues.isPeak;
  }
  reading.values[position] = (currentValues.isPeak==1)? currentValues.hp : currentValues.hc;
  previousValues = currentValues;
  return reading;
}

// Fast forward to the beginning of the next frame (0x02)
void waitForFrameBeginning(){
  char receivedChar ='\0';

  Serial.println("wait for beginning...");
  while(receivedChar != 0x02){
    if (Serial.available()) {
      receivedChar = Serial.read() & 0x7F;
    }
  }
}

void receiveFrame(char frame[]){
  int i=0;
  char receivedChar ='\0';

  Serial.println("receiveFrame");

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
  Serial.print("Cycle time:");
  Serial.println(cycleTime);
  
  delay(period-cycleTime);
  time = millis();
  return position; 
}

