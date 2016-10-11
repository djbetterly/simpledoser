# simpledoser
Web Controlled Peristaltic Doser for Aquariums

Powered by Particle Photon

Controlled through http://www.simpledoser.com - Free for users of this project

Firmware supports up to 6 Pumps with scheduling for 12 and 24hr periods, calibration and manual functions.

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
