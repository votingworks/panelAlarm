# HID / TTY Panel Alarm System

This repository contains the Arduino code for a HID / TTY Panel Alarm System. The system utilizes a SparkFun Qwiic Button to detect panel states and communicates these states to a host computer using HID (Human Interface Device) emulation. It also handles commands and acknowledgements over a UART serial connection.
## Description

The HID / TTY Panel Alarm System is designed to monitor the state of a security panel, indicating whether it is secured or unsecured. It uses a Qwiic Button connected via I2C to detect the state. The system sends this state to a host computer and waits for commands and acknowledgements. The code is structured to handle different panel states and respond to commands like ARM, UNARM, and ACK.
## Installation

To use this code, you will need:

* An Arduino board (such as Arduino Uno or SparkFun Pro Micro)
* SparkFun Qwiic Button
* Arduino IDE for compiling and uploading the code to your board

Steps:

1. Install the Arduino IDE if you haven't already from the Arduino website.
2. Open the Arduino IDE, and add the SparkFun Boards URL to the Arduino Board Manager:
   - Go to File > Preferences.
   - In the "Additional Boards Manager URLs" field, add the following URL:
https://raw.githubusercontent.com/sparkfun/Arduino_Boards/main/IDE_Board_Manager/package_sparkfun_index.json
   - Click "OK" to save.
3. Install the SparkFun Board definitions:
   - Go to Tools > Board > Boards Manager....
   - Search for "SparkFun" and install the package by SparkFun Electronics.
4. Install the SparkFun Qwiic Button library:
   - Go to Sketch > Include Library > Manage Libraries....
   - In the Library Manager, search for "SparkFun Qwiic Button".
   - Find the library in the list and click "Install".
5. Connect your Arduino board to your computer.
6. In the Arduino IDE, select your board and processor:
   - Go to Tools > Board and select "SparkFun Pro Micro".
   - Go to Tools > Processor and select "ATmega32U4 (5V, 16 MHz)".
7. Clone this repository or download the source code.
8. Open the downloaded .ino file in the Arduino IDE.
9. Upload the code to your Arduino board.

## Usage

Once the code is uploaded to the Arduino, the system starts in an 'unarmed' state. It waits for the ARM command to transition to the 'armed' state. In the 'armed' state, it continuously monitors the panel's state (secured or unsecured) and communicates this to the host computer. The arduino listens for commands to arm, disarm, or acknowledge the current state. 

* sw commands are sent with format `<CMD>`
* acceptable commands are `<ARM>` `<UNARM>` and `<ACK>`
  * `<ACK>` is useless unless you include the alert your acknowledging, either `[` or `]`
  * eg `<ACK [>`
* unacknowledged commands currently spam at a frequency of 5000 ms because it's hard to develop when your keyboard is hitting errant keys by itself
* after an `<ACK>` and only after an `<ACK>` can you `<UNARM>`
* commands are sent via /dev/ttyACM* 
* i currently have `#define DEBUG true` so /dev/ttyACM* is also going to have debug messages 
