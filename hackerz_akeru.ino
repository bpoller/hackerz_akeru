#include <SoftwareSerial.h>
#include "Akeru.h"

Akeru_ akeru;

struct reading {
  long hc;
  long hp;
};

char CaractereRecu ='\0';
char Checksum[32] = "";
char Ligne[32]="";
char Etiquette[9] = "";
char Donnee[13] = "";
char Trame[512] ="";
int i = 0;
int j = 0;

long HCHP = 0;
long HCHC = 0; 
int IINST = 0;

int check[3];  // Checksum by etiquette
int trame_ok = 1; // global trame checksum flag
int finTrame=0;

void setup() {
  // LED 13 is used to "debug" sigfox operations
  pinMode(13, OUTPUT);

  // Initialize sigfox modem
  akeru.begin();

  // Start serial communication, because everyone needs to !
  Serial.begin(1200);
  delay(2000);
  Serial.println("Ready");
}

void loop() {
  getTeleinfo();
  sendData();
  delay(1000);
}

// Send data to Sigfox network
void sendData(){
  digitalWrite(13, HIGH);
  Serial.println("Sending...");
  reading consumption = {
    HCHC, HCHP  };
  if(akeru.send(&consumption, sizeof(consumption))){
    Serial.println("done");
  }
  else{
    Serial.println("not this time.");
  }
  digitalWrite(13, LOW);  
}

void getTeleinfo() {
  Serial.println("getting teleinfo");

  /* vider les infos de la dernière trame lue */
  memset(Ligne,'\0',32); 
  memset(Trame,'\0',512);
  memset(check,'\0',3);
  int trameComplete=0;
  HCHC = 0;
  HCHP = 0;
  IINST = 0;

  while (!trameComplete){
    // boucle jusqu'a "Start Text 002" début de la trame
    while(CaractereRecu != 0x02){
      if (Serial.available()) {
        CaractereRecu = Serial.read() & 0x7F;
      }
    }

    i=0;
    // Tant qu'on est pas arrivé à "EndText 003" Fin de trame ou que la trame est incomplète
    while(CaractereRecu != 0x03) { 
      if (Serial.available()) {
        CaractereRecu = Serial.read() & 0x7F;
        Trame[i++]=CaractereRecu;
      }	
    }

    finTrame = i;
    Trame[i++]='\0';
    lireTrame(Trame);	
    if(check[0] == 1 && check[1] == 1 && check[2] ==1){
      trameComplete = 1;
    }
  }

  Serial.print("HC: ");
  Serial.println(HCHC);
  Serial.print("HP: ");
  Serial.println(HCHP);
}

void lireTrame(char *trame){
  int i;
  int j=0;
  for (i=0; i < strlen(trame); i++){
    // Tant qu'on est pas au CR, c'est qu'on est sur une ligne du groupe
    if (trame[i] != 0x0D) {
      Ligne[j++]=trame[i];
    }
    else {
      //On vient de finir de lire une ligne, on la décode (récupération de l'etiquette + valeur + controle checksum
      decodeLigne(Ligne);
      // on vide la ligne pour la lecture suivante
      memset(Ligne,'\0',32);
      j=0;
    }
  }
}

int decodeLigne(char *ligne){ 
  int debutValeur; 
  int debutChecksum;

  debutValeur=lireEtiquette(ligne);  
  debutChecksum=lireValeur(ligne, debutValeur);
  lireChecksum(ligne, debutValeur + debutChecksum -1);

  if (checksum_ok(Etiquette, Donnee, Checksum[0])){
    return affecteEtiquette(Etiquette,Donnee);
  } 
  else return 0;
}

int lireEtiquette(char *ligne){
  int i;
  int j=0;
  memset(Etiquette,'\0',9);
  for (i=1; i < strlen(ligne); i++){ 
    if (ligne[i] != 0x20) {
      // Tant qu'on est pas au SP, c'est qu'on est sur l'étiquette
      Etiquette[j++]=ligne[i];
    }
    else { 
      //On vient de finir de lire une etiquette
      // on est sur le dernier caractère de l'etiquette, il faut passer l'espace aussi (donc +2) pour arriver à la valeur
      return j+2; 
    }

  }
}

int lireValeur(char *ligne, int offset){
  int i;
  int j=0;
  memset(Donnee,'\0',13);
  for (i=offset; i < strlen(ligne); i++){ 
    if (ligne[i] != 0x20) {
      // Tant qu'on est pas au SP, c'est qu'on est sur l'étiquette
      Donnee[j++]=ligne[i];
    }
    else { 
      //On vient de finir de lire une etiquette
      // on est sur le dernier caractère de la valeur, il faut passer l'espace aussi (donc +2) pour arriver à la valeur
      return j+2;
    } 
  }
}

void lireChecksum(char *ligne, int offset){
  int i;
  int j=0;
  memset(Checksum,'\0',32);
  for (i=offset; i < strlen(ligne); i++){ 
    Checksum[j++]=ligne[i];
  }
}

int affecteEtiquette(char *etiquette, char *valeur){
  if(strcmp(etiquette,"HCHP") == 0) { 
    HCHP = atol(valeur);
    check[0] = 1;
  }
  else
    if(strcmp(etiquette,"HCHC") == 0) { 
      HCHC = atol(valeur);
      check[1] = 1;
    }
    else
      if(strcmp(etiquette,"IINST") == 0) { 
        IINST = atoi(valeur);
        check[2] = 1;
      }
  return 0;
}

int checksum_ok(char *etiquette, char *valeur, char checksum) 
{
  unsigned char sum = 32 ;    // Somme des codes ASCII du message + un espace
  int i ;

  for (i=0; i < strlen(etiquette); i++) sum = sum + etiquette[i] ;
  for (i=0; i < strlen(valeur); i++) sum = sum + valeur[i] ;
  sum = (sum & 63) + 32 ;
  if ( sum == checksum) return 1 ;  // Return 1 si checkum ok.
  return 0 ;
}

