# Analysis of PB12 and PB13 on NUCLEO-H745ZI-Q Board

## Objective
The goal of this analysis was to determine why PB12 and PB13 did not show waveforms for FDCAN2 on the STM32H745ZI-Q board and to identify their hardware routing and accessibility.

---

## Findings

### 1. **Hardware Routing**
- **PB12**:
  - Exposed on the Morpho connector (CN12), making it accessible for probing.
  - Can be used as FDCAN2_RX.

- **PB13**:
  - Connected to the Ethernet PHY (LAN8742A) as **RMII_TXD1**.
  - Controlled by jumper **JP6** (indicated by "(6)" in the schematic).
  - **Not exposed** on the Morpho or Arduino headers, making it inaccessible for direct use.

### 2. **Impact of JP6**
- **JP6** is a jumper that connects PB13 to the Ethernet PHY circuit.
- Removing JP6 isolates PB13 from the Ethernet PHY, but the pin remains **physically inaccessible** because it is not routed to any external header.

### 3. **FDCAN2 Requirements**
- FDCAN2 requires both PB12 (FDCAN2_RX) and PB13 (FDCAN2_TX) to function.
- Since PB13 is not accessible, PB12 alone is pointless for FDCAN2 communication.

### 4. **Root Cause**
- PB13 is dedicated to Ethernet functionality on this board and is not broken out to headers.
- Even with JP6 removed, there is no physical access point to probe or use PB13.

---

## Alternate Solution
- **PB5 and PB6**:
  - Remapping FDCAN2 to PB5 (FDCAN2_RX) and PB6 (FDCAN2_TX) was successful.
  - These pins are accessible on the Morpho/Arduino headers and not routed to other peripherals.

---

## Recommendations
1. **Use PB5 and PB6**:
   - Since these pins work for FDCAN2, they are the best alternative.

2. **Custom Hardware Modifications** (Not Recommended):
   - If PB12 and PB13 must be used, hardware modifications would require soldering a wire directly to the PB13 MCU pin or trace.
   - This is risky and not recommended.

---

## Sources
1. **NUCLEO-H745ZI-Q Board Schematic**:
   - Shows PB13 connected to Ethernet PHY (LAN8742A) as RMII_TXD1.
   - The "(6)" notation next to PB13 indicates JP6 jumper control.
   - PB12 is shown on Morpho connector CN12.

2. **NUCLEO-H745ZI-Q User Manual (UM2408)**:
   - Documents JP6 as controlling PB13 connection to Ethernet.
   - Confirms Morpho connector pinout (PB12 accessible, PB13 not).

---

## Conclusion
PB13 is connected to the Ethernet PHY and is not exposed on any external header, making it inaccessible even with JP6 removed. PB12 is accessible but useless without PB13 for FDCAN2. **Remapping FDCAN2 to PB5 and PB6 is the practical solution**, which was verified to work successfully.