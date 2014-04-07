#include <SoftwareSerial.h>
#include "edf_reader.h"
#include "Akeru.h"

Akeru_ akeru;

Reading reading = {
  0, PEAK,{
    0,0,0,0,0,0,0,0,0,0          }
};

Measurement previousMeasure = {
  0,PEAK};

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
  Measurement measure = {
    0, 0, PEAK };

  Serial.println("update reading");

  while (!validFrame){
    waitForFrameBeginning();
    String frame = receiveFrame();
    validFrame = extractValues(frame,measure);
    Serial.println(frame);
  }

  return copyInto(reading, position, measure);
}

boolean extractValues(String frame, Measurement measure){

  int lineCount = countChars(frame, 0x0D)+1;
  short valuesDecoded = 0;

  for(int i = 0; i < lineCount; i++)
  {
    String line = extractString(frame, 0x0D, i);
    valuesDecoded += decodeLine(line, measure);
  }
  return valuesDecoded ==3;
}

short decodeLine(String line, Measurement measure){
  String label = extractString(line, ' ', 0);
  String value = extractString(line, ' ', 1);
  String checkSum = extractString(line, ' ', 2);
  short valueSet = 0;

  if(checksumOk(label, value, checkSum))
  {
    if(label.equals("PTEC") && value.equals("HP..")) {
      measure.unit = PEAK;
      valueSet++;
    }
    if(label.equals("PTEC") && value.equals("HC..")) {
      measure.unit = OFF_PEAK;
      valueSet++;
    }
    if(label.equals("HCHC")) {
      measure.hc = value.toInt();
      valueSet++;
    }
    if(label.equals("HCHP")) {
      measure.hp = value.toInt();
      valueSet++;
    }
  }
  return valueSet;
}

boolean checksumOk(String label, String value, String checksum) 
{
  unsigned char sum = 32;
  int i ;

  for (i=0; i < label.length(); i++) sum = sum + label.charAt(i) ;
  for (i=0; i < value.length(); i++) sum = sum + value.charAt(i) ;
  sum = (sum & 63) + 32 ;
  Serial.print(label);
  Serial.print(" ");
  Serial.print(value);
  Serial.print(" ");
  Serial.println(checksum);
  Serial.print("Sum = "); 
  Serial.println(sum);
  Serial.print("Cheksum = "); 
  Serial.println(int(checksum.charAt(0)));
  return sum == checksum.charAt(0);
}

String extractString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1            };
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int countChars(const String string, char ch)
{
  int count = 0;
  String s = string;

  while(s.indexOf(ch) != -1){
    count++;
    s = s.substring(s.indexOf(ch)+1);
  }
  return count;
}

Reading copyInto(Reading reading, int position, Measurement measurement){
  if (measurement.unit != previousMeasure.unit)  {
    reading.transitionIndex = position;
    reading.transitionDirection = measurement.unit;
  }
  reading.values[position] = measurement.unit==PEAK? measurement.hp : measurement.hc ;
  previousMeasure = measurement;
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

String receiveFrame(){
  String frame = "";
  char receivedChar ='\0';

  Serial.println("receiveFrame");

  while(receivedChar != 0x03) { 
    if (Serial.available()) {
      receivedChar = Serial.read() & 0x7F;
      frame+=receivedChar;
    }	
  }
  frame+='\0';
  return frame;
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









