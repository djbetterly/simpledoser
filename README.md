# Simple Doser
Web Controlled Peristaltic Doser for Aquariums

Powered by Particle Photon

Controlled through http://www.simpledoser.com - Free for users of this project

Firmware supports up to 6 Pumps with scheduling for 12 and 24hr periods, calibration and manual functions.
Calibration automatically runs for 60 seconds for each pump.
##CLI Commands
```bash
  Variables:
    getRelay1 (string)
    getRelay2 (string)
    getRelay3 (string)
    getRelay4 (string)
    getRelay5 (string)
    getRelay6 (string)
    relayON (string)
    relayOFF (string)
  Functions:
    int calibrate(String args) 
    int setupChannel(String args) 
    int setDoseRate(String args) 
    int manual(String args) 
    int setTimeZone(String args)
```
Variable Example
```bash
particle get <photonName> getRelay1
```
Manual Function - args = pump#, on, off
```bash
particle call <photonName> manual 1,on
```
Calibration Function - args = pump#
```bash
particle call <photonName> calibrate 
```
SetupChannel - args = channel#,startTime,dosingPeriod,numberOfDoses,dose

  -startTime is time of day in seconds (Ex: 14:30:00 = 52200)
  
  -dosingPeriod determines if the program runs over a 12hr or 24hr period
  
  -numberOfDoses is how many doses will take place over the dosingPeriod
  
  -dose is the total amount that will be dosed during the dosingPeriod
  
The example below would tell pump 1 to start dosing 200mL at 14:30:00, 12 times over a 12hr period.
```bash
particle call <photonName> setupChannel 1,52200,12,12,200
```

