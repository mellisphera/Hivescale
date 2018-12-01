# CODE POUR RELEVER ARDUINO PAR BLE ET ENVOYER DONNEES A HIVEEYES
# 17/6/18 : modification du format de lecture bluetooth pour qu il soit compatible 


# WARNING : ne pas utiliser des caracteres accentues, sinon il ne s execute pas


import paho.mqtt.client as mqtt
import json
import bluetooth


#--------Bluetooth---------
bd_addr = "98:D3:31:F5:1D:87" #HC-05 internal address (see bluetoothctl)
port = 1
sock = bluetooth.BluetoothSocket (bluetooth.RFCOMM)
sock.connect((bd_addr,port))
print("...connexion a bluetooth ok")

#--------------------------


#Variables
data=""
sensor_dataP = {"Poids": 0}
sensor_dataP1 = {"PoidsCell1" : 0}
sensor_dataP2 = {"PoidsCell2" : 0}
sensor_dataT = {"Temperature": 0}
sensor_dataH = {"Humidite" : 0}

#---------MQTT-------------
lemaBkr_hiveeyes = mqtt.Client() #vers Hiveeyes
lemaBkr_local = mqtt.Client()    #vers le PC en local (une fois installe mosquitto on peut activer l ecoute a partir d un terminal  : mosquitto_sub  -t "lecanal/#")

username = "lorenzo.pons@free.fr"
password = "LarvEigEdsinLyg"
#ATTENTION si j utilise mon compte il faut envoyer les donnees a une instance qui m'appartienne
#Owner-ID: eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c 
#sinon le systeme detecte que c est qqn d autre qui fait les modis et trouve des conflits

#username = "geraldine@lema-bkr.com"
#password = "ontAwvyoHidar"
#Owner-ID : 57eaf23a-968f-4740-b747-137e3ab00580
lemaBkr_hiveeyes.username_pw_set(username, password) 


lemaBkr_hiveeyes.connect('swarm.hiveeyes.org', 1883) #Broker, indique ou se trouve le client (destination) et port
lemaBkr_hiveeyes.loop_start()

#lemaBkr_local.connect('192.168.0.105', 1883) #IP du PC pour l'avoir en local
#lemaBkr_local.connect('192.168.1.5', 1883) #IP du Raspberry pour l'avoir en local
lemaBkr_local.loop_start()
print("...connexion a hiveeyes ok")

#--------------------------
count = 0
try:
    while True:
	data += sock.recv(1024)    #Recuperation des donnees bluetooth
	data_end = data.find('\n') #Fin de donnee signalee par /n => taille du tableau
	if data_end != -1:	       #Si data est different de -1 alors repartition dans Poids et Temperature
		rec = data[:data_end]
	    #print data             #Imprimme le tableau des donnees, toutes collees les unes avec les autres
		sensor_dataP['Poids'] = data[0:5]
		sensor_dataP1['PoidsCell1'] = data[6:11]
		sensor_dataP2['PoidsCell2'] =  data[12:17]
		sensor_dataT['Temperature'] =  data[18:23]
		sensor_dataH['Humidite'] =  data[24:29]
		data = data[data_end+1:]
	
	print("...recuperation des donnees bth ok")

	#pour une raison mysterieuse les donnees sont printees 4 fois a l identique
	#alors je lui dis de les envoyer une fois sur 4 (cest pas beau mais tant pis)
	count = count+1
	if count == 4 : 
	  count = 0
	  #Envoyer via MQTT (topic et conversion des donnees envoyees en json)
	  #realm/network/gateway/node
	  #Pour verifier que les donnees arrivent au serveur
	  #    mosquitto_sub -h swarm.hiveeyes.org -p 1883 -t 'hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/#' -v
	  lemaBkr_hiveeyes.publish('hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/scale01/data.json', json.dumps(sensor_dataP))
	  lemaBkr_hiveeyes.publish('hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/scale01/data.json', json.dumps(sensor_dataP1))
	  lemaBkr_hiveeyes.publish('hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/scale01/data.json', json.dumps(sensor_dataP2))
	  lemaBkr_hiveeyes.publish('hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/scale01/data.json', json.dumps(sensor_dataT))
	  lemaBkr_hiveeyes.publish('hiveeyes/eb87a9f4-b25a-4c7c-a29c-c05bb80f1f2c/lemabkr/scale01/data.json', json.dumps(sensor_dataH))

	print("...envoi a hiveeyes ok")

	  #lemaBkr_local.publish('lecanal/parlequel/estenvoyee/data.json', json.dumps(sensor_dataP))
	  #lemaBkr_local.publish('lecanal/parlequel/estenvoyee/data.json', json.dumps(sensor_dataP1))
	  #lemaBkr_local.publish('lecanal/parlequel/estenvoyee/data.json', json.dumps(sensor_dataP2))
	  #lemaBkr_local.publish('lecanal/parlequel/estenvoyee/data.json', json.dumps(sensor_dataT))
	  #lemaBkr_local.publish('lecanal/parlequel/estenvoyee/data.json', json.dumps(sensor_dataH))
	
	  #Afficher dans le terminal	
	print(sensor_dataP)
	print(sensor_dataP1)
	print(sensor_dataP2)
	print(sensor_dataT)
	print(sensor_dataH)
	print

except KeyboardInterrupt:
    pass

lemaBkr_hiveeyes.loop_stop()
lemaBkr_hiveeyes.disconnect()
lemaBkr_local.loop_stop()
lemaBkr_local.disconnect()
