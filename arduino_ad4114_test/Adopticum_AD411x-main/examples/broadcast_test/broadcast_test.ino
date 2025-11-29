/*
broadcast_test.ino
Example code to broadcast (empty) datagrams over UDP at a high transfer rate.

This example does not use the AD4116 device at all. 
Instead is it only meant to test the WiFi capabilities of the MCU.

## Test results

We can run this on different MCUs with no (or minor) code changes.
We have tested the code on the Arduino Uno R4 WiFi and the Raspberry Pi Pico W.
This has revealed som problems with the Arduino Uno R4 i.e. ArduinoCore-renesas.

### Performance problems

The Pico W can push out > 300 datagrams per second onto the WiFi network.
The Arduino Uno R4 WiFi can only push out about 9 datagrams per second.
On the Arduino R4 very little CPU resources remain for other tasks.

### SSID with space not working on Arduino Uno R4 WiFi

We tested on a WiFi network with a name (SSID) like this: "My wifi".
The Arduino Uno R4 WiFi cannot connect to an SSID with a space in the name.
This is not a problem on the Pico W or on an ESP32.


Created by Greger Burman, Adopticum, 2023.

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include "secrets.h"

namespace config {
	const char* Program = "Broadcast test";
	const char* Version = "2023-12-06";

	// Only REQUIRE serial connection to computer in debugging scenario.
	const bool SerialDebug = false;
	const long SerialSpeed = 115200;

	// Brodcast to every host on LAN on UDP port 50000.
	const char* broadcast_address = "255.255.255.255";
	const int src_port = 54321;

	const uint16_t buffer_count = 250;
}


namespace protocol_ad411x { 
	static const int udp_port = 54110;

	// Define a type for sending datagrams with samples.
	using datagram_t = struct {
		const char fourcc0 = 'A';
		const char fourcc1 = '4';
		const char fourcc2 = '1';
		const char fourcc3 = '1';
		uint16_t datagram_id;
		//TODO: implement channel.
		const uint16_t channel = 0; 
		const uint16_t sample_count = config::buffer_count;
		uint32_t samples[config::buffer_count];
		//TODO: implement CRC32.
		const uint32_t crc32 = 0; 
	};
}


IPAddress broadcast_ipaddr(255, 255, 255, 255);  // Default broadcast address.
WiFiUDP udp;
uint32_t t0;


void halt(const char* message) {
	Serial.println("Halt operation.");
	Serial.println(message);
	Serial.flush();
	while (1) {}
}


void blink(int count, int period = 500) {
	int half = period / 2;
	for (int i = 0; i < count; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(half);
		digitalWrite(LED_BUILTIN, LOW);
		delay(half);
	}
}


bool wifi_connect(const char ssid[], const char password[]) {
	if (WiFi.status() == WL_NO_MODULE) {
		halt("Communication with WiFi module failed!");
	}

	WiFi.disconnect();
	Serial.print("Connecting to ");
	Serial.print(ssid);
	Serial.print("...");

	// Connect to WPA/WPA2 network:
	WiFi.begin(ssid, password);
	for (auto i = 0; i < 10; i++) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println(" Connected.");
			return true;
		}
		Serial.print(".");
		blink(1, 1000);
	}
	return false;
}


void setup() 
{
	// Generic initialisation.
	if (config::SerialDebug) { while (!Serial) { delay(10); } }
	Serial.begin(config::SerialSpeed);
	Serial.println(config::Program);
	Serial.println(config::Version);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	// Parse broadcast address into an IPAddress.
	if (!broadcast_ipaddr.fromString(config::broadcast_address)) {
		halt("Failed to parse broadcast address.");
	}
	Serial.print("Broadcast to: ");
	Serial.println(broadcast_ipaddr);

	wifi_connect(secrets::wifi_ssid, secrets::wifi_password);

	// Initialize the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port.
	udp.begin(config::src_port);
	t0 = millis();
	Serial.println("End of setup.");
}


protocol_ad411x::datagram_t datagram;
uint32_t i = 0;

void loop() 
{
	// Use a delay to simulate time consumed by other work
	delay(2);

	// Automatic reconnection to WiFi if disconnected.
	// bool online = (WiFi.status() == WL_CONNECTED)
	// 	|| wifi_connect(secrets::wifi_ssid, secrets::wifi_password);
	// if (!online) {
	// 	Serial.println("Not connected to WiFi.");
	// 	return;
	// }

	// Send buffer over network.
	datagram.datagram_id++;
	udp.beginPacket(broadcast_ipaddr, protocol_ad411x::udp_port);
	udp.write((uint8_t*)&datagram, sizeof(datagram));
	udp.endPacket();

	// Debug output about the time consumed.
	if (datagram.datagram_id % 250 == 0) {
		auto t1 = millis();
		Serial.print("250 datagrams. 250 * 250 = 62 500 samples. Time consumed (ms): ");
		Serial.println(t1 - t0);
		Serial.print("Sent datagrams per s: ");
		Serial.println(250.0 * 1000 / (t1 - t0));
		t0 = t1;
	}
}