#include <SoftwareSerial.h>
#include "edf_reader.h"
#include "Akeru.h"

Akeru_ akeru;

Reading reading = {
  0, PEAK,{
    0,0,0,0,0,0,0,0,0,0                    }
};

Measurement measure = {
  0, 0, PEAK   };

Measurement previousMeasure = {
  0, 0,7};

int position = 0;

unsigned long time;

void setup() {
  pinMode(13, OUTPUT);
  akeru.begin();
  Serial.begin(1200);
  delay(1000);
  
  Serial.print("initializing system...");
  updateReading();
  Serial.println("done");
  
  digitalWrite(13, HIGH);
  delay(3000);
  digitalWrite(13, LOW);
  
  time = millis();
}

void loop() {
  updateReading();
  printReading();
  sendData();
  resetReading();
  position = tick(60000);
}

void resetReading()
{
  if(position == 9)
  {
    reading.transitionIndex = 0;
    for(int i = 0; i<10; i++){
      reading.values[i] = 0;
    }
  }
}

// Send data to Sigfox network
void sendData(){
  if(position == 9)
  {
    digitalWrite(13, HIGH);
    Serial.println("Sending...");

    akeru.send(&reading, sizeof(reading));
    Serial.println("done");
    digitalWrite(13, LOW); 
  }
}

void updateReading(){

  boolean validFrame = false;
  measure.hc = 0;
  measure.hp = 0;
  measure.tariff=PEAK;

  Serial.println("update reading");
  digitalWrite(13, HIGH);

  while (!validFrame){
    waitForFrameBeginning();
    String frame = receiveFrame();
    validFrame = extractValues(frame);
  }

  copyInto();
  digitalWrite(13, LOW);
}

void printReading(){

  Serial.print("transitionIndex: ");
  Serial.println(reading.transitionIndex);
  Serial.print("Tariff: ");
  Serial.println(reading.tariff);
  Serial.print("Consumption Wh: [");
  for(int i = 0; i < 9;i++){
    Serial.print(reading.values[i]);
    Serial.print(",");
  }
  Serial.print(reading.values[9]);
  Serial.println("]");
}

boolean extractValues(String frame){

  int lineCount = countChars(frame, 0x0D)+1;
  short valuesDecoded = 0;

  for(int i = 0; i < lineCount; i++)
  {
    String line = extractString(frame, 0x0D, i);
    valuesDecoded += decodeLine(line);
  }
  return valuesDecoded ==3;
}

short decodeLine(String line){
  String label = extractString(line, ' ', 0).substring(1);
  String value = extractString(line, ' ', 1);
  String checkSum = extractString(line, ' ', 2);
  short valueSet = 0;

  if(checksumOk(label, value, checkSum))
  {
    if(label.equals("PTEC") && value.equals("HP..")) {
      measure.tariff = PEAK;
      valueSet++;
    }
    if(label.equals("PTEC") && value.equals("HC..")) {
      measure.tariff = OFF_PEAK;
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
  boolean ok = sum == checksum.charAt(0);
  return ok;
}

String extractString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {
    0, -1                      };
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

void copyInto(){
  // the system is initialized
  if(measure.tariff != 7)
  {
    if (measure.tariff != previousMeasure.tariff)  {
      reading.transitionIndex = position;
      reading.tariff = measure.tariff;
    }
    reading.values[position] = measure.tariff==PEAK? measure.hp - previousMeasure.hp: measure.hc - previousMeasure.hc ;
  }
  previousMeasure = measure;
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

String receiveFrame(){
  String frame = "";
  char receivedChar ='\0';
  while(receivedChar != 0x03) { 
    if (Serial.available()) {
      receivedChar = Serial.read() & 0x7F;
      frame+=receivedChar;
    }	
  }
  frame+='\0';
  return frame;
}

int tick(unsigned long period){
  position = (position+1) % 10;
  unsigned long cycleTime = millis() - time;
  Serial.print("Cycle time:");
  Serial.println(cycleTime);

  if(cycleTime<period){

    long waitTime = period-cycleTime;

    while(waitTime > 30000)
    {
      delay(30000);
      waitTime -=30000;  
    }
    delay(waitTime);
  }
  time = millis();
  return position; 
}














