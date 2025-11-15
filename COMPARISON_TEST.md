# Comparison Test

## Step 1: Confirm rs485_Test Works

```
STM32CubeIDE:
1. Open rs485_Test project
2. Build + Flash
3. Check COM7 → Should see "Debug UART initialized"
4. Check COM8 → Should see "HELLO WORLD" messages
```

## Step 2: Compare .ioc Files

Maybe UART configuration is different?

**rs485_Test/.ioc:**
- USART1: ??
- USART2: PD5/PD6, 115200

**SW_Controller_OUT/.ioc:**
- USART1: PB14/PB15, 115200 (Debug)
- USART2: PD5/PD6, 115200 (RS485)

Check if USART1 is properly configured in SW_Controller_OUT!

## Step 3: Simple Build Test

In SW_Controller_OUT, replace ENTIRE main() with:

```c
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  
  char msg[] = "TEST\r\n";
  
  while (1)
  {
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, 6, 100);
    HAL_Delay(500);
  }
}
```

If this works → Complex init breaks
If this doesn't work → USART1 config problem

## Critical Question

When you flash SW_Controller_OUT, does STM32CubeIDE show:
```
Download verified successfully  ← THIS LINE IS CRITICAL!
```

If it shows error or warning, that's the problem!

