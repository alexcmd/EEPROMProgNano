# EEPROM Programmer for Arduino Nano

An EEPROM programmer implementation for Arduino Nano, designed to program 28C16 or similar EEPROMs for retro computing projects.

## Overview

This project provides a simple and efficient way to program EEPROMs using an Arduino Nano. It's particularly useful for projects related to retro computing, such as Ben Eater's 6502 computer project.

## Inspiration

This project was inspired by Ben Eater's 6502 computer project, where he builds a complete 6502-based computer from scratch. The 6502 project requires programming EEPROMs to store program code and data. This EEPROM programmer provides an easy way to write data to these memory chips.

## Features

- Program 28C16 (2K x 8) EEPROMs or similar chips
- Simple serial interface for uploading binary data
- Automatic verification of written data
- Compatible with Arduino Nano and similar boards

## Hardware Requirements

- Arduino Nano
- Breadboard and jumper wires
- 28C16 EEPROM or compatible
- Additional components as specified in the schematic

## Usage

1. Connect the EEPROM to the Arduino Nano according to the pinout in the code
2. Upload this sketch to your Arduino Nano
3. Use the serial interface to send commands and data

## License

This project is open source and available under the MIT License.

## Acknowledgements

- Ben Eater for his excellent [6502 computer project](https://eater.net/6502) that inspired this work
- The Arduino community for their continuous support and resources
