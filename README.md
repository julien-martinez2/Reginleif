# 🕷️ Reginleif — Autonomous Quadruped Walking Robot

[![Platform](https://img.shields.io/badge/Platform-STM32G031%20%7C%20Raspberry%20Pi-blue.svg)](https://www.st.com)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17%20%2F%20C-orange.svg)]()
[![Hardware](https://img.shields.io/badge/EDA-KiCad%20(4--Layer%20PCBs)-green.svg)]()
[![Bus](https://img.shields.io/badge/Bus-RS--485%20%7C%20SPI%20%7C%20UART%20DMA-red.svg)]()
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](LICENSE)

> A custom-engineered, distributed quadruped combat walker inspired by the **XM2 Reginleif** and **M1A4 Juggernaut** from the acclaimed light novel & anime series ***86 - Eighty Six***.

---

## 📖 Table of Contents

- [Motivation & Inspiration](#-motivation--inspiration)
- [System Architecture](#-system-architecture)
  - [Functional Architecture](#functional-architecture)
  - [Technical & Electrical Architecture](#technical--electrical-architecture)
- [Hardware & Key Components](#-hardware--key-components)
  - [Sun Board (Central Controller)](#sun-board)
  - [Planet Boards (Legs Controllers)](#planet-boards)
  - [Moon Boards (Angle Sensors)](#moon-boards)
  - [Dual-Encoder Sensing Strategy](#dual-encoder-sensing-strategy)
- [Firmware Architecture](#-firmware-architecture)
  - [NVIC Preemptive Priority Hierarchy](#nvic-preemptive-priority-hierarchy)
  - [Real-Time Timer Scheduling](#real-time-timer-scheduling)
  - [Zero-CPU UART DMA with IDLE Line Detection](#zero-cpu-uart-dma-with-idle-line-detection)
- [Project Documentation & CAD Assets](#-project-documentation--cad-assets)
- [Roadmap & Deliverables](#-roadmap--deliverables)
- [Author & Community](#-author--community)
- [License](#-license)

---

## 💡 Motivation & Inspiration

The goal of this project is to take the agility, distinctive gait mechanics, and rugged aesthetic of the **XM2 Reginleif** from *86 - Eighty Six* and bring it into physical reality.

Building a multi-jointed legged walker from scratch serves as an end-to-end engineering challenge across multiple disciplines:
1. **Developing Real-Time Embedded Skills:** Mastering bare-metal ARM Cortex-M control, deterministic task scheduling, multi-bus SPI synchronization, and DMA streaming.
2. **Custom PCB & Power Electronics:** Designing compact 4-layer PCBs, handling thermal dissipation, power distribution for 16 simultaneous DC gearmotors, and integrated eFuse circuit protection.
3. **Complex Kinematics & Robotics:** Solving 3D inverse kinematics, gait generation, and closed-loop position control with mechanical backlash compensation.

**Open Source Philosophy:** Advanced quadruped robotics often relies on expensive off-the-shelf smart actuators or proprietary platforms. By documenting every schematic, trade-off, calculation, and line of code, this repository aims to serve as a practical, open reference for robotics students, engineers, and hobbyists building their own legged systems.

---

## 🏛️ System Architecture

### Functional Architecture

The robot employs a **hierarchical distributed architecture**, offloading high-frequency motor control to dedicated microcontrollers while centralizing planning, telemetry, and stabilization:


```
                      ┌────────────────────────────────────────┐
                      │        High-Level Intelligence         │
                      │     Raspberry Pi (Vision, Autonomy)    │
                      └──────────────────┬─────────────────────┘
                                         │ SPI / UART
                                         ▼
                      ┌────────────────────────────────────────┐
                      │    Central Master MCU ("Sun Board")    │
                      │  • IMU Body Stabilization (BNO086)     │
                      │  • Power Distribution & Battery Monitor│
                      │  • Master Gait Coordinator             │
                      └──────────────────┬─────────────────────┘
                                         │
          ┌──────────────────────────────┼──────────────────────────────┐
          │ RS-485 Differential Bus      │ (Master-Slave Protocol)      │
          ▼                              ▼                              ▼
 ┌──────────────────┐           ┌──────────────────┐           ┌──────────────────┐
 │  Planet Board 1  │           │  Planet Board 2  │           │ Planet Boards 3-4│
 │ (STM32G031)      │           │ (STM32G031)      │           │  (STM32G031)     │
 │ 4x N20 Motors    │           │ 4x N20 Motors    │           │ 4x N20 Motors    │
 └──────────────────┘           └──────────────────┘           └──────────────────┘
          │                     SPI Bus  │ (Master-Slave Protocol)      │
          ▼                              ▼                              ▼
 ┌──────────────────┐           ┌──────────────────┐           ┌──────────────────┐
 │ Moon Boards 1-4  │           │ Moon Boards 5-8  │           │ Moon Boards 9-16 │
 │ (MA730 Abs.)     │           │ (MA730 Abs.)     │           │ (MA730 Abs.)     │
 └──────────────────┘           └──────────────────┘           └──────────────────┘

 

```

---

### Technical & Electrical Architecture


```
                              [ 3S LiPo Battery (11.1V - 12.6V) ]
                                              │
                                              ▼
                                 [ TPS259827 15A eFuse ]
                                (UVLO Cutoff at 10.83V)
                                              │
                  ┌───────────────────────────┼───────────────────────────┐
                  │ VM (12V) Motor Power      │                           │
                  ▼                           ▼                           ▼
        ┌───────────────────┐       ┌───────────────────┐       ┌──────────────────────┐
        │ AP63205 (5V Buck) │       │ AP63203 (3.3V)    │       │ TPS22919 Load SW     │
        │ -> Raspberry Pi   │       │ -> Sun Board MCU  │       │ -> Planet Board VDD  │
        └───────────────────┘       └───────────────────┘       └─────────┬────────────┘
                                                                          │
                                  ┌───────────────────────────────────────┘
                                  │ 5-Pin JST-PA Harness (12V, 3.3V, GND, RS485_A/B)
                                  ▼
         ┌─────────────────────────────────────────────────────────────────┐
         │                     MODULAR PLANET BOARD (x4)                   │
         │                                                                 │
         │   [ THVD1406 RS-485 ] <── UART DMA ──> [ STM32G031 MCU ]        │
         │                                              │                  │
         │              ┌───────────────────────────────┴────────┐         │
         │              ▼ (SPI1 Bus)                             ▼ (SPI2)  │
         │     [ 4x MA730 Encoders ]                       [ DRV8908 ]     │
         │     Absolute Joint Angles                   8x Half-Bridges     │
         │     (Diametric Magnets)                               │         │
         │              ▲                                        ▼         │
         │              │ Joint Output Shaft              4x CHF-GW12T-N20 │
         │              └──────────────────────────────── (302:1 Worm)     │
         │                                                       │         │
         │   [ Incremental Encoders ] <── EXTI (PA0, PB1, PB3, PC14) ─────┘│
         └─────────────────────────────────────────────────────────────────┘

```

---

## 🛠️ Hardware & Key Components

### Sun Board
- **Master Processing:** STM32G031 / Raspberry Pi pairing for autonomous operation, telemetry streaming, and wireless communication.
- **Power & Circuit Protection:**
  - **TPS259827 15A eFuse:** Ultra-fast short-circuit protection and hardware programmable undervoltage lockout (V_UVLO = 10.83 V), preventing 3S LiPo over-discharge).
  - **AP63205 & AP63203:** High-efficiency synchronous buck converters generating 5 V (logic/SBC) and 3.3 V (peripherals).
  - **TPS22919 Load Switches:** Independent hardware-level logic power isolation and current limiting for each peripheral board.
- **Inertial Measurement:** **BNO086** 9-axis sensor fusion IMU over SPI for real-time body attitude stabilization.

### Planet Boards
- **Dedicated MCU:** **STM32G031G8U6** (ARM Cortex-M0+ running at 16 MHz, 4-layer compact layout with solid internal ground planes).
- **Motor Driver:** **DRV8908** automotive-grade 8 half-bridge driver, providing 4 fully independent bi-directional motor channels with individual hardware current limiting and fault telemetry.
- **Bus Transceiver:** **THVD1406DR** RS-485 transceiver with auto-direction sensing, eliminating the need for half-duplex DE/RE switching overhead.
- **Connectors:** JST-PA (power and differential communication) and JST-ZH (motor and sensor interconnects).

### Moon Boards
- **Angle Sensor:** MA730 is a magnetic angle sensor, which is placed next to the shaft of the motor on which a magnet is put.
- **Connectors:** JST-ZH (same as the motors).

---

### Dual-Encoder Sensing Strategy

Miniature worm gearboxes provide high stall torque and self-locking capabilities, but introduce mechanical backlash. To solve this, a dual-encoder feedback architecture is implemented:

| Sensor Type | Part & Interface | Resolution | Placement | Role in Control Loop |
| :--- | :--- | :--- | :--- | :--- |
| **Incremental Magnetic Encoder** | 2-Phase Hall (EXTI) | 2x Quadrature (~2107 counts/rev on motor shaft) | Motor Tail Shaft | **High-frequency velocity estimation** and instantaneous PID rate control. |
| **Absolute Magnetic Encoder** | **MA730GQ-Z** (SPI1) | 14-bit (16,384 counts/rev) | Output Joint Shaft (with Diametric Ring Magnets) | **Absolute angle calibration (10 Hz)** to eliminate gearbox play, backlash, and drift. |

---

## 💻 Firmware Architecture

### NVIC Preemptive Priority Hierarchy

The Cortex-M0+ interrupt controller is prioritized to guarantee that critical hardware signals and high-speed encoder transitions are never delayed by slower math computations:


```

Priority 0 (Highest) ──► SysTick & EXTI Lines (PA0, PB1, PB3, PC14, PA2 nFAULT)
│
Priority 1             ──► USART1 Communication & DMA Channels
│
Priority 2             ──► TIM2 (DRV8908 SPI Updates) & TIM3 (MagAlpha SPI Reads)
│
Priority 3 (Lowest)    ──► TIM1 (Software Inverse Kinematics & PID Loops)

```

- **Zero-Jitter Pulse Counting:** Encoders run at Priority 0 so wheel pulses are captured without dropped ticks during motor acceleration.
- **Fault Handling:** The open-drain `nFAULT` pin on `PA2` triggers an immediate shutdown sequence on overcurrent or thermal warnings.
- **Safe Preemption:** Heavy inverse kinematics floating-point calculations (Priority 3) safely yield execution whenever a peripheral event occurs.

---

### Real-Time Timer Scheduling

| Timer | Execution Frequency | Period | Task Description |
| :--- | :--- | :--- | :--- |
| **EXTI Lines** | Asynchronous | < 1 µs | Hardware edge-triggered incremental encoder decoding (Motors 1–4). |
| **TIM3** | **10 Hz** | 100 ms | SPI1 sequential polling of the 4 joint-level MA730 absolute encoders. |
| **TIM2** | **50 Hz** | 20 ms | SPI2 multi-channel register write updating DRV8908 motor PWM/directions. |
| **TIM1** | **20 Hz - 50 Hz** | 20 - 50 ms | Inverse kinematics trajectory solving, PID position/velocity loop execution. |

---

### Zero-CPU UART DMA with IDLE Line Detection

Communication between the central master board and the leg modules uses DMA combined with hardware IDLE line detection (`HAL_UARTEx_ReceiveToIdle_DMA`):

```cpp
// Non-blocking packet reception
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART1) {
        // Validate header and packet checksum
        if (Size >= 11 && rx_buffer[0] == 0xAA && rx_buffer[1] == 0x55) {
            ParseMasterCommand(rx_buffer, Size);
        }
        // Immediately re-arm DMA reception for the next packet
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, RX_BUFFER_SIZE);
    }
}

```

* **Zero CPU overhead** while bytes are on the wire.
* The MCU triggers a single interrupt only after an entire variable-length packet is delivered and the bus goes idle.

---

## 📂 Project Documentation & CAD Assets

All schematics, BOMs, mechanical studies, and test records are available in the repository:

* **Hardware Schematics & KiCad Layouts:** 4-layer Leg Board, Satellite Angle Sensor PCBs, and Central Carrier Board.
* **Mechanical CAD:** 3D printed joint housings, worm gearbox mounts, and chassis brackets.

---

## 🗺️ Roadmap & Deliverables

* [x] Functional specification and torque/power feasibility study
* [x] 4-layer Leg Board PCB design and component assembly (KiCad)
* [ ] Mechanical Design and 3D-printing (FreeCAD)
* [ ] Bare-metal firmware setup (CubeMX, SPI1/SPI2, multi-channel EXTI, UART DMA)
* [ ] MA730 absolute magnetic sensor driver (`magalpha.cpp`)
* [ ] DRV8908 8 half-bridge motor controller driver (`drv8908.cpp`)
* [ ] 3D Inverse Kinematics math library and body orientation matrix
* [ ] Dynamic walking gait generation (Crawl, Trot, and Creep gaits)
* [ ] Central Sun Board assembly & BNO086 IMU stabilization loop
* [ ] Auxiliary subsystems: Tilt-axis turret mechanism and deployable anchor system

---

## 👤 Author & Community

Designed and engineered with passion by **Julien Martinez** ([@julien-martinez2](github.com/julien-martinez2)).

If you are working on custom quadruped walkers, legged robotics, or distributed motor controllers, feel free to open an issue, start a discussion, or contribute!

---

## 📄 License

This project is distributed under the **MIT License**. See [`LICENSE`](https://www.google.com/search?q=LICENSE) for more details.
