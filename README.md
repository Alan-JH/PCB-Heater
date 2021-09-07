# PCB-Heater

Solder reflow hotplate design based on Carl Bugeja's idea and proof of concept.

Aluminum PCB as the resistive heating element, 200x200mm. Control board is separated and has displays for time and temperature (target and actual), and an SD card to read reflow profiles and save data to. Runs off of 3.3V logic and the ATTiny1614 MCU. Input voltage (for the heating element) can range up to around 40V by design (Plan to use 24-30V), and the element itself should have a resistance of 8.1Ohm at 25C. Target maximum temperature is around 200C.
