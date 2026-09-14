# SPADCounter

A simple pulse counter, based on a binary counter for APDs like PerkinElmer/Excelitas single photon avalanche photodiodes. 
It can read the pulse train coming from the diode and communicate wiht a PC, granting a simple interface to track the photon counts.
It also controls the power supply to the diode (a simpel pass-through with some transistors), to avoid diode damage due to overexposure.
It is programmed to trigger at 1 Mcps, but the threshold is programmable. 
Everything can be controlled via a simple serial interface (diode on/off commands, getting counts and setting the threshold value).