/*
Usage :
  + This is a simple Teleinfo Test program
  + It's just to test the TeleInfo board and its 
    connexion to the EDF counter
  + It just displays the received teleInfo data on
    the Serial Monitor
  + No Internet connexion, no data transfered

Hardware needed :
  + 1 x Arduino UNO r3
  + 1 x Opto Coupler : SFH620A
  + 1 x LED
  + 1 x 1 k? resistor
  + 1 x 4,7 k? resistor

PIN USED :
  PIN 8  : Software Serial RX
  PIN 9  : Software Serial TX

*/

// Nous utilisons la bibliothèque Software.Serial, ce  
// qui permet de configurer le port série sur d'autres
// pins que les habituelles 0 & 1 déjà utilisées par
// le "moniteur série"
#include <SoftwareSerial.h>

// Caractère de début de trame
#define startFrame 0x02
// Caractère de fin de trame
#define endFrame 0x03

// On crée une instance de SoftwareSerial
SoftwareSerial* cptSerial;

// Fonction d'initialisation de la carte Arduino, appelée
// 1 fois à la mise sous-tension ou après un reset
void setup()
{
  // On initialise le port utilisé par le "moniteur série".
  // Attention de régler la meme vitesse dans sa fenêtre
  Serial.begin(115200);
  
  // On définit les PINs utilisées par SoftwareSerial, 
  // 8 en réception, 9 en émission (en fait nous ne 
  // ferons pas d'émission)
  cptSerial = new SoftwareSerial(7, 9);
  // On initialise le port avec le compteur EDF à 1200 bauds :
  //  vitesse de la Télé-Information d'après la doc EDF
  cptSerial->begin(1200);
  Serial.println(F("setup complete"));
}

// Boucle principale, appelée en permanence une fois le 
// setup() terminé
void loop()
{
  // Variable de stockage des caractères reçus
  char charIn = 0;

  // Boucle d'attente du caractère de début de trame
  while (charIn != startFrame)
  {
    // on "zappe" le 8ème bit, car d'après la doc EDF 
    // la tramission se fait en 7 bits
    charIn = cptSerial->read() & 0x7F;
  }
  
  // Boucle d'attente d'affichage des caractères reçus, 
  // jusqu'à réception du caractère de fin de trame
  while (charIn != endFrame)
  {
    // S'il y a des caractères disponibles on les traite
    if (cptSerial->available())
    {
      // on "zappe" le 8ème bit
      charIn = cptSerial->read() & 0x7F;
      // on affiche chaque caractère reçu dans le 
      // "moniteur série"
      Serial.print(charIn);
    }
  }
  
  // après chaque fin de trame, on provoque un retour chariot
  Serial.println("");
}
