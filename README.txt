1. Author:      vpos(VOs)
2. Project:     Li-Ion Battery Charger
3. Vers:        v1
4. Data:        30.01.2024       

#############################
Li-Ion Battery Charger
#############################

  This project is a Li-Ion battery charger based on the AVR microcontroller. It uses a MOSFET transistor to control the charging current and three LEDs to indicate the charging status.

  The charger supports charging batteries in the range from 2.7V to 4.2V at a current of 0.3A. The charger is powered by an external power supply in the range from 7V to 9V.

#############################
HARDWARE
#############################
  The charger requires the following hardware:

  - AVR microcontroller: Any AVR microcontroller with at least 8K of program memory and 512 bytes of RAM will work. The ATmega328P is a popular choice.
  - MOSFET transistor: A small-signal MOSFET transistor with a low gate threshold voltage is required. The IRF44N is a good choice.
  - 3 LEDs: Three LEDs are required to indicate the charging status. The LEDs should be connected to the microcontroller's GPIO pins. The following colors are used to indicate the charging status:
      - BLUE:   Power is on.
      - RED:    Charging is in progress.
      - GREEN:  Battery is fully charged.
    Battery: Any Li-Ion battery with a capacity of at least 1000mAh will work.

##############################
SOFTWARE
##############################

  The charger can be programmed using either the Arduino IDE or a text editor and compiler. The source code for the charger is available on GitHub.
Usage

  To use the charger, connect the battery to the charger's input terminals. Then, connect the LEDs to the microcontroller's GPIO pins. Finally, upload the charger's firmware to the microcontroller using your chosen programming method.

  The charger will automatically start charging the battery.

#############################
NEXT STEPS
#############################

  This project is one of the first steps towards developing a comprehensive device for monitoring the charge and state of Li-Ion batteries. Future work will focus on the following areas:

  - Improved charging efficiency: The charger could be improved by using a more efficient charging algorithm.
  - Extended battery life: The charger could be extended to improve battery life by using a trickle charging mode.
  - Additional features: The charger could be extended to include additional features, such as a battery temperature sensor.

