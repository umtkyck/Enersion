# ADC Configuration Required

## Current Status
✅ RS485 protocol is fully implemented and ready to test
⚠️ ADC hardware is NOT configured yet - using STUB implementation

## What Works Now
- RS485 communication (PING, GET_VERSION, GET_STATUS, HEARTBEAT)
- All analog read commands return **simulated test values**
- GUI can connect and display data
- Firmware compiles and runs

## Simulated Values
The firmware currently returns test values:
- **4-20mA channels:** ~12 mA (mid-range)
- **0-10V channels:** ~5 V (mid-range)
- **NTC channels:** ~25°C (room temperature)

## TODO: Configure Real ADC Hardware

### Steps to Enable ADC:

1. **Open `SW_Controller_ANA.ioc` in STM32CubeMX**

2. **Enable ADC1:**
   - Click on ADC1 in Peripherals tree
   - Set Mode: "IN0" through "IN15" (or as many channels as needed for first ADC)
   - Configure:
     - Resolution: 16-bit (for high precision)
     - Data Alignment: Right
     - Continuous Conversion: Enabled
     - DMA Continuous Requests: Enabled

3. **Enable ADC2 (if needed for more channels):**
   - Same configuration as ADC1
   - Use different GPIO pins

4. **Configure DMA:**
   - Enable DMA for ADC1
   - Mode: Circular
   - Data Width: Half Word (16-bit)

5. **GPIO Pin Mapping:**
   Based on schematic, map:
   - **26x 4-20mA inputs** → ADC channels (via current-sense resistors)
   - **6x 0-10V inputs** → ADC channels (via voltage dividers)
   - **4x NTC inputs** → ADC channels (with pull-up resistors)

6. **Generate Code:**
   - Click "Project → Generate Code"
   - This will create `hadc1` and related initialization code

7. **Uncomment ADC Code:**
   In `analog_input_handler.c`:
   ```c
   // Remove comment:
   extern ADC_HandleTypeDef hadc1;
   
   // Uncomment ADC calls:
   HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
   HAL_ADC_Start(&hadc1);
   HAL_ADC_PollForConversion(&hadc1, 10);
   // etc.
   ```

8. **Calibration:**
   - With known 4-20mA source, calibrate offset/gain
   - Use `AnalogInput_Calibrate420mA()` function
   - Same for voltage and NTC channels

## Hardware Considerations

### 4-20mA Input Conditioning:
- Precision resistor (250Ω typical) to convert to voltage
- OpAmp buffer (optional) for impedance matching
- ADC voltage range: 1V - 5V for 4-20mA

### 0-10V Input Conditioning:
- Voltage divider to scale to ADC range (0-3.3V)
- Example: 10V → 3.0V using 1kΩ/330Ω divider

### NTC Input:
- 10kΩ NTC with 10kΩ pull-up resistor
- Steinhart-Hart equation for temperature conversion

## Testing Without ADC

For now, you can:
1. ✅ Test RS485 communication
2. ✅ Verify GUI connects and displays data
3. ✅ Test all command handlers
4. ⚠️ See simulated values (not real measurements)

Once ADC is configured, the real analog values will be displayed!

---

**Note:** The ADC configuration requires hardware knowledge and access to the schematic. The current STUB mode allows software development and RS485 protocol testing without waiting for ADC setup.

