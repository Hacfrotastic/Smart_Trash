// Include Particle Device OS APIs
//This is for a Particle board. To use, copy into the Particle's web IDE or create a project file via VScode Particle workbench
//and put copy code to there

#include "Particle.h"

char buf[50];
int aReading;
float cmDis;
int oldStatus;
int status; 
int nowDis; //distance for avg
int lat;
int lon;
int stateOfCharge;      //for fuel gauge battery monitoring 
int batteryVoltage;     //^^
int report;
FuelGauge fuel;

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);
SystemSleepConfiguration config;
// setup() runs once, when the device is first turned on
void setup() 
{
    pinMode(A0, INPUT);
    config.mode(SystemSleepMode::ULTRA_LOW_POWER).duration(28800000); 
    Particle.function("Debug",debug);
    Particle.function("Bedtime",bedtime);
    Particle.function("Battery_Rep",Battery_Rep );
    //Particle.function("SetLat",SetLat);
    //Particle.function("Good", Good);
    //Particle.function("SetLon",SetLon);
    
  // Put initialization like pinMode and begin functions here
 
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    cmDis = 0;
 for(int ii = 0; ii<10; ii++)
  {
    aReading = analogRead(A0);
    float volts = (aReading*3.3)/4095.0; // convert 
    cmDis += (volts/3.2);                //Converting Volts to millamerter
    delay(2000);
  }
                  // data sheet stuff
  //Serial.println(cmDis);
  cmDis = cmDis/10;
  if(cmDis>60)
  {
    //status = 1;
    status = 100 -((cmDis*100)/70);
    //Serial.println("empty");
  }
  else if(cmDis>30)
  {
      //status = 2;
      status = 100 -((cmDis*100)/70);
      //Serial.println("middle");
  }
  else
  {
      //status = 3;
      status = 100 -((cmDis*100)/70);
     // Serial.println("yikes");
  }
  
  
  if(status != oldStatus)
  {
    
    snprintf(buf, sizeof(buf), "%d Raw: %d", status, cmDis);   //convert/save the data
    Particle.publish("nrStatus", buf, PRIVATE);   //publish
   // Serial.println(buf);
    
    //System.sleep(SLEEP_MODE_DEEP, 60);
    delay(60000);                               //update long time
    // Particle.publish("dmessage", "The status has changed")
    //delay(10000000);
  }
  else
  {
      delay(1000);
  }
  oldStatus = status;
  //delay(1000);
    

    //snprintf(buf, sizeof(buf), "%d", cmDis);  //convert the data
    //Particle.publish("status", buf, PRIVATE); //publish

}

int debug(String cmd)
{
    cmDis=0;
    for(int ii = 0; ii<10; ii++)
  {
    aReading = analogRead(A0);
    float volts = aReading*3300.0/4095.0; // convert 
    cmDis += volts/3.2; 
    delay(2000);
  }
  cmDis = cmDis/10;
  return cmDis;
}

int bedtime(String cmd)
{
    System.sleep(config);
    delay(60000);
    return 1;
}

int Battery_Rep(String cmd)                             //I do not think this is a good way to do this btw 
{
    stateOfCharge = (fuel.getNormalizedSoC() * 1000);   //I believe these return floats
    batteryVoltage = (fuel.getVCell());                 //which is important cause we can only return int 
    return stateOfCharge + batteryVoltage;              //but since are variables are ints it should be okay you just loose some precision
}

//Relics of a vision I did not finish
/*
int SetLat(String cmd)
{
    lat = cmd.toInt();
    return 1;
}

int SetLon(String cmd)
{
    lon = cmd.toInt();
    return 1;
}

int Good(String cmd)
{
    return 1;
}
*/
