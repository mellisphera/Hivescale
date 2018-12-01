//The library used for arduino  https://github.com/bogde/HX711

//*******************************************
// CODE ARDUINO POUR UNE BALANCE A 4 POINTS
// Géraldune FARCY-MERLO + Lorenzo PONS
// Mai 2018
//*******************************************
//EVOLUTIONS
// -9/6/18 LPO : ajouter la boucle if pour adpter la frequence de mesure a quelques minutes
// -17/6/18 LPO : modifier le format du serialprint pour avoir un format constant compatible avec une lecture bluetooth
//                necessite quelques manips avec des strings pour gérer les floats
//*******************************************
// MODE D'EMPLOI 
// 1-inclure les librairies dans votre IDE
//     croquis > Inclure une bibliothèque > Ajouter une bib zip (select main folder)
//     vérifier que le slibrairies apparaissent en bas de la liste
// 2-choisir le port usb pour le televersement
//    outils > port 
// 3-vérifier le type de carte
//    outils > type de carte
// 4-Ouvrir le moniteur série
//      outils > moniteur serie
// 5-Televerser le code
//*******************************************
// WARNING : L'OUVERTURE DU MONITEUR PROVOQUE LE RESET DE LA CARTE => un nouvel étalonnage sera nécessaire
// Corollaire : la fermeture n'a pas d'impact, vous serez seulement dans le noir sans pouvoir monitorer les mesures
//*********************************************
// 6-Rendre autonome l'arduino avec un alimentation XXXX et le PROTOCOLE YY
//
// 7-Vérifier la continuité dans l'envoi de données avec votre smartphone
//   telecharger l'App Blu(e)Term
//*********************************************
#include <stdio.h> // for function sprintf
#include <string.h>
#include "HX711.h" 
#include "DHT.h"
#include "Time.h"
#include <SoftwareSerial.h> //Serial library
/*
 Arduino pin --> HX711_1 (rouge)
 7     ->   CLK_1
 8     ->   DOUT_1
 3.3V  ->   VCC
 5V    ->   VDD
 GND   ->   GND

 Arduino pin --> HX711_2 (vert)
 7     ->   CLK_2
 12    ->   DOUT_2
 5V    ->   VCC
 GND   ->   GND
*/

#define DOUT_1   8
#define CLK_1    7
#define DOUT_2  12
#define CLK_2    7


/**
 * Arduino connection HC-05 connection: 
 * HC-05  | Arduino
 * TX     | 11
 * RX     | 10
*/
//branchement des ports RXD et TXD du module Bluettooth sur les ports 10 et 11 de l'arduino
//une liaison RX/TX est forcement croisée donc la'affectation dans la fonction ci dessous prend les valeurs inversées
//SoftwareSerial bt (RX,TX)
SoftwareSerial bt (11,10);

//Broche et type de capteur de temperature
#define DHTPIN 2 
#define DHTTYPE DHT22 

//Creation des objets HX711, loadCell_1 et loadCell_2 pour chaque FX et DHT capteur de temperature
HX711 loadCell_1; 
HX711 loadCell_2;
DHT dht(DHTPIN, DHTTYPE);

//Variables 
double poidsBrut_1 = 0 ;      // poids brut cell1
double poidsReel_1 = 0;       // poids reel cell1

double poidsBrut_2 = 0 ;      // poids brut cell2
double poidsReel_2 = 0;       // poids reel cell2

double poidsReel = 0;         // poids final lu sur la balance

double poidsTest = 0;
double facteur = 2.0;         // Facteur pour le calcul du poids reel AVEC 4 PTS D APPUI

//Fonctions de lecture du port série
void serial_poll();           // Port serie OK
double serial_read_byte();    // Lecture de la touche avant de continuer
double serial_read_number();  // Lecture du poids tape

//*********************************************
//A remplir avec les valeurs venant du code Tare.ino

double OffsetBrut_1 = -1391871.00;      // offset cell1
double OffsetBrut_2 =  -19458.00;      // offset cell2

double poidsConnuBrut_1 = 685588.00 ; // poids lu par cell1 (qui comprend l’Offset1)
double poidsConnuBrut_2 = 843803.00 ; // poids lu par le cell2 (qui comprend l’Offset2)

double poidsConnuReel = 4000.00;    // valeur saisie pour la tare

//*********************************************
//La gestion de la frequence d'acquisition (IL FAUT LA BAISSER POUR LES TESTS!!)

long previousMillis = 0;        // will store last time scale was updated
long interval = 600000;          // interval at which to launch measurement (milliseconds) : tous les 10min

//*********************************************
// des chaines de caracteres pour passer t et h en string formate
char str_t[6];
char str_h[6];
char tbs[62]; // tableau de strings
long i0, i1, i2; //ints pour convertir le poids en entier pour le b-tooth. il faut que ce soit long et pas int car sinon en 2bytes on ne peut depasser 32000



void setup() {

  //Lancement et configuration vitesse des liaisons series (PC-Arduino et Bluetooth)
  Serial.begin(9600);  //le port serie pour le moniteur
  bt.begin(9600);      //le port serie pour le bluetooth
  
  //si on veut pas saturer à 6kg il faut ajuter le GAIN!
  //Le paramètre gain par défaut est 128 pour le canal A, on le passe à 64 pour le HX rouge et 32, soit le canal B, pour le HX vert
  //!Noter que le HX rouge peut descendre à 32 sur son canal B mais il faut faire une soudure
  //pour 5V en alim
  //A 128 la sortie du HX sature a 20mV, => equivaut à 6kg environ (cf. carac du capteur)
  //A 64 --> 40mV
  //A 32 --> 80mV 
  loadCell_1.begin(DOUT_1, CLK_1, 64) ; //cette loadcell aura une plage de fonctionnement plus étroite
  loadCell_2.begin(DOUT_2, CLK_2, 32) ;
  
  
  //Switch off HX711 / load cell
  loadCell_1.power_down();
  loadCell_2.power_down();

  delay(300) ;

}



void loop() {
 unsigned long currentMillis = millis();
  
 if(currentMillis - previousMillis > interval) {
  previousMillis = currentMillis;
  
  //MESURE DU POIDS *********************
  //Switch on HX711
  loadCell_1.power_up();
  loadCell_2.power_up();

  delay(400);

  //Afficher l'heure
  //digitalClockDisplay();
  
  //Valeur moyenne de 20 valeurs brutes
  poidsBrut_1 = loadCell_1.read_average(20);
  poidsBrut_2 = loadCell_2.read_average(20);

  //Calcul du poids
  poidsReel_1 = (((float)poidsBrut_1 - (float)OffsetBrut_1) * ((float)poidsConnuReel)/3) / ((float)poidsConnuBrut_1 - (float)OffsetBrut_1);
  poidsReel_2 = (((float)poidsBrut_2 - (float)OffsetBrut_2) * ((float)poidsConnuReel)/3) / ((float)poidsConnuBrut_2 - (float)OffsetBrut_2);

  poidsTest = poidsReel_1 + poidsReel_2; //le poids effectivement mesuré par les deux capteurs
  poidsReel = poidsTest*facteur; // pour une balance à 4 points le facteur vaut 2
  
  //Afficher les poids
  //Serial.println(" ");
  //Serial.print("Poids cell1 HX Rouge (en grammes) : \t\t "); //D8
  //Serial.println(poidsReel_1);
  //Serial.print("Poids cell2 HX vert (en grammes) : \t\t ");  //D12
  //Serial.println(poidsReel_2);
  //Serial.print("Poids total (en grammes) : \t\t ");
  //Serial.println(poidsReel);

  delay(300);

  //Switch off HX711 / load cell
  loadCell_1.power_down();
  loadCell_2.power_down();

  delay(300);
    
  //MESURE DE LA TEMPERATURE ET L'HUMIDITE *********************
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;}

  //Serial.print("Humidity: "); 
  //Serial.print(h);
  //Serial.print(" ");
  //Serial.print(" %\t");
  //Serial.print("Temperature: "); 
  //Serial.print(t);
  //Serial.print(" ");
  //Serial.print(" *C \n");
  //Serial.println();

  /*format printing for floats is not included into printf for arduino
  /*therefore it is needed to convert floats in strings
  /* dtostrf converts a float into a formated string
  /* 3 is mininum width, 2 is precision; float value is copied onto str_temp*/
  dtostrf(t, 3, 2, str_t);
  dtostrf(h, 3, 2, str_h);
  //converting floats into integers
  i0=poidsReel;  
  i1=poidsReel_1;
  i2=poidsReel_2;
  sprintf(tbs,"%05ld %05ld %05ld %s %s ", i0, i1, i2, str_t, str_h  );
  Serial.println(tbs);
  //exit format looks like
  //07650 02220 05430 40.20 50.20 


  //TRANSMISSION BLUETOOTH *************************************
  //ne pas oublier que pour le Bluetooth on a activé un port série spécifique qui s'applle "bt"
  //la transmission de données de l'Arduino vers le module Blueatooh se résume simplement à un "bt.print"
  //ensuite le module il se débrouille pour transmettre

// il est possible de visualiser les données sur un smartphone en installant une App de type "BlueTerm"
/*Test Telephone OU Raspi formatage adapté pour le teléphone*/
/*
 bt.print ("Poids Cell1 : " + String(poidsReel_1));
 bt.print("\n");
 bt.print ("Poids Cell2 : " + String(poidsReel_2));
 bt.print("\n");
 bt.print ("Poids total : " + String(poidsReel));
 bt.print("\n");
 bt.print ("Humidite : " + String(h));
 bt.print("\n");
 bt.print ("Temperature : " + String(t));
 bt.print("\n");
 bt.print("\n");
*/
 /*Test jusqu'a HiveEyes*/
   bt.println(tbs);

 // delay(500);
 }
}

// FIN DU PROGRAMME *******************






// FONCTIONS  *******************
//Fonctions de saisie et lecture du port série

// Poll serial port until anything is received
void serial_poll() {
  while (!Serial.available()) {
    // "yield" is not implemented as noop in older Arduino Core releases
    // See also: https://stackoverflow.com/questions/34497758/what-is-the-secret-of-the-arduino-yieldfunction/34498165#34498165
    #if !(ARDUINO_VERSION <= 106) || ARDUINO >= 10605 || defined(ESP8266)
    yield();
    #endif
  };
}

// Read byte from serial port
double serial_read_byte() {
  serial_poll();  
return Serial.read();
}


// Read numeric value from serial port
double serial_read_number() {
  serial_poll();
return Serial.parseInt();
}


