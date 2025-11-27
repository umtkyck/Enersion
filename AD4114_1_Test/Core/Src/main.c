/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : AD4114_0 Test - Using Analog Devices Official no-OS Driver
  ******************************************************************************
  * @attention
  *
  * AD4114_0 is connected to SPI1 (PA5=SCK, PA6=MISO, PA7=MOSI)
  * Debug output on USART1 (PB14=TX, PB15=RX) @ 115200 baud
  *
  * Hardware: AD4114BCPZ 24-bit Sigma-Delta ADC
  * - SPI Mode 3 (CPOL=1, CPHA=1)
  * - CS tied to GND (3-wire mode, directly connected to SPI)
  * - Internal 2.5V reference
  *
  * This version uses the official Analog Devices no-OS driver from:
  * https://github.com/analogdevicesinc/no-OS/tree/master/drivers/adc/ad717x
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ad717x.h"
#include "ad411x_regs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_REGS (sizeof(ad4111_regs) / sizeof(ad4111_regs[0]))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static char uart_buf[256];
static ad717x_dev *ad4114_dev = NULL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
static void Debug_Print(const char *fmt, ...);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief  Debug print to UART1
 */
static void Debug_Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(uart_buf, sizeof(uart_buf), fmt, args);
    va_end(args);
    
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)uart_buf, len, 100);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  
  /* Wait for AD4114 power-up */
  HAL_Delay(500);
  
  Debug_Print("\r\n");
  Debug_Print("============================================\r\n");
  Debug_Print("  AD4114_0 TEST - Analog Devices no-OS Driver\r\n");
  Debug_Print("  SPI1: PA5(SCK) PA6(MISO) PA7(MOSI)\r\n");
  Debug_Print("  CS: Tied to GND (3-wire mode)\r\n");
  Debug_Print("============================================\r\n\r\n");
  
  /* Configure AD717x initialization parameters */
  ad717x_init_param init_param = {0};
  
  /* SPI configuration - use our STM32 HAL wrapper */
  init_param.spi_init.hspi = &hspi1;
  init_param.spi_init.chip_select_port = 0;  /* Not used - CS tied to GND */
  init_param.spi_init.chip_select_pin = 0;
  
  /* Device configuration */
  init_param.active_device = ID_AD4114;
  init_param.regs = ad4111_regs;
  init_param.num_regs = NUM_REGS;
  init_param.ref_en = true;  /* Enable internal 2.5V reference */
  init_param.num_channels = 4;  /* We'll use 4 channels for testing */
  init_param.num_setups = 1;
  init_param.mode = SINGLE;  /* Single conversion mode */
  
  /* Setup 0 configuration - for 0-10V voltage measurement */
  init_param.setups[0].bi_unipolar = false;  /* Bipolar mode for voltage */
  init_param.setups[0].ref_source = INTERNAL_REF;  /* Internal 2.5V reference */
  init_param.setups[0].input_buff = true;  /* Enable input buffers */
  init_param.setups[0].ref_buff = false;  /* Disable ref buffers (internal ref) */
  
  /* Filter configuration - 50 SPS */
  init_param.filter_configuration[0].odr = sps_49;
  
  /* Channel 0: VIN0 (0-10V) - AIN1 vs VINCOM
   * From AD4114 datasheet Table 27:
   * VIN0_VINCOM = 0x10 = AIN0 vs VINCOM
   */
  init_param.chan_map[0].channel_enable = false;  /* Disabled initially */
  init_param.chan_map[0].setup_sel = 0;
  init_param.chan_map[0].analog_inputs.analog_input_pairs = VIN0_VINCOM;
  
  /* Channel 1: VIN1 (0-10V) - AIN1 vs VINCOM */
  init_param.chan_map[1].channel_enable = false;
  init_param.chan_map[1].setup_sel = 0;
  init_param.chan_map[1].analog_inputs.analog_input_pairs = VIN1_VINCOM;
  
  /* Channel 2: VIN2 (0-10V) - AIN2 vs VINCOM */
  init_param.chan_map[2].channel_enable = false;
  init_param.chan_map[2].setup_sel = 0;
  init_param.chan_map[2].analog_inputs.analog_input_pairs = VIN2_VINCOM;
  
  /* Channel 3: VIN3 (0-10V) - AIN3 vs VINCOM */
  init_param.chan_map[3].channel_enable = false;
  init_param.chan_map[3].setup_sel = 0;
  init_param.chan_map[3].analog_inputs.analog_input_pairs = VIN3_VINCOM;
  
  /* First, do a manual reset and ID read to verify SPI communication */
  Debug_Print("[AD4114] Step 1: Manual SPI Reset (64 clocks with DIN=1)...\r\n");
  {
      uint8_t reset_buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      
      /* Send 64 clocks with DIN HIGH to reset */
      HAL_SPI_Transmit(&hspi1, reset_buf, 8, 100);
      HAL_Delay(1);  /* Wait 500us+ for LDO */
      HAL_SPI_Transmit(&hspi1, reset_buf, 8, 100);
      HAL_Delay(10);
      
      Debug_Print("[AD4114] Reset complete, waiting for stabilization...\r\n");
      HAL_Delay(100);
  }
  
  Debug_Print("\r\n[AD4114] Step 2: Manual ID Register Read (0x07)...\r\n");
  {
      /* Try multiple times with different approaches */
      for (int attempt = 1; attempt <= 3; attempt++) {
          Debug_Print("\r\n  Attempt %d:\r\n", attempt);
          
          /* Send command byte first, then read */
          uint8_t cmd = 0x47;  /* Read ID register */
          uint8_t rx_cmd = 0;
          uint8_t data[2] = {0x00, 0x00};
          uint8_t rx_data[2] = {0};
          
          /* Method 1: Separate transmit and receive */
          HAL_SPI_TransmitReceive(&hspi1, &cmd, &rx_cmd, 1, 100);
          HAL_Delay(1);  /* Small delay between command and data */
          HAL_SPI_TransmitReceive(&hspi1, data, rx_data, 2, 100);
          
          Debug_Print("    Method 1 (separate): CMD_RX=%02X, DATA_RX=%02X %02X\r\n", 
                     rx_cmd, rx_data[0], rx_data[1]);
          
          uint16_t id1 = ((uint16_t)rx_data[0] << 8) | rx_data[1];
          Debug_Print("    Parsed ID: 0x%04X\r\n", id1);
          
          HAL_Delay(10);
          
          /* Method 2: All in one transaction */
          uint8_t tx_buf[3] = {0x47, 0x00, 0x00};
          uint8_t rx_buf[3] = {0};
          HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 3, 100);
          
          Debug_Print("    Method 2 (combined): RX=%02X %02X %02X\r\n", 
                     rx_buf[0], rx_buf[1], rx_buf[2]);
          
          uint16_t id2 = ((uint16_t)rx_buf[1] << 8) | rx_buf[2];
          Debug_Print("    Parsed ID: 0x%04X\r\n", id2);
          
          if ((id1 & 0xFFF0) == 0x30D0 || (id2 & 0xFFF0) == 0x30D0) {
              Debug_Print("  -> VALID AD4114 ID detected!\r\n");
              break;
          } else if (id2 == 0xFFFF) {
              Debug_Print("  -> MISO stuck HIGH\r\n");
          } else if (id2 == 0x0000) {
              Debug_Print("  -> MISO stuck LOW\r\n");
          }
          
          /* Extra reset between attempts */
          uint8_t reset_buf[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
          HAL_SPI_Transmit(&hspi1, reset_buf, 8, 100);
          HAL_Delay(100);
      }
      
      Debug_Print("\r\n  Hardware Check Required:\r\n");
      Debug_Print("  - Is CS (Pin 17) tied to GND?\r\n");
      Debug_Print("  - Is AVDD = 5V, IOVDD = 3.3V?\r\n");
      Debug_Print("  - Are SPI wires correct? (SCK=PA5, MISO=PA6, MOSI=PA7)\r\n");
  }
  
  Debug_Print("\r\n[AD4114] Step 3: Initializing with no-OS driver...\r\n");
  
  int32_t ret = AD717X_Init(&ad4114_dev, init_param);
  
  if (ret < 0) {
      Debug_Print("[AD4114] ERROR: Init failed with code %ld\r\n", (long)ret);
      Debug_Print("[AD4114] Possible causes:\r\n");
      Debug_Print("  - SPI Mode incorrect (need Mode 3: CPOL=1, CPHA=1)\r\n");
      Debug_Print("  - CS not tied to GND\r\n");
      Debug_Print("  - Wrong SPI pins (need PA5=SCK, PA6=MISO, PA7=MOSI)\r\n");
      Debug_Print("  - AD4114 not powered\r\n");
  } else {
      Debug_Print("[AD4114] Init SUCCESS!\r\n");
      
      /* Read and display ID */
      ad717x_st_reg *id_reg = AD717X_GetReg(ad4114_dev, AD717X_ID_REG);
      if (id_reg) {
          Debug_Print("[AD4114] Chip ID: 0x%04lX\r\n", (unsigned long)id_reg->value);
      }
      
      /* Read and display ADCMODE register */
      AD717X_ReadRegister(ad4114_dev, AD717X_ADCMODE_REG);
      ad717x_st_reg *adcmode_reg = AD717X_GetReg(ad4114_dev, AD717X_ADCMODE_REG);
      if (adcmode_reg) {
          Debug_Print("[AD4114] ADCMODE: 0x%04lX (REF_EN=%d)\r\n", 
                     (unsigned long)adcmode_reg->value,
                     (adcmode_reg->value & 0x8000) ? 1 : 0);
      }
      
      /* Read and display SETUPCON0 register */
      AD717X_ReadRegister(ad4114_dev, AD717X_SETUPCON0_REG);
      ad717x_st_reg *setup_reg = AD717X_GetReg(ad4114_dev, AD717X_SETUPCON0_REG);
      if (setup_reg) {
          Debug_Print("[AD4114] SETUPCON0: 0x%04lX (BI_UNI=%d, INBUF=%d, REF=%d)\r\n", 
                     (unsigned long)setup_reg->value,
                     (setup_reg->value & 0x1000) ? 1 : 0,
                     (setup_reg->value >> 8) & 0x3,
                     (setup_reg->value >> 4) & 0x3);
      }
  }
  
  Debug_Print("\r\n");
  
  uint32_t scan_count = 0;
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    Debug_Print("\r\n========== SCAN #%lu ==========\r\n", (unsigned long)++scan_count);
    Debug_Print("CH  |    RAW ADC   |  Voltage  | Notes\r\n");
    Debug_Print("----|--------------|-----------|------------------\r\n");
    
    if (ad4114_dev != NULL) {
        /* Read channels 0-3 using the official driver */
        for (uint8_t ch = 0; ch < 4; ch++) {
            int32_t adc_raw = 0;
            
            /* Use the official single_read function */
            ret = ad717x_single_read(ad4114_dev, ch, &adc_raw);
            
            const char* label = "";
            const char* expected = "";
            if (ch == 0) { label = "VIN0"; expected = "POT ~0V"; }
            else if (ch == 1) { label = "VIN1"; expected = "POT ~0V"; }
            else if (ch == 2) { label = "VIN2"; expected = "float"; }
            else if (ch == 3) { label = "VIN3"; expected = "float"; }
            
            if (ret == 0) {
                /* BIPOLAR voltage formula:
                 * AD4114 Bipolar Output Code:
                 * - Code 0x000000 (0) = -VREF = -2.5V
                 * - Code 0x800000 (8388608) = 0V (mid-scale)
                 * - Code 0xFFFFFF (16777215) = +VREF - 1LSB ≈ +2.5V
                 * 
                 * With 4:1 voltage divider for 0-10V:
                 *   Actual_mV = ((Code - 8388608) * 10000) / 8388608
                 */
                int32_t centered = adc_raw - 8388608;
                int64_t temp = (int64_t)centered * 10000LL;
                int32_t voltage_mv = (int32_t)(temp / 8388608LL);
                
                char sign = (voltage_mv >= 0) ? '+' : '-';
                int32_t abs_mv = (voltage_mv >= 0) ? voltage_mv : -voltage_mv;
                int32_t volts = abs_mv / 1000;
                int32_t millis = abs_mv % 1000;
                
                Debug_Print("CH%02d| RAW: %8ld | %c%2ld.%03ld V | %s (%s)\r\n", 
                           ch, (long)adc_raw, 
                           sign, (long)volts, (long)millis,
                           label, expected);
            } else {
                Debug_Print("CH%02d| ERROR: %ld   |   ----    | %s (%s)\r\n", 
                           ch, (long)ret, label, expected);
            }
            
            HAL_GPIO_TogglePin(LED_RUN_GPIO_Port, LED_RUN_Pin);
        }
    } else {
        Debug_Print("AD4114 not initialized - skipping channel reads\r\n");
        
        /* Manual SPI test */
        Debug_Print("\r\nManual SPI test:\r\n");
        uint8_t tx_buf[4] = {0x47, 0xFF, 0xFF, 0xFF};
        uint8_t rx_buf[4] = {0};
        HAL_SPI_TransmitReceive(&hspi1, tx_buf, rx_buf, 4, 100);
        Debug_Print("  RX: %02X %02X %02X %02X\r\n", rx_buf[0], rx_buf[1], rx_buf[2], rx_buf[3]);
    }
    
    Debug_Print("==================================\r\n");
    Debug_Print("Next scan in 5 seconds...\r\n");
    
    /* Wait 5 seconds before next scan */
    HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV4;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_RUN_Pin|LED_ERR_Pin|RS485_ANA_COM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_RUN_Pin LED_ERR_Pin RS485_ANA_COM_Pin */
  GPIO_InitStruct.Pin = LED_RUN_Pin|LED_ERR_Pin|RS485_ANA_COM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
