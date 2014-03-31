#include <Arduino.h>

// hcIndex - the index when a value shall be interpreted as 'Heure creuse'.
// hc = -1 means all values are HC
// hc = 10 means all values are HP
// hc = 3 means values 3 and onwards are HC. Values 0, 1 and 2 are HP.
struct Reading {
  int hcIndex;
  int values[10];
};
 
void sendData(Reading reading, int position);

Reading updateReading(Reading reading, int position);
