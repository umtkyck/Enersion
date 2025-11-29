/*
Simple sketch to connect to Wifi from an Arduino Uno R4 Wifi
and stream some data over UDP.
*/

#include <Arduino.h>
#include <Wire.h>

// Either include standard Arduino WiFi headers...
#include <WiFi.h>
#include <WiFiUdp.h>
// ...or include WiFi headers recommended for Arduino UNO R4 WiFi.
//#include <WiFiS3.h>  // Applies to Arduino UNO R4 WiFi.

// Store secrets (like WiFi password) in a separate file.
#include "secrets.h"

namespace config {
	static const char* Program = "Hello WiFi on Uno R4";
	static const char* Version = "2023-12-05";

	// Only REQUIRE serial connection to computer in debugging scenario.
	static const bool SerialDebug = false;

	static const char* broadcast_address = "255.255.255.255";
	static const int dst_port = 50000;
	static const int src_port = 54321;
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


const char* encryption_type_to_string(int enc_type) {
  switch (enc_type) {
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_WPA:
      return "WPA";
    case ENC_TYPE_WPA2:
      return "WPA2";
    case ENC_TYPE_WPA3:
      return "WPA3";
    case ENC_TYPE_NONE:
      return "None";
    case ENC_TYPE_AUTO:
      return "Auto";
    case ENC_TYPE_UNKNOWN:
    default:
      return "Unknown";
  }
}


void wifi_init() {
  if (WiFi.status() == WL_NO_MODULE) {
    halt("Communication with WiFi module failed!");
  }

  auto fwv = WiFi.firmwareVersion();
  if (fwv != WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print("The WiFi firmware version is ");
    Serial.println(fwv);
    Serial.print("The latest available firmware version is ");
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
  }
}


bool wifi_connect() {
  WiFi.disconnect();
  Serial.print("Connecting to ");
  Serial.print(secrets::wifi_ssid);
  Serial.print("...");

  // Connect to WPA/WPA2 network:
  WiFi.begin(secrets::wifi_ssid, secrets::wifi_password);
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


// Show a lite of WiFi networks ordered by signal strength.
// Optionally limit to a maximum number of networks.
void wifi_show_networks(int max_count = 0) {
  // Scan WiFi networks and show a list of them.
  Serial.print("Scanning nearby WiFi networks... ");
  auto net_count = WiFi.scanNetworks();
  Serial.println("Done");

  if (net_count <= 0) {
    Serial.println("No WiFi networks found.");
    return;
  }

  if (max_count == 0 || net_count < max_count) {
    max_count = net_count;
  }

  for (int i = 0; i < max_count; ++i) {
    // Print SSID and RSSI for each network found
    Serial.print("Nr: ");
    Serial.print(i + 1);
    Serial.print(", Ch: ");
    Serial.print(WiFi.channel(i));
    Serial.print(", RSSID: ");
    Serial.print(WiFi.RSSI(i));
    Serial.print(" dBm");
    Serial.print(", SSID: ");
    Serial.print(WiFi.SSID(i));
    Serial.print(", Encryption: ");
    Serial.print(encryption_type_to_string(WiFi.encryptionType(i)));
    Serial.println();
  }
  if (max_count < net_count) {
    Serial.print("... ");
    Serial.print(net_count - max_count);
    Serial.println(" more networks not displayed.");
  }
}


void wifi_info_print(bool verbose = false) {
  char mac_addr_str[6 * 3];

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected");
    return;
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!verbose) return;

  // MAC address of our own network interface.
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  Serial.print("MAC address: ");
  Serial.println(mac_addr_str);

  // SSID of the network we are connected to.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // MAC address of the router we are connected to.
  byte bssid[6];
  WiFi.BSSID(bssid);
  sprintf(mac_addr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
          bssid[5], bssid[4], bssid[3], bssid[2], bssid[1], bssid[0]);
  Serial.print("Router's MAC address (BSSID): ");
  Serial.println(mac_addr_str);

  // Received signal strength.
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.println(rssi);

  // Encryption type used by the network we are connected to.
  Serial.print("Encryption Type:");
  Serial.println(encryption_type_to_string(WiFi.encryptionType()));
}


void setup() {
  // Generic initialisation.
  Serial.begin(115200);
  delay(10);
  if (config::SerialDebug) {
    while (!Serial) {}
  }
  Serial.println(config::Program);
  Serial.println(config::Version);
  pinMode(LED_BUILTIN, OUTPUT);

  // Parse broadcast address into an IPAddress.
  if (broadcast_ipaddr.fromString(config::broadcast_address)) {
    Serial.print("Broadcast to: ");
    Serial.println(broadcast_ipaddr);
  } else {
    halt("Failed to parse broadcast address.");
  }

  wifi_init();
  wifi_show_networks(5);
  if (wifi_connect()) {
    wifi_info_print(true);
  }

  // Initialize the WiFi UDP library and network settings. Starts WiFiUDP socket, listening at local port.
  udp.begin(config::src_port);
  t0 = millis();
  Serial.println("End of setup.");
}


int loop_counter = 0;
void loop() {
  // Limit the rest of the loop to only once per second.
  auto now = millis();
  if ((now - t0) < 5000) { return; }
  t0 = now;

  bool online = ((WiFi.status() == WL_CONNECTED) || wifi_connect());
  if (!online) {
    Serial.println("Not connected.");
    return;
  }

  // Prepare a message/datagram in a buffer.
  char msg[64];
  sprintf(msg, "Hello number %d from Arduino", ++loop_counter);
  // Print the message to serial console.
  Serial.print(msg);
  Serial.print(", chars: ");
  Serial.println(strlen(msg));

  // Broadcast the message over UDP.
  udp.beginPacket(broadcast_ipaddr, config::dst_port);
  udp.write((uint8_t*)&msg, strlen(msg));
  udp.endPacket();
}