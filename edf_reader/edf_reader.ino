#include <SoftwareSerial.h>
#include "edf_reader.h"


Reading reading = {
  0,{
    0,0,0,0,0,0,0,0,0,0    }
};

int position = 0;
int counter = 23;

unsigned long time;

void setup() {

  Serial.begin(1200);
  time = millis();
}

void loop() {
  reading = updateReading(reading, position);
  sendData(reading, position);
  position = tick(position, 1000);
}

// Send data to Sigfox network
void sendData(Reading reading, int position){
  
  if(position == 9)
  { 
    Serial.println("-------------");
    Serial.print("hcIndex");
    Serial.println(reading.hcIndex);
    Serial.println("-------------");
    for(int i = 0; i < 10; i++)
    {
      Serial.print("value ");
      Serial.print(i);
      Serial.print(":");
      Serial.println(reading.values[i]);
    }
  }
}

Reading updateReading(Reading reading, int position){
  reading.values[position] = counter++;
  return reading;
}

int tick(int position, int period){
  position = (position+1) % 10;
  unsigned long cycleTime = millis() - time;
  delay(period-cycleTime);
  time = millis();
  return position; 
}


