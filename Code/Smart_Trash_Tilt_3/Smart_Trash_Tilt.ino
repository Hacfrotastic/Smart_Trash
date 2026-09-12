// This #include statement was automatically added by the Particle IDE

// Include Particle Device OS APIs
#include "Particle.h"
char buf[255];
int aReading;
int status; 
int nowDis;             //distance for avg
int stateOfCharge;      //for fuel gauge battery monitoring 
int batteryVoltage;     //^^
int report;
FuelGauge fuel;         
int readingPin = A0;    //Analog pin for reading distance
int powerPin = D13;     // pin for powering sensor (poor mans MosFet)
int tilt = A1;          //For tracking when the bag gets changed
bool isAwake;           //reports if the sensor is awake (currently it wakes up and stays after the noon report)
int numSamp = 25;       //Number of times the sensor will raje a reading before averaging
float cmDis = 0;        //
String myDev;
//Required constructor to remember device name.

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);
SystemSleepConfiguration config;

int EEPUT(String x)             // Functions for storing the name of the device in EEPROM
{
    //Get rid of spaces in the string
    for(int ii = 0; ii<x.length(); ii++)
    {
        if(x[ii]== ' ')
        {
            x[ii] = '_';
        }
    }
    //put the string in EEPROM
    EEPROM.put(1, x.length());
    for(int ii = 0; ii < x.length(); ii++)
    {
        EEPROM.put((ii + 2),x[ii]);
    }
    System.reset();
    return 1;
}

char* EEGET()                //Gets Device name from EEPROM
{
    if(EEPROM.read(1) != 0xFFFF) //if the memory is not empty, read it
    {
        
        int bytes2Read = EEPROM.read(1);
        static char dev_Name[100] = " "; 
        for(int ii = 0; ii<bytes2Read; ii++)
        {
         dev_Name[ii] = EEPROM.read((ii + 2));
        }
        return dev_Name; 
    }
    
    return "ERROR";
}

void SetBagFlag(bool state)
{
    if (state == 1)
    {
        EEPROM.put(201, "1");    //Flag on
    }
    else if(state == 0)
    {
        EEPROM.put(201,"0");     //Flag off
    }
    delay(1000);                 //Delay because I do not trust things 
    System.reset();              //Reset it for EEPROM to register
}

byte GetBoolFlag()
{
    byte state = EEPROM.read(201);
    return state;
}
// setup() runs once, when the device is first turned on
void setup() 
{
    Time.zone(-4);
    myDev = EEGET(); 
    while(!Particle.connected() || myDev == "ERROR") //Wait for board to connect or recieve name.
    {
        //wait for boron board to connect to cell
    }
    pinMode(readingPin, INPUT);
    pinMode(powerPin, OUTPUT);
    Particle.variable("Sensor Awake", isAwake);
    Particle.function("Measure Percent Full",debug);
    config.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(60min); 
    Particle.function("Bedtime",bedtime);
    Particle.function("Set Name", EEPUT);
    Serial.begin(9600);
    
    //Setup required to retain name.
}

// loop() runs over and over again, as quickly as it can execute.
void loop() 
{
 /*
 Problem: The sensor sometimes reported 0 as a measurement when there were items in the trash can
 
 Fix:     I added a for loop that loops 2 times if and only if the reading reported is 0, this way
          the ultrasonic will double check all reports of 0s but can still publish 0 if the can is 
          actually empty 
          Also upped the amount of samples taken (10 -> 50)
 */
    
    //give a five minute delay so the sensor can retrieve its name 
    
 
  digitalWrite(powerPin, HIGH);  //turn on sensor through GPIO pin so it is not drainging the battery
  delay(30000);
  float avg;
  for(int ii=0; ii<numSamp ; ii++)
  {
     int aReading = analogRead(readingPin);
     float volts = (aReading*3.3)/4096;
     float cmDis = volts/0.0032;
     avg += cmDis; 
     Serial.print("Cm: ");
     Serial.print(cmDis);
     Serial.print("Avg: ");
     Serial.println(avg);
     delay(1000);
     
  }
  avg = avg/numSamp;
  int status = 100 - (((avg-15)/70)*100);  
  //filter status
  if (status > 100)
  {
      status = 100;
  }
  else if (status < 0)
  {
      status = 0;
  }
  
  Serial.println(status);
  //set the bag ch
  //Bag check code
  char bag = GetBoolFlag();                 //grab bag state from EEPROM
  if(bag =='1')                             //The tilt ball has fired and sensed that the trash has been changed
  {
      if(status >= 75)                      //It was just changed and got a full reading, this must be bag
      {
        status = 0;
      }
      else                                  //we are not seeing bag so lets reset the bag flag and not adjust the reading
      {
        SetBagFlag(0);                      //set the bag flag to 0 in EEPROM
      }
      
  }
    
    //Use for discord integration
    float voltage = fuel.getVCell();
    stateOfCharge = fuel.getNormalizedSoC();
    char* myDev = EEGET();
    //Under construction  DeviceNameHelperRetained::instance().getName()
    snprintf(buf, sizeof(buf), "{\"DeviceName\":\"%s\", \"PoC\": %d, \"BatteryVolts\": %.2f,\"TrashPercent\": %d, \"Bag\": %d}", myDev, stateOfCharge, voltage, status, bag);   //convert/save the data
    delay(100);
    Particle.publish("TashStop", buf, PRIVATE);          //publish battery data
   // digitalWrite(powerPin, LOW);
    delay(10000);
    Particle.publishVitals();
    delay(10000);
   // Serial.println(buf);
   SystemSleepResult result;
   
   if((Time.hour() <= 2) && (Time.hour() >= 0))  //  so this is midnight to 2 am have it sleep till morning
    {
        config.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(6h).analog(tilt, 2000, AnalogInterruptMode::ABOVE);
        digitalWrite(powerPin, LOW);
        result = System.sleep(config);
    }
    else if(Time.hour() == 1)
    {
        isAwake = true;
        digitalWrite(powerPin, LOW);
        delay(3600000); //delay an hour and then go again
        //Stay awake for updates
        isAwake = false;
    }
    else
    {
        config.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(60min).analog(tilt, 2000, AnalogInterruptMode::ABOVE);
        digitalWrite(powerPin, LOW);
        result = System.sleep(config);
    }
    //The tilt ball switch went off AKA trash got changed
    if (result.wakeupReason() == SystemSleepWakeupReason::BY_LPCOMP) 
    {
        SetBagFlag(1); //log that we may be seeing bag now
    }
  // Waken by pin 
    
    //delay(120000000);                               //update 2 minutes
    
    // Particle.publish("dmessage", "The status has changed")
}

int debug(String cmd)
{ 
    cmDis = 0;
    for(int ii = 0; ii<10; ii++)
  {
    aReading = analogRead(A0);
    float volts = aReading*3300.0/4095.0; // convert 
    cmDis += volts/3.2; 
    delay(1000);
  }
  cmDis = cmDis/10; //avg reading 
  //cmDis = 100 -((cmDis*100)/70);// make into percent
  return cmDis;
}

int bedtime(String cmd)
{
    System.sleep(config);
    //delay(6000000);
    return 1;
}

////////////////////////////
int Battery_Rep(String cmd)
{
   // stateOfCharge = (fuel.getNormalizedSoC() * 1000);
    batteryVoltage = (fuel.getVCell()*1000);
    return stateOfCharge + batteryVoltage;
}

