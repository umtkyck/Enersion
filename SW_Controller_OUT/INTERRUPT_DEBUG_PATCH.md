# Interrupt Debug Patch

## Test: UART Interrupt Çalışıyor mu?

### Basit Test Ekleyin

`rs485_protocol.c` dosyasında `HAL_UART_RxCpltCallback` içine:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        DEBUG_INFO("RX: 0x%02X", rxBuffer[0]);  // ← BU SATIRI EKLEYİN
        RS485_ProcessReceivedByte(rxBuffer[0]);
        HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    }
}
```

Bu ekledikten sonra:
1. Build
2. Flash
3. COM8'e (Python veya GUI ile) bir şey gönderin
4. COM7'de (Debug console) "RX: 0xXX" mesajları görmeli

**Eğer görürseniz:** Interrupt çalışıyor, sorun packet parsing'de
**Eğer görmezseniz:** Interrupt çalışmıyor, NVIC sorunu var

## Alternative: LED Toggle Test

Veya daha basit, `HAL_UART_RxCpltCallback` içine:

```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        // Toggle any LED or pin to show interrupt fired
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // Örnek LED pin
        
        RS485_ProcessReceivedByte(rxBuffer[0]);
        HAL_UART_Receive_IT(&huart2, rxBuffer, 1);
    }
}
```

Her byte geldiğinde LED yanıp sönmeli.

