# I2C Master-Slave Loopback Communication Workflow

## STM32H745ZI - CM7 Core

This document describes the I2C master-slave loopback communication implementation using two I2C peripherals on the same MCU.
master transmits via interrupt since same core , halt CPU through interrupt write / read operation, and slave receives and transmits via interrupt. Because both master and slave are on the same MCU core, the communication is looped back internally, blocking mode is not used to avoid halting the CPU, non blocking interrupt mode is used instead.
---

## Hardware Configuration

### Pin Assignments

| Peripheral | Role   | SCL Pin | SDA Pin | Address |
|------------|--------|---------|---------|---------|
| I2C1       | Master | PB8     | PB9     | N/A     |
| I2C4       | Slave  | PF14    | PF15    | 0x02    |

### Physical Connections Required

```
I2C1 (Master)          I2C4 (Slave)
    PB8 (SCL) ────────── PF14 (SCL)
    PB9 (SDA) ────────── PF15 (SDA)
                │
            4.7kΩ Pull-ups to VDD
```

### I2C Parameters
- **Speed**: 100 kHz (Standard Mode)
- **Addressing**: 7-bit mode
- **Slave Address**: `0x01 << 1 = 0x02` (shifted for HAL API)

---

## Communication Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        INITIALIZATION                                    │
├─────────────────────────────────────────────────────────────────────────┤
│  1. HAL_I2C_Slave_Receive_IT(&hi2c4, ...)  → Arm slave receiver         │
│  2. HAL_I2C_Master_Transmit_IT(&hi2c1, ...) → Start first TX            │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     CONTINUOUS COMMUNICATION LOOP                        │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────┐         I2C Bus          ┌──────────────┐             │
│  │  I2C1 (M)    │ ═══════════════════════> │  I2C4 (S)    │             │
│  │  Master TX   │  "Hello from CM7 Master!"│  Slave RX    │             │
│  └──────────────┘                          └──────────────┘             │
│         │                                         │                      │
│         │ MasterTxCpltCallback                    │ SlaveRxCpltCallback  │
│         │ sets masterRxReady = 1                  │ arms Slave TX        │
│         ▼                                         ▼                      │
│  ┌──────────────┐                          ┌──────────────┐             │
│  │  Main Loop   │                          │  I2C4 (S)    │             │
│  │  1ms delay   │                          │  TX Armed    │             │
│  └──────────────┘                          └──────────────┘             │
│         │                                         │                      │
│         ▼                                         │                      │
│  ┌──────────────┐         I2C Bus          ┌──────────────┐             │
│  │  I2C1 (M)    │ <═══════════════════════ │  I2C4 (S)    │             │
│  │  Master RX   │  "...Positive response.."│  Slave TX    │             │
│  └──────────────┘                          └──────────────┘             │
│         │                                         │                      │
│         │ MasterRxCpltCallback                    │ SlaveTxCpltCallback  │
│         │ sets masterTxReady = 1                  │ re-arms Slave RX     │
│         ▼                                         ▼                      │
│  ┌──────────────┐                          ┌──────────────┐             │
│  │  Main Loop   │                          │  I2C4 (S)    │             │
│  │  100ms delay │                          │  RX Armed    │             │
│  └──────────────┘                          └──────────────┘             │
│         │                                         │                      │
│         └─────────────── REPEAT ─────────────────┘                      │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Interrupt Chain Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         IRQ HANDLER CHAIN                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Hardware Event (I2C transfer complete)                                  │
│         │                                                                │
│         ▼                                                                │
│  ┌─────────────────┐                                                    │
│  │      NVIC       │  Nested Vectored Interrupt Controller              │
│  └─────────────────┘                                                    │
│         │                                                                │
│         ▼                                                                │
│  ┌─────────────────────────────────────────┐                            │
│  │  IRQ Handler (stm32h7xx_it.c)           │                            │
│  │  - I2C1_EV_IRQHandler()  → Master events│                            │
│  │  - I2C1_ER_IRQHandler()  → Master errors│                            │
│  │  - I2C4_EV_IRQHandler()  → Slave events │                            │
│  │  - I2C4_ER_IRQHandler()  → Slave errors │                            │
│  └─────────────────────────────────────────┘                            │
│         │                                                                │
│         ▼                                                                │
│  ┌─────────────────────────────────────────┐                            │
│  │  HAL Handler (stm32h7xx_hal_i2c.c)      │                            │
│  │  - HAL_I2C_EV_IRQHandler(&hi2cX)        │                            │
│  │  - HAL_I2C_ER_IRQHandler(&hi2cX)        │                            │
│  └─────────────────────────────────────────┘                            │
│         │                                                                │
│         ▼                                                                │
│  ┌─────────────────────────────────────────┐                            │
│  │  User Callbacks (main.c)                │                            │
│  │  - HAL_I2C_MasterTxCpltCallback()       │                            │
│  │  - HAL_I2C_MasterRxCpltCallback()       │                            │
│  │  - HAL_I2C_SlaveRxCpltCallback()        │                            │
│  │  - HAL_I2C_SlaveTxCpltCallback()        │                            │
│  └─────────────────────────────────────────┘                            │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Callback Function Details

### Master Callbacks (I2C1)

| Callback | Trigger | Action |
|----------|---------|--------|
| `HAL_I2C_MasterTxCpltCallback` | Master finished transmitting | Sets `masterRxReady = 1` flag |
| `HAL_I2C_MasterRxCpltCallback` | Master finished receiving | Sets `masterTxReady = 1` flag |

### Slave Callbacks (I2C4)

| Callback | Trigger | Action |
|----------|---------|--------|
| `HAL_I2C_SlaveRxCpltCallback` | Slave received data from master | Arms `HAL_I2C_Slave_Transmit_IT()` |
| `HAL_I2C_SlaveTxCpltCallback` | Slave finished transmitting | Re-arms `HAL_I2C_Slave_Receive_IT()` |

---

## ⚠️ CRITICAL: ISR Delay Violation

### The Problem

```c
// ❌ WRONG - This will cause system hang!
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    HAL_Delay(100);  // DEADLOCK! System freezes here!
    HAL_I2C_Master_Transmit_IT(...);
}
```

### Why It Fails

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    HAL_Delay() DEADLOCK IN ISR                           │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  1. I2C1_EV_IRQHandler executes (high priority)                         │
│         │                                                                │
│         ▼                                                                │
│  2. HAL_I2C_MasterRxCpltCallback called                                 │
│         │                                                                │
│         ▼                                                                │
│  3. HAL_Delay(100) called                                               │
│         │                                                                │
│         ▼                                                                │
│  4. HAL_Delay waits for HAL_GetTick() to increment                      │
│         │                                                                │
│         ▼                                                                │
│  5. HAL_GetTick() is updated by SysTick_Handler                         │
│         │                                                                │
│         ▼                                                                │
│  6. SysTick_Handler CANNOT execute!                                     │
│     - We're still in I2C IRQ context                                    │
│     - SysTick has equal or lower priority                               │
│     - IRQ preemption blocked                                            │
│         │                                                                │
│         ▼                                                                │
│  7. ∞ DEADLOCK - HAL_Delay waits forever                                │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### The Solution

```c
// ✅ CORRECT - Use flags and handle delays in main loop

// In callback (ISR context) - just set flag
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    masterTxReady = 1;  // Quick, non-blocking
}

// In main loop (non-ISR context) - safe to delay
while (1) {
    if (masterTxReady) {
        masterTxReady = 0;
        HAL_Delay(100);  // Safe here!
        HAL_I2C_Master_Transmit_IT(...);
    }
}
```

---

## Data Buffers

| Buffer | Size | Content | Direction |
|--------|------|---------|-----------|
| `txMData` | 23 bytes | "Hello from CM7 Master!" | Master → Slave |
| `rxSData` | 23 bytes | Received master data | Slave receives |
| `txSData` | 53 bytes | "Hello from CM7 Master! Positive response from slave!" | Slave → Master |
| `rxMData` | 53 bytes | Received slave response | Master receives |

---

## Timing Diagram

```
Time ──────────────────────────────────────────────────────────────────────>

Master TX    ████████░░░░░░░░░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░░░░░░░░░
                    │                         │
                    │ 1ms                     │
                    ▼                         ▼
Master RX    ░░░░░░░░████████░░░░░░░░░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░
                            │                             │
                            │ 100ms                       │
                            ▼                             ▼
             ├──────────────┼─────────────────────────────┼────────────────
             Cycle 1                                Cycle 2

Slave RX     ████████░░░░░░░░░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░░░░░░░░░
                    │                         │
Slave TX     ░░░░░░░░████████░░░░░░░░░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░
```

---

## Files Structure

```
I2C_h745zi_master_slave/
├── CM7/
│   └── Core/
│       ├── Src/
│       │   ├── main.c              ← Main application + callbacks
│       │   ├── stm32h7xx_it.c      ← IRQ handlers
│       │   └── stm32h7xx_hal_msp.c ← GPIO + NVIC configuration
│       └── Inc/
│           ├── main.h
│           └── stm32h7xx_it.h
└── I2C_WORKFLOW.md                  ← This document
```

---

## Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No communication | Wires not connected | Check PB8↔PF14, PB9↔PF15 |
| NACK errors | Wrong slave address | Verify `I2C_Slave_ADDRESS` matches I2C4 OwnAddress1 |
| System hangs | HAL_Delay in ISR | Move delays to main loop using flags |
| Intermittent failures | Missing pull-ups | Add 4.7kΩ external pull-ups |
| Only first TX works | Missing re-arm | Ensure callbacks re-arm RX/TX |

---

## Version History

| Date | Change |
|------|--------|
| 2026-01-11 | Initial implementation with ISR delay fix |
| 2026-01-11 | Changed delay between Master TX and RX from 20ms to 1ms |
| 2026-01-11 | Added comprehensive workflow documentation |

---

## Step-by-Step Workflow Explanation

This section explains the complete I2C communication flow in detail, step by step.

### What is a Callback?

A **callback** is a function that gets called automatically by the system when something happens. You don't call it directly - the hardware/HAL calls it for you when an event occurs (like "transmission finished").

Think of it like ordering food at a restaurant:
- You place your order (start I2C transmission)
- You don't stand at the kitchen waiting - you sit down
- The waiter calls your name when food is ready (callback)
- You then take action (start next operation)

---

### Phase 1: System Startup (Initialization)

**Step 1: MCU Powers On**
```
Power ON → HAL_Init() → SystemClock_Config() → GPIO_Init() → I2C1_Init() → I2C4_Init()
```
- All peripherals are configured
- I2C1 is set up as Master (no address needed)
- I2C4 is set up as Slave with address 0x02

**Step 2: Arm the Slave Receiver**
```c
HAL_I2C_Slave_Receive_IT(&hi2c4, rxSData, sizeof(rxSData));
```
- This tells I2C4: "Be ready! When someone sends data to address 0x02, store it in `rxSData`"
- The slave is now **listening** on the I2C bus
- `_IT` means "Interrupt mode" - it won't block, just waits in background

**Step 3: Start First Master Transmission**
```c
HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_Slave_ADDRESS, txMData, sizeof(txMData));
```
- Master (I2C1) starts sending "Hello from CM7 Master!" to address 0x02
- This is non-blocking - code continues to main loop immediately
- The actual transmission happens in the background via interrupts

---

### Phase 2: First Communication Cycle

**Step 4: Master Transmits Data on I2C Bus**
```
I2C Bus Activity:
[START] [0x02+W] [ACK] [H] [e] [l] [l] [o] [...] [!] [ACK] [STOP]
```
- Master sends START condition
- Master sends slave address (0x02) with Write bit
- Slave acknowledges (ACK)
- Master sends each byte of "Hello from CM7 Master!"
- Slave acknowledges each byte
- Master sends STOP condition

**Step 5: Hardware Triggers I2C1 Event Interrupt**
```
Hardware Event: "I2C1 transmission complete!"
    │
    ▼
NVIC activates I2C1_EV_IRQn
    │
    ▼
I2C1_EV_IRQHandler() in stm32h7xx_it.c executes
    │
    ▼
HAL_I2C_EV_IRQHandler(&hi2c1) processes the event
    │
    ▼
HAL detects: "This was a Master TX complete event"
    │
    ▼
HAL calls: HAL_I2C_MasterTxCpltCallback(&hi2c1)  ← YOUR CODE RUNS HERE
```

**Step 6: MasterTxCpltCallback Executes**
```c
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        masterRxReady = 1;  // Just set a flag, nothing else!
    }
}
```
- This callback runs in **interrupt context** (ISR)
- We CANNOT use `HAL_Delay()` here (would freeze the system)
- We simply set `masterRxReady = 1` to signal the main loop
- Callback exits quickly, returning control to normal operation

**Step 7: Simultaneously - Slave Receives Data**
```
While master was transmitting, the slave was receiving!
```
- I2C4 hardware automatically stored incoming bytes in `rxSData`
- When all expected bytes received, hardware triggers I2C4 event interrupt

**Step 8: Hardware Triggers I2C4 Event Interrupt**
```
Hardware Event: "I2C4 reception complete!"
    │
    ▼
I2C4_EV_IRQHandler() → HAL_I2C_EV_IRQHandler(&hi2c4)
    │
    ▼
HAL calls: HAL_I2C_SlaveRxCpltCallback(&hi2c4)
```

**Step 9: SlaveRxCpltCallback Executes**
```c
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C4) {
        // rxSData now contains "Hello from CM7 Master!"
        HAL_I2C_Slave_Transmit_IT(&hi2c4, txSData, sizeof(txSData));
    }
}
```
- The received message is now in `rxSData` buffer
- We immediately arm the slave transmitter with response data
- Slave is now ready: "When master asks for data, send `txSData`"

---

### Phase 3: Main Loop Processes Flag

**Step 10: Main Loop Detects masterRxReady Flag**
```c
while (1) {
    if (masterRxReady) {        // Flag was set in Step 6
        masterRxReady = 0;       // Clear the flag
        HAL_Delay(1);            // Wait 1ms (SAFE here in main loop!)
        HAL_I2C_Master_Receive_IT(&hi2c1, I2C_Slave_ADDRESS, rxMData, sizeof(rxMData));
    }
    // ... rest of loop
}
```
- Main loop runs continuously, checking flags
- When `masterRxReady == 1`, we know master TX finished
- We clear the flag to prevent re-triggering
- `HAL_Delay(1)` is SAFE here because we're NOT in an interrupt
- We start master receive operation

---

### Phase 4: Master Receives Slave Response

**Step 11: Master Requests Data from Slave**
```
I2C Bus Activity:
[START] [0x02+R] [ACK] [H] [e] [l] [l] [o] [...response...] [!] [NACK] [STOP]
```
- Master sends START condition
- Master sends slave address (0x02) with Read bit
- Slave acknowledges
- Slave sends each byte of response (txSData)
- Master acknowledges each byte (NACK on last byte)
- Master sends STOP condition

**Step 12: Hardware Triggers I2C1 Event Interrupt (RX Complete)**
```
I2C1_EV_IRQHandler() → HAL_I2C_EV_IRQHandler(&hi2c1)
    │
    ▼
HAL calls: HAL_I2C_MasterRxCpltCallback(&hi2c1)
```

**Step 13: MasterRxCpltCallback Executes**
```c
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        // rxMData now contains slave's response!
        masterTxReady = 1;  // Signal: ready for next TX cycle
    }
}
```
- `rxMData` now contains "Hello from CM7 Master! Positive response from slave!"
- We set `masterTxReady = 1` to signal main loop

**Step 14: Hardware Triggers I2C4 Event Interrupt (TX Complete)**
```
I2C4_EV_IRQHandler() → HAL_I2C_EV_IRQHandler(&hi2c4)
    │
    ▼
HAL calls: HAL_I2C_SlaveTxCpltCallback(&hi2c4)
```

**Step 15: SlaveTxCpltCallback Executes**
```c
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C4) {
        // Re-arm slave receiver for next incoming message
        HAL_I2C_Slave_Receive_IT(&hi2c4, rxSData, sizeof(rxSData));
    }
}
```
- Slave finished sending response
- We re-arm the receiver to be ready for next master transmission
- One complete cycle is now finished!

---

### Phase 5: Cycle Repeats

**Step 16: Main Loop Detects masterTxReady Flag**
```c
while (1) {
    // ... masterRxReady handling ...
    
    if (masterTxReady) {         // Flag was set in Step 13
        masterTxReady = 0;        // Clear the flag
        HAL_Delay(100);           // Wait 100ms between cycles
        HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_Slave_ADDRESS, txMData, sizeof(txMData));
    }
}
```
- Main loop detects `masterTxReady == 1`
- Waits 100ms (inter-cycle delay)
- Starts new master transmission
- **Go back to Step 4** - cycle repeats forever!

---

### Complete Cycle Summary

```
┌─────────────────────────────────────────────────────────────────────────┐
│  STEP   │  LOCATION          │  ACTION                                 │
├─────────┼────────────────────┼─────────────────────────────────────────┤
│   1-3   │  main()            │  Initialize and start first TX          │
│    4    │  I2C Hardware      │  Master transmits on bus                │
│   5-6   │  ISR → Callback    │  MasterTxCplt sets masterRxReady=1      │
│   7-9   │  ISR → Callback    │  SlaveRxCplt arms slave TX              │
│   10    │  Main Loop         │  Detect flag, delay 1ms, start RX       │
│   11    │  I2C Hardware      │  Master receives from slave             │
│  12-13  │  ISR → Callback    │  MasterRxCplt sets masterTxReady=1      │
│  14-15  │  ISR → Callback    │  SlaveTxCplt re-arms slave RX           │
│   16    │  Main Loop         │  Detect flag, delay 100ms, start TX     │
│         │                    │  → REPEAT FROM STEP 4                   │
└─────────────────────────────────────────────────────────────────────────┘
```

---

### Why Flags Instead of Direct Calls?

**❌ BAD: Calling next operation directly in callback with delay**
```c
void HAL_I2C_MasterTxCpltCallback(...) {
    HAL_Delay(1);  // DEADLOCK! SysTick can't run!
    HAL_I2C_Master_Receive_IT(...);
}
```

**✅ GOOD: Using flags**
```c
void HAL_I2C_MasterTxCpltCallback(...) {
    masterRxReady = 1;  // Fast! Just set a variable
}

// In main loop (NOT in interrupt)
if (masterRxReady) {
    masterRxReady = 0;
    HAL_Delay(1);  // Safe! SysTick can run normally
    HAL_I2C_Master_Receive_IT(...);
}
```

**Key Insight**: Callbacks run inside interrupts. Keep them SHORT and FAST. Do the heavy work (delays, complex logic) in the main loop by using flags as signals.
