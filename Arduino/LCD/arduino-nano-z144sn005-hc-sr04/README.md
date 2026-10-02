# Nano 3.3 V + Z144SN005 LCD + HC-SR04

This project draws a circle whose size and color represent the measured distance:

- At 2 cm: radius is approximately 62 pixels and the circle is red.
- From 2 to 200 cm: radius decreases linearly and color changes from red through yellow to green.
- At 200 cm: radius is 2 pixels and the circle is green.
- Beyond 200 cm or when there is no echo: the LCD displays `NO TARGET`.

## Required parts

- Arduino Nano whose processor and GPIO operate at 3.3 V
- Z144SN005 bare 1.44-inch 128x128 LCD with ST7735S controller
- Suitable 14-pin, 0.8 mm-pitch FPC/solder adapter for the bare LCD
- HC-SR04 ultrasonic module
- Regulated 3.3 V supply for the Nano and LCD
- Regulated 5 V supply for a standard HC-SR04
- 1 kohm resistor
- 2 kohm resistor, or two 1 kohm resistors in series
- 22 ohm resistor for the LCD backlight
- One 100 nF ceramic capacitor marked `104` for LCD decoupling
- One 4.7 to 10 uF capacitor for LCD bulk decoupling
- Jumper wires or a small PCB

The two capacitors are connected in parallel between the LCD 3.3 V node and common GND. If the 4.7-10 uF capacitor is polarized, connect its positive terminal to 3.3 V and negative terminal to GND.

## LCD wiring

| LCD FPC pin | Signal | Connection |
|---:|---|---|
| 1 | NC | Leave disconnected |
| 2 | GND | Common GND |
| 3 | LED- | Common GND |
| 4 | LED+ | 3.3 V through 22 ohm resistor |
| 5 | GND | Common GND |
| 6 | /RESET | Nano D8 |
| 7 | A0/DC | Nano D9 |
| 8 | SDA | Nano D11/MOSI |
| 9 | SCK | Nano D13/SCK |
| 10 | VCC | Regulated 3.3 V |
| 11 | IOVCC | Regulated 3.3 V |
| 12 | CS | Nano D10 |
| 13 | GND | Common GND |
| 14 | NC | Leave disconnected |

Join LCD pins 10 and 11 at the adapter and connect the joined node to 3.3 V. Place the 100 nF and 4.7-10 uF capacitors close to these pins.

## HC-SR04 wiring

| HC-SR04 pin | Connection |
|---|---|
| VCC | Regulated 5 V |
| GND | Common GND |
| TRIG | Nano D2 |
| ECHO | Through voltage divider to Nano D3, as shown below |

The standard HC-SR04 normally requires 5 V. Its ECHO output is also approximately 5 V, which can damage a 3.3 V Nano input. Use this divider:

```text
HC-SR04 ECHO ---- 1 kohm ----+---- Nano D3
                             |
                           2 kohm
                             |
Common GND ------------------+
```

The divider changes approximately 5 V to approximately 3.3 V. Do not reverse the resistor positions.

## Complete power schematic

```text
REGULATED 3.3 V
   +---- Nano 3.3 V logic supply only if permitted by the exact board
   +---- LCD pin 10 VCC
   +---- LCD pin 11 IOVCC
   +---- 22 ohm resistor ---- LCD pin 4 LED+
   +---- 100 nF capacitor ---+
   +---- 4.7-10 uF capacitor-+--- COMMON GND

REGULATED 5 V
   +---- HC-SR04 VCC

COMMON GND
   +---- Nano GND
   +---- 3.3 V supply negative/GND
   +---- 5 V supply negative/GND
   +---- HC-SR04 GND
   +---- LCD pin 2 GND
   +---- LCD pin 3 LED-
   +---- LCD pin 5 GND
   +---- LCD pin 13 GND
   +---- lower end of the ECHO divider's 2 kohm resistor
```

Never join the 5 V and 3.3 V positive rails. Only their grounds are joined.

Do not feed power into the `3V3` pin of a standard classic Nano when that pin is documented only as an output. Power the Nano according to its exact board documentation. The requirement here is that its GPIO voltage is 3.3 V and that the LCD receives a regulated 3.3 V supply.

## Libraries and upload

Install these libraries using Arduino Library Manager:

- Adafruit GFX Library
- Adafruit ST7735 and ST7789 Library

Open `arduino-nano-z144sn005-hc-sr04.ino`, select the correct Nano board and processor, then upload. Open Serial Monitor at 115200 baud to see the measured distance.

If the sensor always reports no target, first verify the 5 V supply and ECHO divider. Most HC-SR04 boards accept a 3.3 V TRIG signal, but if a particular clone does not, add a proper 3.3-to-5 V logic-level shifter on TRIG.
