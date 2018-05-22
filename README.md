# MeleeArduino
Arduino assisted Smash Bros. Melee

## ABOUT

AUTHOR:  Jeffrey Reeves
WEBSITE: https://alchemist.digital/
GITHUB:  https://github.com/JeffReeves
PURPOSE: Improves consistency in Super Smash Brothers Melee for the 
    Nintendo GameCube.

## FEATURES
- Three profile toggle using D-pad (Left, Right, Down)
- Perfect dashback that prevents tilt turn

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

## How to Wire Arduino and Logic Level Converter

![Diagram](https://raw.githubusercontent.com/JeffReeves/MeleeArduino/master/images/arduino-controller-diagram.png)