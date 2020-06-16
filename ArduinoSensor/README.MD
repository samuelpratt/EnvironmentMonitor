# What

A custom PCB to form the basis of a Mini Weather station that can upload to the internet.

This will upload to a Cloud Solution

# Contents: -

An Arduino nano. This is connected to the following: -
* A Serial connection to an ESP-01 Wifi Module
* An i2c connection to a BME280 Sensor
* An i2c connection to an OLED display
* 2 x Regulators to bring the 9v input down to: -
** 5v for the Temp Sensor and display
** 3.3v for the ESP-01

# What do I need to build this?

# What did I learn / what would i do differently?

## Regulator Design

The ESP-01 is quite power hungry and so the LD1117V33 is having to disapate over 1W of power. This means it gets quite hot.
More than 50degC above ambient. I had to use a heat sink as it was too hot to touch. If I did this again I would put the 5v and 3.3v
Regulator in series. That way the heat would shared between them.

## Choice of board

The ESP-01 actually contains a 32bit ARM CPU that is far more powerful than the 8bit AT Mega CPU on the Arduino.
You can get Nano style development boards with i2c and GPIO pins (https://www.amazon.co.uk/AZDelivery-ESP-32-Dev-including-eBook/dp/B07Z83MF5W). It would be much easier to build the board around one of these.

