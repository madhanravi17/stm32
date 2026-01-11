# PicoScope CAN Frame Decoding from STM32 FDCAN TX Pin

## Overview

This document explains how to interpret CAN signals when probing the STM32 FDCAN TX logic pin directly (without a CAN transceiver) using a PicoScope MSO (Mixed Signal Oscilloscope).

---

## Signal Layers in PicoScope

When capturing a CAN frame from the TX pin, PicoScope displays three layers:

### 1. Analog Voltage Trace (Blue)
- **Source:** Direct probe on FDCAN TX pin (e.g., PB6 for FDCAN2_TX)
- **Voltage Range:** 0V to 3.3V (STM32 logic levels)
- **Behavior:**
  - **3.3V (High):** TX pin is actively driving → **Dominant bit**
  - **0V (Low):** TX pin is released → **Recessive bit**

### 2. Digital Logic Trace (Purple)
- **Source:** PicoScope's internal ADC converts analog to digital
- **Threshold:** 1.65V (midpoint of 3.3V logic)
- **Conversion:**
  - **Voltage > 1.65V:** Digital "1"
  - **Voltage < 1.65V:** Digital "0"

### 3. Decoded CAN Frame (Bottom Bar)
- **Source:** PicoScope's serial decoder analyzes the digital trace
- **Output:** Frame fields (ID, DLC, Data, CRC, ACK, EOF)

---

## The Polarity Inversion Problem

### Standard CAN Bus Convention

In the CAN protocol specification (ISO 11898):

| CAN Logic | Bus State   | Description                          |
|-----------|-------------|--------------------------------------|
| **0**     | Dominant    | Active drive, wins arbitration       |
| **1**     | Recessive   | Passive/idle state                   |

### STM32 FDCAN TX Pin Behavior

The FDCAN TX pin from the STM32 uses **active-high** for dominant bits:

| TX Pin Voltage | TX Pin Logic | CAN Bus State | CAN Logic Bit |
|----------------|--------------|---------------|---------------|
| **3.3V (High)**| 1            | Dominant      | **0**         |
| **0V (Low)**   | 0            | Recessive     | **1**         |

This is **inverted** from what standard CAN decoders expect!

### Why This Happens

The STM32 FDCAN TX pin outputs a signal meant to drive a **CAN transceiver** (like MCP2551, SN65HVD230, TJA1050):

```
STM32 FDCAN TX Pin          CAN Transceiver          CAN Bus
      │                           │                     │
      │  TX HIGH (3.3V)  ───────► │  Drives CANH/CANL   │ Dominant (0)
      │  TX LOW  (0V)    ───────► │  Releases bus       │ Recessive (1)
      │                           │                     │
```

When probing TX directly (without transceiver), you see the raw logic before inversion.

---

## PicoScope "Low" Setting Explanation

### What the "Low" Option Does

In PicoScope's CAN decoder settings (next to baud rate), the "Low" option tells the decoder:

> "The **recessive/idle state** corresponds to a **LOW voltage level**"

### Internal Processing

```
Without "Low" setting (Default - High):
Physical:  ▔▔▔▁▁▔▔▁▁▁▔▔▔▔▁▁▔▔▔▔
           3.3V   0V
Decoded:   1  1  1  0  0  1  1  0  0  0  1  1  1  1  0  0  1  1  1  1
           ↑ WRONG - Treats high as recessive

With "Low" setting:
Physical:  ▔▔▔▁▁▔▔▁▁▁▔▔▔▔▁▁▔▔▔▔
           3.3V   0V
Decoded:   0  0  0  1  1  0  0  1  1  1  0  0  0  0  1  1  0  0  0  0
           ↑ CORRECT - Treats high as dominant
```

### Summary Table

| PicoScope Setting | HIGH Voltage Means | LOW Voltage Means | Use Case                    |
|-------------------|--------------------|--------------------|----------------------------|
| **High** (default)| Recessive (1)      | Dominant (0)       | Probing CAN_H or CAN_L     |
| **Low**           | Dominant (0)       | Recessive (1)      | Probing STM32 TX pin directly|

---

## Decoded Frame Verification

From your capture with:
- `txh.Identifier = 0x123`
- `uint8_t txd[2] = {0x1, 0x2}`

### Expected Decoded Fields

| Field         | Value      | Description                              |
|---------------|------------|------------------------------------------|
| **ID**        | 0x123      | 11-bit Standard Identifier               |
| **DLC**       | 2          | Data Length Code (2 bytes)               |
| **Data[0]**   | 0x01       | First data byte                          |
| **Data[1]**   | 0x02       | Second data byte                         |
| **CRC**       | 0x69FE     | 15-bit CRC (calculated by FDCAN)         |
| **ACK**       | (varies)   | ACK slot (self-acknowledged in loopback) |

### Your Capture Shows
- `ID - 123` ✓
- `2` (DLC) ✓
- `Data - 01` ✓
- `Data - 02` ✓
- `CRC - 69` and `CRC - FE` ✓
- Frame duration: `14 µs` (visible on right side)

---

## Bit Timing Verification

### Configured Parameters
```c
NominalPrescaler = 10
NominalTimeSeg1 = 12
NominalTimeSeg2 = 3
// Total TQ = 1 + 12 + 3 = 16
// Bit Time = 16 × (10/80MHz) = 2 µs
// Bit Rate = 500 kbit/s
```

### Expected Frame Duration

For a Standard CAN frame with 2 data bytes:
- Minimum bits (no stuffing): ~47 bits
- With bit stuffing: ~55-65 bits typical
- At 500 kbit/s (2 µs/bit): 94-130 µs

Your capture shows transitions consistent with 500 kbit/s timing.

---

## Troubleshooting

### If Decoding Fails

1. **Check baud rate:** Must match your `NominalPrescaler`, `TimeSeg1`, `TimeSeg2` settings
2. **Check polarity:** Use "Low" for direct TX pin probing
3. **Check threshold:** 1.65V is correct for 3.3V logic; adjust if using 5V logic
4. **Check sample rate:** PicoScope sample rate should be ≥10× baud rate (5 MS/s for 500 kbit/s)

### Common Issues

| Symptom                     | Likely Cause                        | Solution                     |
|-----------------------------|-------------------------------------|------------------------------|
| Garbage decoded data        | Wrong polarity setting              | Toggle High/Low setting      |
| No frames detected          | Wrong baud rate                     | Verify bit timing calculation|
| Intermittent decoding       | Threshold too close to noise        | Adjust threshold voltage     |
| Truncated frames            | Trigger position wrong              | Adjust pre-trigger capture   |

---

## References

1. **ISO 11898-1:** CAN protocol specification (dominant/recessive definitions)
2. **RM0399:** STM32H745/755 Reference Manual, FDCAN chapter
3. **PicoScope User Manual:** Serial decoding configuration

---

## Appendix: CAN Frame Structure

```
┌─────┬─────────────┬─────┬─────┬───────────────┬───────┬─────┬─────┬───────┐
│ SOF │  Identifier │ RTR │ IDE │      DLC      │ Data  │ CRC │ ACK │  EOF  │
│  1  │    11 bits  │  1  │  1  │    4 bits     │ 0-64B │ 15  │  2  │   7   │
└─────┴─────────────┴─────┴─────┴───────────────┴───────┴─────┴─────┴───────┘
       ◄────────────── Arbitration ──────────────►
```

- **SOF:** Start of Frame (always dominant)
- **Identifier:** Message priority/address
- **RTR:** Remote Transmission Request
- **IDE:** Identifier Extension (0 = standard 11-bit)
- **DLC:** Data Length Code
- **Data:** Payload bytes
- **CRC:** Cyclic Redundancy Check
- **ACK:** Acknowledgment slot
- **EOF:** End of Frame (7 recessive bits)
