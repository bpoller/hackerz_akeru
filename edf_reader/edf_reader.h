#include <Arduino.h>

// offPeakIndex - the index when a value shall be interpreted as 'OffPeak'.
// offPeakIndex = 0 means all values are off peak (HC)
// offPeakIndex = 10 means all values are peak (HP)
// offPeakIndex = 3 means values 3 and onwards are off peak (HC). Values 0, 1 and 2 are peak HP.
struct Reading {
  short offPeakIndex ;
  short values[10];
};
 
void sendData(Reading reading, int position);

Reading updateReading(Reading reading, int position);
