/*
 * AD4114 Test - STM32H753ZI
 * 
 * Hardware:
 * - USART1: PB14(TX), PB15(RX) -> SerialUser
 * - SPI1: PA5(SCK), PA6(MISO), PA7(MOSI)
 * - CS: PA4 (Dummy - Physical CS tied to GND)
 * 
 * Configuration:
 * - Tools > U(S)ART: Enabled (generic 'Serial')
 * 
 * ID Values (from no-OS driver):
 * - AD4111/AD4112: 0x30D0
 * - AD4114/AD4115: 0x31D0
 * - AD4116:        0x34D0
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adopticum_AD411x.h>

HardwareSerial SerialUser(PB15, PB14);

#define DUMMY_CS_PIN PA4
#define MISO_PIN PA6

// Chip ID definitions (from Analog Devices no-OS driver)
#define AD717X_ID_REG_MASK      0xFFF0
#define AD411X_ID_REG_VALUE     0x30D0  // AD4111, AD4112
#define AD4114_5_ID_REG_VALUE   0x31D0  // AD4114, AD4115
#define AD4116_ID_REG_VALUE     0x34D0  // AD4116

// Reference Input for diagnostic (from no-OS: REFERENCE = 0x2B6)
#define INPUT_REFERENCE         0x2B6

uint16_t readChipID() {
  byte buf[2];
  ad4116.read_register(0x07, buf, 2);
  return ((uint16_t)buf[0] << 8) | buf[1];
}

const char* identifyChip(uint16_t id) {
  uint16_t masked = id & AD717X_ID_REG_MASK;
  if (masked == AD4114_5_ID_REG_VALUE) return "AD4114/AD4115";
  if (masked == AD411X_ID_REG_VALUE)   return "AD4111/AD4112";
  if (masked == AD4116_ID_REG_VALUE)   return "AD4116";
  if (id == 0xFFFF) return "ERROR: MISO stuck HIGH";
  if (id == 0x0000) return "ERROR: MISO stuck LOW";
  return "Unknown Device";
}

void setup() {
  SerialUser.begin(115200);
  delay(1000);
  
  SerialUser.println();
  SerialUser.println("=============================================================");
  SerialUser.println("              AD4114 DIAGNOSTIC TEST                         ");
  SerialUser.println("=============================================================");
  SerialUser.println("MCU       : STM32H753ZI @ 200MHz (HSE 25MHz)");
  SerialUser.println("SPI       : SPI1 @ 1MHz, Mode 3 (CPOL=1, CPHA=1)");
  SerialUser.println("Serial    : USART1 (PB14/PB15) @ 115200");
  SerialUser.println("-------------------------------------------------------------");
  
  // Initialize AD4116 library instance
  ad4116.setup(DUMMY_CS_PIN, &SPI, 1000000);

  SerialUser.print("[INIT] Library setup........... ");
  if (!ad4116.begin()) {
    SerialUser.println("FAILED");
  } else {
    SerialUser.println("OK");
  }

  // Read Chip ID
  SerialUser.print("[ID]   Reading Chip ID......... ");
  uint16_t chipID = readChipID();
  SerialUser.print("0x");
  if (chipID < 0x1000) SerialUser.print("0");
  SerialUser.print(chipID, HEX);
  SerialUser.print(" -> ");
  SerialUser.println(identifyChip(chipID));

  // Configure Setup 0: Bipolar, Internal 2.5V Ref, Buffers ON
  SerialUser.print("[CFG]  Setup 0 (Bipolar, 2.5V). ");
  uint16_t setup0 = ad4116.make_setup(true, true, true, true, true, false);
  ad4116.configure_setup(0, setup0);
  ad4116.set_v_ref(2.5);
  SerialUser.println("OK");

  // Configure Input Channels (CH0-CH9: VINx vs VINCOM)
  SerialUser.print("[CFG]  CH0-CH9 (VINx-VINCOM)... ");
  for(int i = 0; i < 10; i++) {
    // VINx vs VINCOM: AINPOS = i, AINNEG = 16 (VINCOM)
    // Format: (AINPOS << 5) | AINNEG
    uint16_t inputPair = (i << 5) | 16;
    ad4116.configure_channel(i, (AD4116::Input)inputPair, 0, true);
  }
  SerialUser.println("OK");

  // Disable CH10-CH14 (not used)
  for(int i = 10; i < 15; i++) {
    ad4116.disable_channel(i);
  }

  // Configure CH15 for Internal Reference Monitoring
  // Reference input: 0x2B6 (from no-OS driver)
  SerialUser.print("[CFG]  CH15 (VREF Monitor)..... ");
  ad4116.configure_channel(15, (AD4116::Input)INPUT_REFERENCE, 0, true);
  SerialUser.println("OK");

  SerialUser.println("-------------------------------------------------------------");
  SerialUser.println();
  SerialUser.println("Starting ADC Measurements...");
  SerialUser.println();
  SerialUser.println("CH   | RAW (HEX)  | RAW (DEC)  | VOLTAGE    | TYPE");
  SerialUser.println("-----|------------|------------|------------|----------------");
}

void loop() {
  // Wait for Data Ready (MISO goes LOW)
  uint32_t timeout = millis();
  while(digitalRead(MISO_PIN) == HIGH) {
    if(millis() - timeout > 500) {
      SerialUser.println("[TIMEOUT] No data ready - check ADC connection");
      delay(2000);
      return;
    }
  }
  
  // Read Data Register (0x04): 3 bytes Data + 1 byte Status
  byte buf[4];
  ad4116.read_register(0x04, buf, 4);
  
  // Parse Data
  uint32_t raw = ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
  byte status = buf[3];
  byte ch = status & 0x0F;
  
  // Check for errors in status
  bool adcError = (status & 0x40) != 0;  // Bit 6: ADC_ERR
  bool regError = (status & 0x20) != 0;  // Bit 5: REG_ERR
  
  // Calculate Voltage (Bipolar mode with 2.5V reference)
  // Formula: ((Raw / 2^23) - 1) * Vref
  // For Reference channel (CH15): Expected ~1.25V (REF/2)
  double voltage = ((double)raw / 8388608.0 - 1.0) * 2.5;
  
  // Print formatted output
  SerialUser.print("[");
  if(ch < 10) SerialUser.print("0");
  SerialUser.print(ch);
  SerialUser.print("] | 0x");
  
  // Hex with padding (6 digits for 24-bit)
  if(raw < 0x100000) SerialUser.print("0");
  if(raw < 0x10000) SerialUser.print("0");
  if(raw < 0x1000) SerialUser.print("0");
  if(raw < 0x100) SerialUser.print("0");
  if(raw < 0x10) SerialUser.print("0");
  SerialUser.print(raw, HEX);
  
  // Decimal (8 digits)
  SerialUser.print(" | ");
  char decStr[12];
  sprintf(decStr, "%8lu", raw);
  SerialUser.print(decStr);
  
  // Voltage (with sign)
  SerialUser.print(" | ");
  if(voltage >= 0) SerialUser.print(" ");
  SerialUser.print(voltage, 4);
  SerialUser.print("V | ");
  
  // Type description
  if(ch == 15) {
    SerialUser.print("VREF (~1.25V)");
  } else {
    SerialUser.print("Input VIN");
    SerialUser.print(ch);
  }
  
  // Error flags
  if(adcError) SerialUser.print(" [ADC_ERR]");
  if(regError) SerialUser.print(" [REG_ERR]");
  
  SerialUser.println();
  
  // After CH15, print separator and delay
  if(ch == 15) {
    SerialUser.println("-----|------------|------------|------------|----------------");
    delay(1000); // 1 second between scans
  }
}

// Override SystemClock_Config for HSE 25MHz
extern "C" void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) while(1);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV4;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) while(1);
}