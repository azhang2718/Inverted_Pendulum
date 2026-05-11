# Inverted Pendulum Control System

This page showcases an inverted pendulum on a cart, stabilized in real time by a custom PCB running a PID controller on a TI MSPM0G3507 as part of our final project for **ELEC 327 — Junior Design Lab**.

**Team:** Anthony Zhang · Bilin Yang · Brandon Sung · Setsuna Jiang

---

## Overview

The inverted pendulum is a classic control-theory benchmark: a rigid rod pivots freely on top of a cart that slides along a linear rail, and the controller must drive the cart left and right fast enough to keep the rod balanced upright. The upright position is inherently unstable, so any small tilt has to be detected and corrected before gravity wins.

Our implementation reads the pendulum angle from an AS5600 magnetic encoder over I²C, runs a PID loop at 200 Hz on a custom PCB, and drives a NEMA 17 stepper through a DRV8825 to move the cart along a V-slot aluminum rail.

---

## Hardware

| Component | Part | Role |
|---|---|---|
| Microcontroller | TI MSPM0G3507 | Runs the 200 Hz PID loop |
| Angle sensor | AS5600 magnetic encoder (I²C) | Reads pendulum angle (12-bit) |
| Magnet | Diametrically magnetized, 6 mm OD | Required for AS5600 to detect rotation |
| Motor | NEMA 17 stepper | Drives the cart |
| Motor driver | DRV8825 | STEP/DIR interface, 1/8 microstepping |
| Rail | V-slot aluminum extrusion + GT2 belt | Cart travel |
| Cart | V-slot gantry plate + V-wheels | Rides the rail |
| Bearings | 625RS | Pulley + idler support |
| Endstops | Limit switches (×2) | Homing on startup |
| Power | 12 V / 5 A supply | Motor + logic rail |
| Regulator | LM1117 3.3 V LDO | Logic supply *(see Known Issues)* |
| Pendulum | 6 mm × 300 mm steel rod | The thing being balanced |

The full PCB design is in `pcb/` as a KiCad project. Open `*.kicad_pro` in the KiCad project manager to view the schematic and layout

---

## Software

### Control loop

A standard PID controller on the pendulum angle:

- **P** — proportional response to current tilt
- **I** — corrects for slow drift / sensor offset
- **D** — damps oscillation by reacting to angular velocity

The loop runs at 200 Hz. PID gains are stored as integers scaled by 100 so all arithmetic stays in fixed-point — no FPU needed, and the timing is deterministic.

### `config.h`

Every tunable value in the project lives in a single header. The idea is that PID tuning is iterative — you change a number, flash, test, repeat — and having every constant in one place makes that loop fast and error-free.

Things in `config.h`:
- `KP`, `KI`, `KD` — PID gains (scaled ×100)
- `MIN_STEP_RATE`, `MAX_STEP_RATE` — motor speed bounds (prevents stall and runaway)
- `DEADBAND` — angle window where no correction is applied
- `ENGAGE_THRESHOLD` — angle within which the controller activates
- `GIVE_UP_THRESHOLD` — angle past which the controller disengages
- `INTEGRAL_CLAMP` — anti-windup limit on the integral term
- `MOTOR_DIR_SIGN` — direction polarity

### I²C driver (AS5600)

The angle sensor is read over I²C from the raw-angle register. The driver:

1. Waits for the bus to be idle
2. Flushes the TX FIFO
3. Writes the target register address
4. Reads back two bytes → 12-bit angle

Every busy-wait loop has a **timeout counter** that decrements and returns an error code if it hits zero. Without these, a single bad I²C transaction could lock the entire control loop — and on a pendulum that's trying to balance, a hang means it falls instantly.

### Startup sequence

1. Init peripherals (I²C, timers, GPIO)
2. Home the cart against the limit switches and zero the step counter
3. Move cart to center of the rail
4. Wait until the pendulum is within `ENGAGE_THRESHOLD` of vertical
5. Enable PID loop

---

## Building & Flashing

1. Install **Code Composer Studio** (or use the TI Arm Clang toolchain directly).
2. Open the firmware project, build, and flash via the SWD header on the PCB.
3. Adjust values in `inc/config.h` as needed and reflash to tune.

