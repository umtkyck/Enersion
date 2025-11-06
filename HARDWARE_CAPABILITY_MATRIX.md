# Hardware Capability Matrix - Cross-Check

## Hardware Specification (from HW_ENERSION_CONTROLLER_R1M1.pdf)

| Interface Type | Quantity | Controller | Address | Description |
|----------------|----------|------------|---------|-------------|
| 4-20mA Inputs  | 26       | Controller_420 | 0x01 | Analog current loop inputs |
| 0-10V Inputs   | 6        | Controller_420 | 0x01 | Analog voltage inputs |
| NTC Sensors    | 4        | Controller_420 | 0x01 | Temperature sensors (NTC) |
| Digital Inputs | 56       | Controller_DIO | 0x02 | Digital input channels |
| Digital Outputs| 56       | Controller_OUT | 0x03 | Digital output channels |

**Total I/O Count: 148 channels**

## Controller Assignment

### Controller_420 (Address 0x01) - Analog Interface Controller
- **26x 4-20mA Inputs**
  - Range: 4-20mA (0-100% scale)
  - Resolution: 16-bit ADC (65536 steps)
  - Conversion: Current → Engineering units
  
- **6x 0-10V Inputs**
  - Range: 0-10V (0-100% scale)
  - Resolution: 16-bit ADC (65536 steps)
  - Conversion: Voltage → Engineering units

- **4x NTC Temperature Sensors**
  - Type: NTC thermistor
  - Range: Typical -40°C to +125°C
  - Resolution: 16-bit ADC
  - Conversion: Resistance → Temperature (°C)

**Total Analog Channels: 36**

### Controller_DIO (Address 0x02) - Digital Input Controller
- **56x Digital Inputs**
  - Logic levels: 0V (LOW), 24V (HIGH) typical
  - Debouncing: 20ms
  - Update rate: 10ms
  - Data format: 7 bytes (56 bits)

### Controller_OUT (Address 0x03) - Digital Output Controller
- **56x Digital Outputs**
  - Logic levels: 0V (LOW), 24V (HIGH) typical
  - Drive capability: Per hardware spec
  - Update: Immediate on command
  - Data format: 7 bytes (56 bits)

## Firmware-Hardware Mapping

### Controller_420 Analog Channel Mapping

#### 4-20mA Inputs (26 channels)
| Channel | ADC | GPIO/Pin | Range | Scale Factor |
|---------|-----|----------|-------|--------------|
| AI_420_0 - AI_420_25 | ADC1 | Various | 4-20mA | 0-100% |

**ADC Configuration:**
- Resolution: 16-bit
- Reference: Internal VREF (typically 2.5V or 3.3V)
- Sampling time: Optimized for accuracy
- Conversion formula: `Current(mA) = ((ADC_Value / 65535) * V_ref / R_sense)`
- Engineering units: `Value(%) = ((Current - 4) / 16) * 100`

#### 0-10V Inputs (6 channels)
| Channel | ADC | GPIO/Pin | Range | Scale Factor |
|---------|-----|----------|-------|--------------|
| AI_V_0 - AI_V_5 | ADC1 | Various | 0-10V | 0-100% |

**ADC Configuration:**
- Resolution: 16-bit
- Reference: Internal VREF
- Voltage divider: If needed for ADC protection
- Conversion formula: `Voltage(V) = (ADC_Value / 65535) * V_ref * Divider_Ratio`
- Engineering units: `Value(%) = (Voltage / 10) * 100`

#### NTC Temperature Sensors (4 channels)
| Channel | ADC | GPIO/Pin | Range | Type |
|---------|-----|----------|-------|------|
| NTC_0 - NTC_3 | ADC1 | Various | -40 to 125°C | 10k NTC typical |

**ADC Configuration:**
- Resolution: 16-bit
- Reference: Internal VREF
- Pull-up resistor: 10kΩ typical
- Conversion: Steinhart-Hart equation or lookup table
- Formula: `T(°C) = (1 / (A + B*ln(R) + C*ln(R)^3)) - 273.15`

### Controller_DIO Digital Input Mapping

#### Digital Inputs (56 channels = 7 bytes)
| Byte | Bits | Channels | GPIO Ports |
|------|------|----------|------------|
| 0    | 0-7  | DI_0 - DI_7   | GPIOF, GPIOG |
| 1    | 0-7  | DI_8 - DI_15  | GPIOG |
| 2    | 0-7  | DI_16 - DI_23 | GPIOG |
| 3    | 0-7  | DI_24 - DI_31 | GPIOE |
| 4    | 0-7  | DI_32 - DI_39 | GPIOE, GPIOB |
| 5    | 0-7  | DI_40 - DI_47 | GPIOB, GPIOD |
| 6    | 0-7  | DI_48 - DI_55 | GPIOD, GPIOC |

### Controller_OUT Digital Output Mapping

#### Digital Outputs (56 channels = 7 bytes)
| Byte | Bits | Channels | GPIO Ports |
|------|------|----------|------------|
| 0    | 0-7  | DO_0 - DO_7   | GPIOF |
| 1    | 0-7  | DO_8 - DO_15  | GPIOF, GPIOC |
| 2    | 0-7  | DO_16 - DO_23 | GPIOC |
| 3    | 0-7  | DO_24 - DO_31 | GPIOA |
| 4    | 0-7  | DO_32 - DO_39 | GPIOA, GPIOB |
| 5    | 0-7  | DO_40 - DO_47 | GPIOB, GPIOE, GPIOD |
| 6    | 0-7  | DO_48 - DO_55 | GPIOD, GPIOG |

## Communication Protocol Updates

### New Command Codes for Analog Inputs

| Code | Name | Description | Data Format |
|------|------|-------------|-------------|
| 0x40 | READ_ANALOG_420 | Read all 26x 4-20mA inputs | 52 bytes (26 x uint16) |
| 0x41 | ANALOG_420_RESPONSE | 4-20mA data response | 52 bytes |
| 0x42 | READ_ANALOG_VOLTAGE | Read all 6x 0-10V inputs | 12 bytes (6 x uint16) |
| 0x43 | ANALOG_VOLTAGE_RESPONSE | Voltage data response | 12 bytes |
| 0x44 | READ_NTC | Read all 4x NTC sensors | 16 bytes (4 x float32) |
| 0x45 | NTC_RESPONSE | Temperature data response | 16 bytes |
| 0x46 | READ_ALL_ANALOG | Read all analog channels | 80 bytes total |
| 0x47 | ALL_ANALOG_RESPONSE | All analog data | 80 bytes |

### Analog Data Structures

#### 4-20mA Data Format
```c
typedef struct {
    uint16_t raw_adc[26];           // Raw ADC values
    float current_mA[26];           // Converted to mA
    float scaled_percent[26];       // Scaled to 0-100%
    uint8_t status[26];             // Channel status (0=OK, 1=Underrange, 2=Overrange)
} Analog420_Data_t;
```

#### 0-10V Data Format
```c
typedef struct {
    uint16_t raw_adc[6];            // Raw ADC values
    float voltage_V[6];             // Converted to Volts
    float scaled_percent[6];        // Scaled to 0-100%
    uint8_t status[6];              // Channel status
} AnalogVoltage_Data_t;
```

#### NTC Data Format
```c
typedef struct {
    uint16_t raw_adc[4];            // Raw ADC values
    float resistance_ohm[4];        // Calculated resistance
    float temperature_C[4];         // Temperature in Celsius
    uint8_t status[4];              // Sensor status (0=OK, 1=Open, 2=Short)
} NTC_Data_t;
```

## GUI Interface Requirements

### Main Window Layout
```
┌─────────────────────────────────────────────────────────────────┐
│ Connection Panel                                                 │
├─────────────────────────────────────────────────────────────────┤
│ Controller Status (3 MCUs)                                       │
├─────────────────────────────────────────────────────────────────┤
│ ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│ │ Analog Inputs   │ │ Digital Inputs  │ │ Digital Outputs │   │
│ │ (Controller_420)│ │ (Controller_DIO)│ │ (Controller_OUT)│   │
│ │                 │ │                 │ │                 │   │
│ │ 26x 4-20mA     │ │ 56x DI         │ │ 56x DO         │   │
│ │ 6x  0-10V      │ │                 │ │                 │   │
│ │ 4x  NTC        │ │                 │ │                 │   │
│ └─────────────────┘ └─────────────────┘ └─────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### Analog Input Display Requirements
- **4-20mA Channels**: Show current (mA) and scaled value (%)
- **0-10V Channels**: Show voltage (V) and scaled value (%)
- **NTC Channels**: Show temperature (°C) with alarm indicators
- **Update Rate**: Configurable (default 1 second)
- **Trend Graphs**: Optional real-time plotting
- **Alarm Limits**: Configurable high/low alarms

### Digital I/O Display Requirements
- **Digital Inputs**: 56 indicators (green=HIGH, gray=LOW)
- **Digital Outputs**: 56 checkboxes with visual feedback
- **Update Rate**: Real-time on change

## Cross-Check Verification

### ✅ Hardware Specification Verified
- [x] 26x 4-20mA analog inputs
- [x] 6x 0-10V analog inputs
- [x] 4x NTC temperature sensors
- [x] 56x Digital inputs
- [x] 56x Digital outputs

### 🔄 Firmware Updates Required
- [ ] Controller_420: Add analog input handler
- [ ] Controller_420: Add 4-20mA reading (26 channels)
- [ ] Controller_420: Add 0-10V reading (6 channels)
- [ ] Controller_420: Add NTC temperature reading (4 channels)
- [ ] Controller_DIO: Update for 56 inputs (was 64)
- [ ] Controller_OUT: Update for 56 outputs (was 64)

### 🔄 GUI Updates Required
- [ ] Add analog input display panel
- [ ] Add 4-20mA input visualization (26 channels)
- [ ] Add 0-10V input visualization (6 channels)
- [ ] Add NTC temperature display (4 channels)
- [ ] Update digital input display (56 channels)
- [ ] Update digital output control (56 channels)
- [ ] Add trend graphs (optional)
- [ ] Add alarm management

### 🔄 Protocol Updates Required
- [ ] Add READ_ANALOG_420 command (0x40)
- [ ] Add READ_ANALOG_VOLTAGE command (0x42)
- [ ] Add READ_NTC command (0x44)
- [ ] Add READ_ALL_ANALOG command (0x46)
- [ ] Update packet size limits if needed

## Performance Specifications

### Analog Input Performance
- **ADC Resolution**: 16-bit (0.0015% per step)
- **Sampling Rate**: Up to 1kSPS per channel
- **Accuracy**: ±0.1% typical (hardware dependent)
- **Update Rate to GUI**: 1 Hz default, configurable to 10 Hz
- **Filtering**: Moving average (configurable window)

### Digital I/O Performance
- **Input Debounce**: 20ms
- **Input Scan Rate**: 10ms (100 Hz)
- **Output Update**: Immediate on command
- **Response Time**: < 50ms typical

### Communication Performance
- **Baud Rate**: 115200 bps
- **Max Analog Packet**: 80 bytes
- **Max Digital Packet**: 7 bytes
- **Round-trip Time**: < 100ms typical
- **Throughput**: ~1000 readings/second system-wide

## Safety and Error Handling

### Analog Input Errors
- **Underrange**: < 3.8mA for 4-20mA (wire break detection)
- **Overrange**: > 21mA for 4-20mA
- **Out of Range**: < 0V or > 11V for 0-10V inputs
- **NTC Open Circuit**: Very high resistance
- **NTC Short Circuit**: Very low resistance

### Digital I/O Errors
- **Stuck Input**: Input doesn't change for extended period
- **Output Fault**: Verification read doesn't match write
- **Communication Loss**: Timeout on response

### Error Reporting
- Status bits in each data packet
- Error counters in status response
- Visual indicators in GUI
- Debug log messages

## Update Priority

1. ✅ **HIGH**: Update digital I/O counts (56 instead of 64)
2. ✅ **HIGH**: Add analog input handlers to Controller_420
3. ✅ **HIGH**: Update GUI to show all interfaces
4. ⚠️ **MEDIUM**: Add NTC temperature reading
5. ⚠️ **MEDIUM**: Add trend graphs
6. ⚠️ **LOW**: Add alarm management
7. ⚠️ **LOW**: Add data logging

---
**Document Version**: 2.0  
**Hardware Revision**: R1M1  
**Last Updated**: October 27, 2025


