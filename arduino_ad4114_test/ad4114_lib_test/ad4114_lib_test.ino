/*
 * AD4114 Library Test - Using Adopticum_AD411x Library
 * STM32H753ZI
 * 
 * Hardware:
 * - PA5 = SCK (SPI1)
 * - PA6 = MISO/DOUT (SPI1)
 * - PA7 = MOSI/DIN (SPI1)
 * - CS tied to GND (3-wire mode)
 * 
 * Serial (USART1):
 * - PB14 = TX
 * - PB15 = RX
 * 
 * Clock: 25MHz HSE
 * 
 * Note: Since CS is tied to GND, we use a dummy CS pin (PA4)
 * and keep it LOW after initialization.
 */

#include <Adopticum_AD411x.h>

// Configuration
namespace config {
  const char *Program = "AD4114 Library Test";
  const char *Version = "2024-11-29";
  
  const bool SerialDebug = false;
  const long SerialSpeed = 115200;
  
  // Dummy CS pin - not actually connected, but library requires it
  // We'll keep it LOW to simulate 3-wire mode
  const int CS_PIN = PA4;
  
  // Output data rate
  const uint16_t ODR = (uint16_t)AD4116::OutputDataRate::SPS_50;
}

// UART pins (USART1: PB14=TX, PB15=RX)
HardwareSerial SerialDebug(PB15, PB14);  // RX, TX

void setup() {
  // LED for visual feedback
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Configure dummy CS pin as output and keep LOW
  pinMode(config::CS_PIN, OUTPUT);
  digitalWrite(config::CS_PIN, LOW);
  
  SerialDebug.begin(config::SerialSpeed);
  delay(1000);
  
  SerialDebug.println();
  SerialDebug.println("========================================");
  SerialDebug.println(config::Program);
  SerialDebug.println(config::Version);
  SerialDebug.println("========================================");
  SerialDebug.println("HSE: 25MHz");
  SerialDebug.println("SPI1: PA5(SCK) PA6(MISO) PA7(MOSI)");
  SerialDebug.println("UART: PB14(TX) PB15(RX)");
  SerialDebug.println("CS: Tied to GND (3-wire mode)");
  SerialDebug.println();
  
  // Setup communication with AD411x
  SerialDebug.println("Setting up SPI...");
  ad4116.setup(config::CS_PIN);
  
  // Keep CS LOW for 3-wire mode
  digitalWrite(config::CS_PIN, LOW);
  
  SerialDebug.println("Calling begin()...");
  if (!ad4116.begin()) {
    SerialDebug.println("ERROR: SPI begin failed!");
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  
  SerialDebug.println("Checking ID...");
  if (!ad4116.check_id()) {
    SerialDebug.println("ERROR: ID check failed!");
    SerialDebug.println("Reading registers anyway...");
  } else {
    SerialDebug.println("ID OK!");
  }
  
  SerialDebug.println("Resetting device...");
  ad4116.reset();
  
  // Keep CS LOW after reset
  digitalWrite(config::CS_PIN, LOW);
  
  SerialDebug.println("AD411x initialized.");
  SerialDebug.println();
  
  digitalWrite(LED_BUILTIN, LOW);  // LED off = success
}

void loop() {
  SerialDebug.println("--- Reading Registers ---");
  ad4116.read_many_things();
  
  // Toggle LED to show we're alive
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  
  delay(10000);
}

