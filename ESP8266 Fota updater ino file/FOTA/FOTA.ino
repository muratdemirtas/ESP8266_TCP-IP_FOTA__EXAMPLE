//include all required function of esp8266 and arduino port libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

//use native ekspressif functions
extern "C" {
#include "user_interface.h"
#include "espconn.h"
}

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1

//Your access point settings, lets configure
const char* ssid = "Muhendis";
const char* password = "Murtiaxi133.";

//fota timer will start with esp8266 powered on
os_timer_t  fotaTimer;

//Wifi tcp server and client settings.
WiFiServer server(80);
WiFiClient serverClients[MAX_SRV_CLIENTS];

//start maximum sketch size(i use 1mb flash, 3mb SPIFFS)
uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

//esp8266 setup functions
void setup() {
	//Start serial with 115200 baudrate
	Serial.begin(115200);
	//Wait for open serial.
	while (!Serial) {}
	//wait for delay
	delay(100);

	//Connect to your access point
	WiFi.begin(ssid, password);

	Serial.print("\nConnecting to "); Serial.println(ssid);
	uint8_t i = 0;
	while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
	if (i == 21) {
		Serial.print("Could not connect to"); Serial.println(ssid);
		while (1) delay(500);
	}

	//start the server
	server.begin();
	server.setNoDelay(true);

	Serial.print("Ready! Use program with");
	Serial.print(WiFi.localIP());
	Serial.println(" 80' to connect");


	Serial.println("THIS IS YOUR VERSION 1.1 LETS DOWNGRADE.");
	//arm fota update timeout.
	os_timer_setfn(&fotaTimer, fotaCb, NULL);
	os_timer_arm(&fotaTimer, 40000, 0); Update.begin(maxSketchSpace);

	//start this fota service asynchronous
	Update.runAsync(true);
}

//Fota timer timeout callback, this will finish ota update
void ICACHE_FLASH_ATTR fotaCb(void *arg) {

	Update.end(true);
	Update.end();
}
void loop() {

	uint8_t i;
	//check if there are any new clients
	if (server.hasClient()) {
		for (i = 0; i < MAX_SRV_CLIENTS; i++) {
			//find free/disconnected spot
			if (!serverClients[i] || !serverClients[i].connected()) {
				if (serverClients[i]) serverClients[i].stop();
				serverClients[i] = server.available();
				Serial.print("New client: "); Serial.print(i);
				continue;
			}
		}
		//no free/disconnected spot so reject
		WiFiClient serverClient = server.available();
		serverClient.stop();
	}
	//check clients for data
	for (i = 0; i < MAX_SRV_CLIENTS; i++) {
		if (serverClients[i] && serverClients[i].connected()) {

			if (serverClients[i].available()) {

				while (serverClients[i].available()) {

					//READ BINARY DATA AND WRITE TO FLASH
					size_t k = serverClients[i].available();
					uint8_t * sbuf = (uint8_t *)malloc(k);
					serverClients[i].readBytes(sbuf, k);

					//INCREASE CPU TIME FOR WRITING FLASH.
					yield();

					//WHILE UPDATE IS NOT FINISHED
					if (!Update.isFinished()) {
						Update.write(sbuf, k);
						Update.printError(Serial);
					}

					//CLEAN BUFFER FOR NEW DATAS.
					free(sbuf);
					k = 0;
				}
		
			}
		}

	}
	if (Update.hasError())
		Update.printError(Serial);

	//IF UPDATE FINISHED SUCCESSFULLY THEN RESET ESP MODULE
	if (Update.isFinished()) {
		Update.printError(Serial);
		delay(2000);
		ESP.restart();
	}
}








