#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <string.h>

//Used to store large strings in progmem.
char p_buffer[80];
#define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)

//  Ethernet Variables
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };  // MAC address for the ethernet controller.

EthernetClient client;
SoftwareSerial* cptSerial;
unsigned long time;

struct Measurement {
  int hp;
  int hc;
  String tariff;
};

Measurement measure = {0, 0, ""};
Measurement previousMeasure = {0, 0, ""};
Measurement reading =  {0, 0, ""};

void setup() {
  //Ethernet.begin(mac);
  Serial.begin(9600);
  
  cptSerial = new SoftwareSerial(7, 9);
  cptSerial->begin(1200);
  
  delay(1000);
  
  Serial.print(P("initializing system..."));
  updateReading();
  Serial.println(P("done"));
  
  Serial.println(P("ready"));
  time = millis();
}

void loop() {
  updateReading();
  send(12,"PEAKO");
  tick(5000);
}

void send(int count, String HPHC){

  String data = "{\"measurement\":" +String(count)+",\"tariff\":\""+HPHC+"\"}";
  
  Serial.println(data);
  /*
  if(client.connect("178.62.86.159",9200)){
    Serial.println("connected!");
    
    client.println("POST /edf/measurement_2 HTTP/1.1");
    client.println("Host: 178.62.86.159:9200");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
    client.stop();
  }
  else{
    Serial.println("problems connecting!");
  }
  
  */
}

void tick(unsigned long period){
  unsigned long cycleTime = millis() - time;
  Serial.print(P("Cycle time:"));
  Serial.println(String(cycleTime));
  if(cycleTime<period){
    long waitTime = period-cycleTime;
    delay(waitTime);
  }
  time = millis();
}

void updateReading(){
  boolean validFrame = false; 
  Serial.println(P("update reading"));
 
  while (!validFrame){
    waitForFrameBeginning();
    String frame = receiveFrame();
    validFrame = extractValues(frame);
  }
  copyInto();
}

void copyInto(){
  // the system is initialized
  if(measure.tariff != "")
  {
    reading.tariff = measure.tariff;
    reading.hp = measure.hp - previousMeasure.hp;
    reading.hc = measure.hc - previousMeasure.hc;
  }
  previousMeasure = measure;
}

// Fast forward to the beginning of the next frame (0x02)
void waitForFrameBeginning(){
  Serial.println(P("Wait for frame"));
  char receivedChar ='\0';
  while(receivedChar != 0x02){
    if (cptSerial->available()) {
      receivedChar = cptSerial->read() & 0x7F;
      Serial.println(receivedChar);
    }
  }
}

String receiveFrame(){
  String frame = "";
  char receivedChar ='\0';
  while(receivedChar != 0x03) { 
    if (cptSerial->read()) {
      receivedChar = cptSerial->read() & 0x7F;
      frame+=receivedChar;
      Serial.print(receivedChar);
    }	
  }
  frame+='\0';
  Serial.println("");
  return frame;
}

boolean extractValues(String frame){

  int lineCount = countChars(frame, 0x0D)+1;
  short valuesDecoded = 0;

  for(int i = 0; i < lineCount; i++)
  {
    String line = extractString(frame, 0x0D, i);
    valuesDecoded += decodeLine(line);
  }
  return valuesDecoded == 3;
}

String extractString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
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

short decodeLine(String line){
  String label = extractString(line, ' ', 0).substring(1);
  String value = extractString(line, ' ', 1);
  String checkSum = extractString(line, ' ', 2);
  short valueSet = 0;

  if(checksumOk(label, value, checkSum))
  {
    if(label.equals("PTEC") && value.equals("HP..")) {
      measure.tariff = "PEAK";
      valueSet++;
    }
    if(label.equals("PTEC") && value.equals("HC..")) {
      measure.tariff = "OFF_PEAK";
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
