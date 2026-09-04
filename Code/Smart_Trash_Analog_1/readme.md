# Smart_Trash Analog 

This is where it all started (kinda), so let's run through how it works.

## Setup()

Here we do 3 things. 
- set up our variable
- Establish some particle functions (see more in function section)
- configure sleep settings (config.mode)

## Loop()

Meat and potatoes time!!!

This is your typical analog readings code. 
- Get 10 readings from the ultrasonic sensor
    + We are working with 3.3V logic
    + The Borons ADC is 4096 bits
    + This Maxbotic sensors conversion formula is  3.2 cm per mV
    My code looks a little wrong to me, I believe in the code I'm using just volts in the formula (Needs Testing)
- Average readings 
- Calculate status based on reading
    + This started out as a discrete status, some may even say it was quantized. 1=Empty 2=halfway 3=full that is what you see on the first line of each if statement commented out.
    + Eventually we moved status to be a percent calculation with the formula in the second line. The formula takes the reading and divides it by the known max of the trashcan (70cm) while multiplying that number by 100 so it mimics a percent. Lastly subracting that number from 100 since the distance reading is the inverse of how full the trash can is. Cool right. 
- If status changes publish the status
    + The snprintf statement make the string; Particle.publish then sends it.

## Functions 
Lets go ahead and describe what each function does

### Debug 

 Expected input : Leave blank, input does not matter
 This runs the basic reading cycle and returns what ever status was reported
 

 ### Bedtime

 Expected input : Leave blank, input does not matter
 Forces the sensor to go to bed. The sleep time is hardcoded and set by the config.mode statement in setup()

 ### Battery_Rep
 
 Expected input : Leave blank, input does not matter
 This ones a little weird and a relic of me not knowiung how to do some things, but I'll leve it how it is so the reader can judge me. 
 First the board gets the percent of battery charge (ex. 100 -> 100% -> full charge). Then it reads the batteries voltage. Both of these readings are read using the Particle Borons built in "FuelGauge" functions that I have instatiated at the top of the code file. 
 To return both of the numbers in the same function, SOMONE had the "brilliant idea" of shifting the battery percent up by 1000 and adding the voltage to that. Example, battery percent = 83 Voltage = 3.7 makes the reported value 83003.7. Weird but it kinda works... 
 However,
  A) having two seperate published just seems better 
  B) THE INT CUTS THE DECIMAL OFF!!! I mean what the point of that kind of reading anyway. 3.7 drops to 3, so when you have full charge ~3.9V on my battery the function reads 4 and when you get low, hell even DEAD, ~3.5 it drops it to, you'll never guess 3!!!!!
  (Or new me is wrong and old me is better at coding, which is possible. Test it and find out)

  ### Function Graveyard

  There are some commented out functions I put in there with hipes of expanding the code to store GPS cords. ,but I never around to it. I'll leave them in there though to inspire.