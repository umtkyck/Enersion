# RS485 Direction Control Check

## Hardware Info
- **MCU:** STM32 (Controller OUT)
- **PD4:** COMOUT (RS485 DE/RE control)
- **PD5:** UART TX
- **PD6:** UART RX

## RS485 Transceiver Expected Connections
```
STM32                RS485 Transceiver
PD5 (TX)    ------>  DI (Driver Input)
PD6 (RX)    <------  RO (Receiver Output)
PD4 (DE)    ------>  DE (Driver Enable)
              |---->  /RE (Receiver Enable - inverted)
```

## Firmware Checklist

### 1. GPIO Configuration
```c
// PD4 as output (push-pull)
GPIO_InitStruct.Pin = GPIO_PIN_4;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

// Default to receive mode
HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
```

### 2. UART Configuration
```c
// USART2 or USART3 depending on your setup
huart.Instance = USART2;  // Check which UART uses PD5/PD6
huart.Init.BaudRate = 115200;
huart.Init.WordLength = UART_WORDLENGTH_8B;
huart.Init.StopBits = UART_STOPBITS_1;
huart.Init.Parity = UART_PARITY_NONE;
huart.Init.Mode = UART_MODE_TX_RX;
```

### 3. RS485 Transmit Function
```c
void RS485_Transmit(uint8_t *data, uint16_t len)
{
    // Enable transmitter
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    
    // Small delay for transceiver switching
    HAL_Delay(1);  // 1ms
    
    // Transmit data
    HAL_UART_Transmit(&huart, data, len, 100);
    
    // Wait for transmission to complete
    while(__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);
    
    // Small delay before disabling
    HAL_Delay(1);  // 1ms
    
    // Switch back to receive mode
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
}
```

### 4. RS485 Receive Function
```c
void RS485_Receive_Init(void)
{
    // Ensure we're in receive mode
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    
    // Start UART receive interrupt
    HAL_UART_Receive_IT(&huart, rx_buffer, 1);
}
```

## Common Issues

### Issue 1: Always in TX mode
**Symptom:** PC sends data but MCU never responds
**Cause:** PD4 stuck HIGH
**Fix:** Check GPIO init, ensure PD4 goes LOW after TX

### Issue 2: Always in RX mode  
**Symptom:** MCU receives but cannot transmit to PC
**Cause:** PD4 stuck LOW or not controlled
**Fix:** Verify PD4 goes HIGH before transmission

### Issue 3: Timing Issues
**Symptom:** First few bytes lost
**Cause:** Transceiver switching delay not accounted for
**Fix:** Add 1-2ms delays before/after transmission

### Issue 4: Echo on Bus
**Symptom:** MCU receives its own transmitted data
**Cause:** DE/RE not properly controlled
**Fix:** Ensure RX is disabled during TX (PD4 HIGH should disable RX)

## Debug Steps

### Step 1: Check PD4 with Oscilloscope/Logic Analyzer
- Monitor PD4 pin during operation
- Should be LOW most of the time (RX mode)
- Should pulse HIGH only during transmission

### Step 2: Check UART Signals
- PD5 (TX) should show data when PD4 is HIGH
- PD6 (RX) should show data when PD4 is LOW

### Step 3: Software Debug
```c
// Add debug prints
void RS485_Transmit_Debug(uint8_t *data, uint16_t len)
{
    printf("TX: Setting PD4 HIGH\n");
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    
    HAL_Delay(1);
    
    printf("TX: Sending %d bytes\n", len);
    HAL_UART_Transmit(&huart, data, len, 100);
    
    printf("TX: Waiting for TC flag\n");
    while(__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);
    
    HAL_Delay(1);
    
    printf("TX: Setting PD4 LOW\n");
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
}
```

## Quick Test Code

### Minimal TX Test
```c
void RS485_Test_Transmit(void)
{
    uint8_t test_msg[] = "HELLO\r\n";
    
    while(1)
    {
        // Set TX mode
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
        HAL_Delay(1);
        
        // Send
        HAL_UART_Transmit(&huart, test_msg, 7, 100);
        
        // Wait
        while(__HAL_UART_GET_FLAG(&huart, UART_FLAG_TC) == RESET);
        HAL_Delay(1);
        
        // Set RX mode
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
        
        // Wait 1 second
        HAL_Delay(1000);
    }
}
```

### Minimal RX Test
```c
void RS485_Test_Receive(void)
{
    uint8_t rx_byte;
    
    // Stay in RX mode
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    
    while(1)
    {
        if(HAL_UART_Receive(&huart, &rx_byte, 1, 100) == HAL_OK)
        {
            printf("RX: 0x%02X (%c)\n", rx_byte, rx_byte);
        }
    }
}
```

## Verification Checklist
- [ ] PD4 configured as GPIO output
- [ ] PD4 defaults to LOW (RX mode)
- [ ] PD4 goes HIGH before UART TX
- [ ] PD4 goes LOW after UART TX complete
- [ ] Delays added for transceiver switching (1-2ms)
- [ ] UART configured correctly (115200, 8N1)
- [ ] RS485 A+/B- wired correctly
- [ ] RS485 termination resistor present (120Ω at ends)

## Expected Behavior
1. **Idle:** PD4=LOW, MCU ready to receive
2. **PC sends data:** MCU receives on PD6
3. **MCU responds:** PD4=HIGH, sends on PD5, then PD4=LOW
4. **Back to idle:** PD4=LOW, ready for next message


