# Smart curtain: light-controlled window automation

**Team design project** · MEC 830 Mechatronics Systems Design · Fall 2025<br>
**Group 02:** Nitin Mahay, Joshua Jeyasanker, Dante Rieger, Elti Tabaku<br>
**Tools:** Arduino Uno (C++) · TinkerCAD · KiCad · Solid Edge

![Solid Edge assembly with the front panel off, showing the servo and slider-crank](images/cad-mechanism.png)

An Arduino reads a light sensor and drives a servo and slider-crank to open, half-open or close a curtain on its own, with manual and remote modes that override the automation.

## The brief

Turn our Phase 1 control logic into a working hardware prototype: a low-cost curtain that opens in daylight and closes at night by itself, while still letting the user take over with a button or a remote. Built from an Arduino Uno kit and inexpensive materials, and it had to work consistently.

## My role

- Proposed one of the team's four design concepts (photoresistor sensing, push-button control, a slider design and a popsicle-stick curtain rod), the combination the final prototype was built on
- Worked with the team on mechanism selection, testing and the final report

## How we designed it

1. **Four concepts, one morphological chart.** All used an Arduino Uno, a servo and an IR remote; three used a photoresistor, push buttons and a slider.
2. **Three mechanisms prototyped.** A roll-up shaft, a string pulley and a slider-crank. Only the slider-crank opened and closed the curtain consistently.
3. **Linkage sized** at a 4 in crank and a 6.5 in slider link, through calculation and trial and error:

| Servo angle | Slider reach | Curtain |
|---|---|---|
| 0° | 6.5 in | Closed |
| 90° | Partway | Half open |
| 180° | 0 in (links overlap) | Fully open |

![Curtain function drawing](images/slider-crank-drawing.png)

## Simulation first: Tinkercad

Before building hardware, we built and tested the whole control system in a Tinkercad circuit simulation: Arduino Uno, photoresistor, micro servo, two push buttons, an IR receiver and remote, and a 16×2 LCD showing the current mode.

![Tinkercad circuit: Arduino Uno wired to a breadboard with photoresistor, two push buttons, potentiometer, IR receiver, micro servo and a 16x2 LCD, plus an IR remote](images/tinkercad-circuit.png)

| Signal | Pin |
|---|---|
| Photoresistor (light level) | A0 |
| Servo | D9 |
| Open / close buttons | D2 / D12 (internal pull-ups) |
| IR receiver | D7 |
| LCD (RS, E, D4 to D7) | D4, D5, D6, D8, D10, D11 |

What the simulation version does:

- **Automatic:** opens above 700 and closes below 300; in between it holds its state, so light hovering near one level can't make the curtain flutter.
- **Manual:** open and close buttons.
- **Wireless:** a real IR remote and receiver. Remote buttons map to open, close, toggle and a noise test.
- **Noise test:** typing `NOISE` in the Serial Monitor (or the remote) adds ±50 of random noise to the light reading, to check the thresholds stay stable with a jittery sensor.
- **Serial commands** `AUTO`, `MANUAL` and `WIRELESS` switch modes, and the LCD shows the mode and noise status.

The hardware prototype then added a half-open position, smooth servo motion and a dedicated mode button.

## Electronics

- Photoresistor voltage divider on A0 (0 to 1023), the only analog signal
- Open, close, half-open and mode buttons on D2, D3, D6 and D4 with internal pull-ups and software debouncing
- Remote input on D5; IR receiver module on 5 V, ground and one digital pin
- Servo on PWM pin D9; everything powered from the Arduino's 5 V USB supply

![KiCad schematic](images/schematic.png)

## Control logic

A three-mode state machine, cycled by the mode button:

| Mode | How it decides |
|---|---|
| Automatic | Above 700: fully open. 500 to 700: half open. Below 300: closed. Between 300 and 500 it holds its last position. |
| Manual | Open, half and close buttons set the position directly. |
| Remote | The remote button cycles closed, half, open. Serial commands O, H, C, T and S open, half-open, close, cycle or report status. |

The servo moves 2° at a time with a 15 ms pause, so the curtain glides instead of snapping and the motor and cardboard linkage are spared shock loads.

## Build

Cardboard frame and links, a popsicle-stick rod, zip-tie curtain rings, and a front panel that hides the mechanism.

## Code

| File | What it is |
|---|---|
| [`code/simulation/smart_curtain_sim.ino`](code/simulation/smart_curtain_sim.ino) | Tinkercad simulation: open/closed with hysteresis, IR remote, LCD, noise test |
| [`code/hardware/smart_curtain.ino`](code/hardware/smart_curtain.ino) | Hardware prototype: open, half and closed positions, smooth servo motion, mode button, serial commands |

Both run on an Arduino Uno. The simulation sketch needs the `IRremote` and `LiquidCrystal` libraries; the hardware sketch only needs `Servo`.

## Results and what we'd change

The curtain moved smoothly between all three positions and followed the light thresholds and user inputs in testing.

- On the hardware, remote control was stood in for by a button and serial commands; the IR receiver code already works in the Tinkercad version and can be carried over
- 3D-printed links and frame instead of cardboard
- Tune the 300 / 500 / 700 thresholds against real window light
- LED indicators for the current mode

Case study on my website: [danterieger.github.io/projects/smart-curtain.html](https://danterieger.github.io/projects/smart-curtain.html)
