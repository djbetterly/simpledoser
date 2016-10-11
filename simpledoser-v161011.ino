//October 11,2016
//Added Timezone Support

typedef struct {
    int dosingPeriod; // pass either 24 or 12 for this value
    int startTime;
    int endTime;
    int onTime;
    int offTime;
    float totalDose;
    float doseRate; // mL/min
    int numberOfDoses;
    int connectedPin;
    int calibrationEnd = 90000; // a value greater than "now" can ever be
} Channel;

 void updateChannel(Channel c);
 void updateParticleVariables(int index, Channel* c); 
 
 float now;
 retained char getRelay1[150];
 retained char getRelay2[150];
 retained char getRelay3[150];
 retained char getRelay4[150];
 retained char getRelay5[150];
 retained char getRelay6[150];
 retained char relayONstr[100];
 retained char relayOFFstr[100];
 retained float timeZone;
 retained int timeZoneStatus;

retained Channel channels[8];

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));
SYSTEM_THREAD(ENABLED);


void setup() {
    
    if(timeZoneStatus == 1)
    {
        Time.zone(timeZone); 
    }
    else
    {
        Time.zone(-4); // Eastern Daylight Savings Time
    }
    
    Particle.function("calibrate", calibrate);
    Particle.function("setupChannel", setupChannel);
    Particle.function("setDoseRate", setDoseRate);
    Particle.function("manual",manual);
    Particle.function("setTimeZone", setTimeZone);
    
    Particle.variable("getRelay1", getRelay1);
    Particle.variable("getRelay2", getRelay2);
    Particle.variable("getRelay3", getRelay3);
    Particle.variable("getRelay4", getRelay4);
    Particle.variable("getRelay5", getRelay5);
    Particle.variable("getRelay6", getRelay6);
    Particle.variable("relayON", relayONstr, STRING);
    Particle.variable("relayOFF", relayOFFstr, STRING);
    
    channels[1].connectedPin = D0;
    channels[2].connectedPin = D1;
    channels[3].connectedPin = D2;
    channels[4].connectedPin = D3;
    channels[5].connectedPin = D4;
    channels[6].connectedPin = D5;
    
    pinMode(D0,OUTPUT);
    pinMode(D1,OUTPUT);
    pinMode(D2,OUTPUT);
    pinMode(D3,OUTPUT);
    pinMode(D4,OUTPUT);
    pinMode(D5,OUTPUT);
    
    Serial.begin(9600);
    delay(3000);
    now = Time.hour() * 3600 + Time.minute() * 60; // + Time.second();
    Serial.println(now);
    
    Serial.printlnf("getRelay1: %p   getRelay2: %p   getRelay3: %p   getRelay4: %p channels: %p", getRelay1, getRelay2, getRelay3, getRelay4, channels);
}



void loop() {
     now = Time.hour() * 3600 + Time.minute() * 60 + Time.second();
     for (int i=1;i<8;i++) {
         updateChannel(i);
     }
     if (Particle.connected() == false) {
    Particle.connect();
     }
}



void updateChannel(int index) {
    Channel *c = &channels[index];
    char publishString[200];
    if (c->onTime == now && c->totalDose > 0) { 
        digitalWrite(c->connectedPin, HIGH);
        sprintf(publishString, "timely_1_%d", index);
        Particle.publish ("simpleDoserLogs", publishString);
        Serial.printlnf("Fired Relay %d", index);
        if (c->dosingPeriod == 24) {
            int proposedOnTime = (c->onTime + 86400/c->numberOfDoses) % 86400;
            c->onTime = (abs(proposedOnTime - c->startTime) < 20)? c->startTime : proposedOnTime; // correction to keep times from creeping because of rounding caused by doing integer division
        }else{
            int proposedOnTime = (c->onTime + 43200/(c->numberOfDoses - 1)) % 86400;
            c->onTime = (abs((proposedOnTime + 43200 - 43200/(c->numberOfDoses - 1)) % 86400 - c->startTime) < 60)? c->startTime : proposedOnTime;  
        }
        
        Serial.printlnf("onTime is: %d", c->onTime);
        updateParticleVariables(index);
    }

    if (c->offTime == now && c->totalDose > 0) {
        digitalWrite(c->connectedPin, LOW);
        sprintf(publishString, "timely_0_%d", index);
        Particle.publish ("simpleDoserLogs", publishString);
        Serial.printlnf("Turned off Relay %d", index);
        if (c->dosingPeriod == 24) {
            int proposedOffTime = (c->offTime + 86400/c->numberOfDoses) % 86400;
            c->offTime = (abs(proposedOffTime - c->endTime) < 60)? c->endTime : proposedOffTime;
        }else{
            int proposedOffTime = (c->offTime + 43200/(c->numberOfDoses - 1)) % 86400;
            c->offTime = (abs((proposedOffTime + 43200 - 43200/(c->numberOfDoses - 1)) % 86400 - c->endTime) < 60)? c->endTime : proposedOffTime;
        }
        Serial.printlnf("offTime is: %d", c->offTime);
        updateParticleVariables(index);
    }
    
    if (c->calibrationEnd <= now) {
        digitalWrite(c->connectedPin, LOW);
        sprintf(publishString, "timely_0_%d", index);
        Particle.publish ("simpleDoserLogs", publishString);
        c->calibrationEnd = 90000;
    }
}


int setupChannel(String cmd) { // cmd syntax: "channel#,startTime,dosingPeriod,numberOfDoses,dose" (startTime is sent as hour*3600 + minute*60 + second)
    
    char *stringArgs = (char*)cmd.c_str();
    int index = atoi(strtok(stringArgs, ","));
    Channel* c = &channels[index];
    c->startTime = atoi(strtok(NULL, ","));
    if (c->startTime >= 86400) {
        return -1; // bad start time, so abort
    }
    c->onTime = c->startTime;
    c->dosingPeriod = atoi(strtok(NULL, ","));
    c->numberOfDoses = atoi(strtok(NULL, ","));
    c->totalDose = atof(strtok(NULL, ","));
    float dosePerActuation = c->totalDose/c->numberOfDoses;
    float secondsPerActuation = (dosePerActuation/c->doseRate) * 60; // assuming doseRate is in volume/minute
    c->endTime = (c->onTime + (int)secondsPerActuation) % 86400;
    c->offTime = c->endTime;
    Serial.printlnf("channel#: %d, onTime: %d, dosingPeriod: %d, doseRate: %f, numberOfDoses: %d, totalDose: %f, pump run time: %f seconds\n\n", index, c->startTime, c->dosingPeriod, c->doseRate, c->numberOfDoses, c->totalDose, secondsPerActuation);
    Serial.println();
    Serial.printlnf("Pump %d starts at %s", index, convertTime(c->startTime));
    Serial.printlnf("The next of the %d doses per day (of %f mL each) will occur at %s", c->numberOfDoses, dosePerActuation, convertTime((c->startTime + 86400/c->numberOfDoses) % 86400));
    Serial.printlnf("This pump will run for %d seconds for each dose", c->endTime - c->startTime);
    
    updateParticleVariables(index);
    return 1;
}

char* convertTime(int secs) {
    int hour = secs/3600;
    int minutes = (secs % 3600)/60;
    int seconds = (secs % 3600) % 60;
    static char result[10];
    sprintf(result, "%d:%02d:%02d", hour, minutes, seconds);
    return result;
}

void updateParticleVariables(int index) {
    Channel* c = &channels[index];
    char* str;
    switch (index) {
        case 1:
            str = getRelay1;
            break;
        case 2:
            str = getRelay2;
            break;
        case 3:
            str = getRelay3;
            break;
        case 4:
            str = getRelay4;
            break;
        case 5:
            str = getRelay5;
            break;
        case 6:
            str = getRelay6;
            break;
    }
    sprintf(str, "channel#: %d, onTime: %s, doseRate: %.1f, dosingPeriod: %d, numberOfDoses: %d, totalDose: %.1f", index, convertTime(c->startTime), c->doseRate, c->dosingPeriod, c->numberOfDoses, c->totalDose);
}




int calibrate(String cmd) {
    int index = atoi(cmd);
    Channel* c = &channels[index];
    digitalWrite(c->connectedPin,HIGH);
    c->calibrationEnd = Time.hour() * 3600 + Time.minute() * 60 + Time.second() + 60;
    return 1;
}

int setDoseRate(String cmd) {
    char *rmdr;
    char *stringArgs = (char*)cmd.c_str();
    char publishString[200];
    int index = atoi(strtok_r(stringArgs, ",", &rmdr));
    Channel* c = &channels[index];
    c->doseRate = atof(rmdr);
    sprintf(publishString, "channel#: %d, doseRate: %.1f", index, c->doseRate);
    Particle.publish ("setDoseRate", publishString);
    updateParticleVariables(index);
    return 1;
}


int manual(String cmd) {
    char *rmdr;
    char *stringArgs = (char*)cmd.c_str();
    int index = atoi(strtok_r(stringArgs, ",", &rmdr));
    Channel* c = &channels[index];
    char publishString[200];
    if (strcmp(rmdr, "on") == 0) {
        digitalWrite(c->connectedPin,HIGH);
        sprintf(publishString, "manual_1_%d", index);
        Particle.publish ("simpleDoserLogs", publishString);
        return 1;
    }
    else if (strcmp(rmdr, "off") == 0) {
        digitalWrite(c->connectedPin,LOW);
        sprintf(publishString, "manual_0_%d", index);
        Particle.publish ("simpleDoserLogs", publishString);
        return 0;
    }
    else {
        return -1;
    }
}

int setTimeZone(String timeZoneValue) {
    // Read the timezone
    char *rmdr;
    char *stringArgs = (char*)timeZoneValue.c_str();
    float index = atof(strtok_r(stringArgs, ",", &rmdr));
    char publishString[200];
    sprintf(publishString, "Time Zone: %d:%d", Time.hour(), Time.minute());
    Particle.publish ("logs", publishString);
    // Assign the timezone to retained variable
    timeZone = index;
    Time.zone(timeZone);
    timeZoneStatus = 1;
    return 1;
}