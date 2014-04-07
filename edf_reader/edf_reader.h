#include <Arduino.h>

/*

transitionIndex indicates the index of values where a transition from off-peak to 
peak tariff or vice-versa happened. Default value is -1, meaning no transition took 
place during the measurement intervall.

tariff 0 means downwards -> from peak tariff to off-peak tariff.
                     1 means upwards   -> from off peak to peak tariff.
                     
If transition index is -1, then transition direction indicates the current 
tariff that operates (0: off peak; 1: peak)

*/

struct Reading {
  short transitionIndex;
  short tariff;
  short values[10];
};

#define OFF_PEAK 0
#define PEAK 1

struct Measurement {
  int hc;
  int hp;
  short tariff;
};
 
void sendData(Reading reading, int position);

Reading updateReading(Reading reading, int position);
