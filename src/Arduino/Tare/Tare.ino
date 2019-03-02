


#include "HX711.h" 

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

//Creation des objets HX711, loadCell_1 et loadCell_2 pour chaque FX 
HX711 loadCell_1; 
HX711 loadCell_2;

//Variables
double OffsetBrut_1 = 0;      // offset cell1
double poidsConnuBrut_1 = 0 ; // poids lu par cell1 (qui comprend l’Offset1)

double OffsetBrut_2 = 0;      // offset cell2
double poidsConnuBrut_2 = 0 ; // poids lu par le cell2 (qui comprend l’Offset2)

double poidsConnuReel = 0;    // valeur saisie pour la tare


void setup() {
//Lancement et configuration vitesse des liaisons series (PC-Arduino et Bluetooth)
 Serial.begin(9600);
  
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

  //Switch on HX711 / load cell
  loadCell_1.power_up();
  loadCell_2.power_up();

  // DEBUT DU PROTOCOLE DE TARE //
  //décharger la balance 
  Serial.println("Enlever toute charge de la balance : \t\t ");
  Serial.println("Si c est fait tapez G puis Entree pour continuer ... ");
  Serial.println();
  Serial.flush();
  //Attente d’une touche
  serial_read_byte();
  
  //OffsetBrut est la valeur brute du capteur pour "0 kg"
  OffsetBrut_1 = loadCell_1.read_average(20);
  //Afficher la valeur moyenne de 20 valeurs brutes pour 0kg
  Serial.print("Valeur moyenne brute n1 (sur 20 valeurs) pour 0kg : \t\t ");
  Serial.println(OffsetBrut_1);

  OffsetBrut_2 = loadCell_2.read_average(20);
  //Afficher la valeur moyenne de 20 valeurs brutes pour 0kg
  Serial.print("Valeur moyenne brute n2 (sur 20 valeurs) pour 0kg : \t\t ");
  Serial.println(OffsetBrut_2);


  delay(400);
  Serial.println();
  Serial.println();
  Serial.println();

  // poidsConnuBrut est la valeur brute du capteur pour une charge connue
  Serial.println("Poser une charge connue sur la balance : \t\t ");
  Serial.println("Entrer son poids en grammes... ");
  Serial.println();
  Serial.flush();
  //Lecture du port serie
  poidsConnuReel = serial_read_number();
  //Pour etre sur, afficher le poids tape au clavier
  Serial.print("Vous avez donne le poids suivant (en grammes) : \t\t ");
  Serial.println(poidsConnuReel);
  
  poidsConnuBrut_1 = loadCell_1.read_average(20);
  //Afficher la valeur moyenne de 20 valeurs brutes pour le poids proposé
  Serial.print("Valeur moyenne brute n1 (sur 20 valeurs) pour le poids propose : \t\t ");
  Serial.println(poidsConnuBrut_1);

  poidsConnuBrut_2 = loadCell_2.read_average(20);
  //Afficher la valeur moyenne de 20 valeurs brutes pour le poids proposé
  Serial.print("Valeur moyenne brute n2 (sur 20 valeurs) pour le poids propose : \t\t ");
  Serial.println(poidsConnuBrut_2);

  //A ce stade nous avons trois valeurs : poids reel connu vs poids mesuré pour les deux capteurs
  //ces valeurs sont utiliées dans le code ci dessous pour effectuer les corrections
  delay(250);
  Serial.println();
  Serial.println("   ---------------------------------");
  Serial.println();
  delay(250);

  Serial.println("Appuyer sur G puis entree pour continuer ...");
  Serial.flush();

  //Attente d’une touche
  serial_read_byte();

  //Switch off HX711 / load cell
  loadCell_1.power_down();
  loadCell_2.power_down();
  Serial.println();

}

void loop() {
  
Serial.print("OffsetBrut_1 : \t\t ");
Serial.println(OffsetBrut_1);
delay(1000);
Serial.print("OffsetBrut_2 : \t\t ");
Serial.println(OffsetBrut_2);
Serial.println();
delay(1000);

Serial.print("poidsConnuBrut_1 : \t\t ");
Serial.println(poidsConnuBrut_1);
delay(1000);
Serial.print("poidsConnuBrut_2 : \t\t ");
Serial.println(poidsConnuBrut_2);
Serial.println();
delay(1000);

Serial.print("poidsConnuReel : \t\t ");
Serial.println(poidsConnuReel);
Serial.println();
delay(5000);
Serial.println();
Serial.println("   ---------------------------------");
}





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








