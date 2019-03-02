LIBRAIRIES UTILISEES DANS LE CODE DEFINITIF

Pour les librairies, il suffit de les ajouter dans l'environnement Arduino.

Croquis --> Inclure une bibliothèque --> Ajouter la bibliothèque


DHT : capteur T et HR

HX711 : ampli et convertisseur de signal des cellules => identique pour les 2 HX vert et rouge

SoftwareSerial : création d’une liaison série RX-TX dédiée au Bluetooth (au lieu des RX-TX (0-1) par défaut pour Arduino qui sont déjà reliés avec la prise USB et qui transmet des données au menteur série de l’IDE

Time : pour écrire des dates/heures