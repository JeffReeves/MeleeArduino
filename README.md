# MeleeArduino
Improves accessibility and consistency for human players in 
Super Smash Brothers Melee for the Nintendo GameCube.

## ABOUT

**AUTHOR**:  Jeffrey Reeves

**WEBSITE**: https://alchemist.digital/

**GITHUB**:  https://github.com/JeffReeves

**LICENSE**: GNU General Public License v3.0

## FEATURES
- Perfect dashback that prevents slower tilt turn state
- Dedicated shorthop (X) and fullhop (Y) buttons
- Always jumpcancel grab with Z button
- Always jumpcancel upsmash with C-Stick Up
- Dedicated light shield (L) button
- Three profile toggle using D-pad (Left, Right, Down)
- L+R+A+Start macro (dpad up)

### DEMO VIDEO

[![Melee Arduino Controller V1 Demo](https://raw.githubusercontent.com/JeffReeves/MeleeArduino/master/images/arduino-melee-thumbnail.png)](https://youtu.be/7cQj1xHGyI0 "Melee Arduino Controller V1 Demo")

## PREREQUISITES
- Arduino (UNO, Nano, etc.)
- Arduino IDE
- Logic Level Converter (3.3V to 5V)
- Controller (GameCube, extension cable, adapter, etc.)
- Game Console (GameCube, Wii, etc.)
- NicoHood's Nintendo Library for Arduino

## HOW TO INSTALL
1. Connect the Controller to the Arduino and the Logic Level Converter (see diagram below).
2. Connect Arduino to PC and open the Arduino IDE.
3. Select the correct Board and Port under the Tools menu.
4. Copy the entire contents of the melee.ino file from the melee folder. 
5. Paste over the current Arduino Sketch.
6. Select the Upload option to upload this code to the Arduino.
7. Disconnect the Arduino from the PC.
8. Plug in the Controller into the Game Console and start it up.
9. Enjoy playing Melee!

### NOTE 
Unless approved by a Tournament Organizer (TO), do **NOT** use this 
Arduino assistant during competitive play.

## How to Wire Arduino and Logic Level Converter

![Diagram](https://raw.githubusercontent.com/JeffReeves/MeleeArduino/master/images/arduino-controller-diagram.png)