sensorNet
=========

Arduino powered temperature and humidity sensors that report values to a Raspberry Pi using NRF2401+ modules.  
The master node (Raspberry Pi) receives the readings and stores them in a sqlite database.

Heavily inspired by http://blog.the-jedi.co.uk/2014/05/08/rf24network/

sensorMasterPi
--------------
- rfweather_net.cpp: Runs as a daemon and listens on its own NRFL2401+ module for network activity from the sensor nodes. Stores every reading in a temporary file for now (TODO: directly write to the sqlite database)  
Download and compile the RPi parts from https://github.com/TMRh20/RF24 and https://github.com/TMRh20/RF24Network  
Compile on the Raspberry Pi with: g++ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -L/usr/local/lib -I/usr/local/include -lrf24-bcm -lrf24network ./rf24weather_net.cpp -o rf24weather_net  

- rf24weather.py: Picks up new readings from the daemon and adds them to the sqlite database. 

sensorNode
----------
The sensor node reads the temperature and humidity every 30 minutes and goes into power save mode in between. 
Also the clock frequency is slowed to 1 MHz (from usually 16), except for when we read sensor data.
