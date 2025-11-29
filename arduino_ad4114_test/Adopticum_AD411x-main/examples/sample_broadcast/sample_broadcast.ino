/*
Simple sketch to connect to Wifi from an Arduino Uno R4 Wifi
and stream some data over UDP.
*/

#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include <Adopticum_AD411x.h>

#include "secrets.h"

namespace config {
	const char* Program = "Sample broadcaster";
	const char* Version = "2023-12-06";

	// Only REQUIRE serial connection to computer in debugging scenario.
	const bool SerialDebug = false;
	const long SerialSpeed = 115200;

	// Brodcast to every host on LAN on UDP port 50000.
	const char* broadcast_address = "255.255.255.255";
	const int src_port = 54321;

	// Pin on Arduino connected to chip select on AD411x.
	const int CS_PIN = 10;
	// Pin on Arduino to trigger interrupt when data is ready.
	// Connect DRDY pin to CIPO (a.k.a. MISO) pin.
	const int DRDY_PIN = 3;  
	// Select an output data rate for the AD4116.
	const uint16_t ODR = (uint16_t)AD4116::OutputDataRate::SPS_10416;

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


void setup_ad4116() 
{
	ad4116.setup(config::CS_PIN);
	if (!ad4116.begin() || !ad4116.check_id()) {
		halt("Could not initialize AD411x device.");
	}
	
	// Reset and configure AD411x.
	ad4116.reset();

	// Configure channel 0 to use setup 0 and measure voltage between VIN0 and VIN1.
	ad4116.configure_channel(0, AD4116::Input::VIN0_VIN1, 0);

	// Configure setup 0 to measure bipolar voltage with external reference voltage,
	// enable input buffer, REF+ and REF- buffers.
	uint16_t setup_cfg = AD411x::Setup::BIPOLAR | AD411x::Setup::EXTERNAL_REF
	| AD411x::Setup::INPUT_BUFFERS | AD411x::Setup::REFBUF_P | AD411x::Setup::REFBUF_N;
	ad4116.configure_setup(0, setup_cfg);

	// Configure setup 0 to use default filtering (sinc5+sinc1 + 20 SPS 50/60Hz rejection)
	ad4116.write_filter_register(0, AD411x::Filter::DEFAULT_FILTER | config::ODR);

	// Configure ADC mode to continuous measurement.
	ad4116.write_adc_mode((uint16_t)AD411x::ADCMode::Mode::CONTINUOUS);

	// Use one pin for data ready interrupt.
	ad4116.enable_interrupt(config::DRDY_PIN);
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

	// Setup analog outputs as sample signals.
	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	digitalWrite(A0, LOW);  // keep A0 as "GND" always.

	// Configure AD4116.
	setup_ad4116();
	Serial.println("AD411x initialized.");


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


byte channel;
double volt;
double samples[3] = {0.0, 0.0, 0.0};

protocol_ad411x::datagram_t datagram;
uint32_t i = 0;

void loop() 
{
	// AD411x is set to continuous mode.
	// We do nothing until there is data ready to read.
	if (!ad4116.is_data_ready()) { return; }

	// Read the latest sample from the AD-converter.
	ad4116.read_volt(&channel, &volt);
	samples[channel] = volt;

	// Buffer uV values to send over network.
	datagram.samples[i] = (uint32_t)(volt * 10e6);
	i = (i + 1) % datagram.sample_count;
	if (i > 0) { return; }

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

	// Debug the time consumed.
	auto t1 = millis();
	Serial.println(t1 - t0);
	t0 = t1;
}