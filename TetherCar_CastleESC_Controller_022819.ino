//This software is intended to be used in an electric tether car with an electrical safety shutoff switch.
//
//This software is intended to be used with a Castle Phoenix Edge HV 120 ESC with no BEC programmed for fixed end pointsin Airplane mode.
//According to Thomas from Castle, the high/low PWM is 1.3 to 1.8 milli seconds respectively with with a resolution of 256 steps in between.
//1800 - 1300 micro seconds = 500 with 256 steps, the increments need to be a minimum of 1.95.
//However, for ease of programming, we just increment by 1 microsecond from 1300 to 1800 in the ramp up duration, thus making every 2nd increment actually being recognized by the ESC.
//The fixed end points must be programmed into the ESC with their field programmer or computer based programmer.
//
//Matt Geders 02/28/19.  mgeders@gmail.com
//Not responsible if you blow something up.

#include <Servo.h>                                // include an Adruino provided library that knows how to do servo stuff
 
Servo esc;                                        // define a Server (using the Servo library) and call it "esc"
int LowIdle = 1000;		                            // The lowest "throttle" (PWM) for the ESC. (should produce zero RPM)
int RXLED = 17;                                   // The RX LED has a defined Arduino pin; this is the dim yellow blinky LED on the actual Arduino board.
int StartDelay = 10;                              // The number of seconds it will delay until starting the motor (not including the Arduino boot time)  Do not go less than 5 seconds.
int RampUpDuration = 30;                          // The number of seconds the ramp up will occur.  For reasons of Arduino limitations, recomment don't go lower than 7 seconds.
int MinSpeed = 1300;                              // The minimum PWM signal.
int MaxSpeed = 1800;                              // The maximum PWM signal.
int MaxMinDiff = 500;                             // Difference between Max and Min (1800-1300=500)
const int RampStepDelay = round(((RampUpDuration*1000)/(MaxSpeed-MinSpeed)));
int i = 0;

const int switchPin = 2;                          // the number of the SPST safety switch pin
int buttonState = 0;                              // variable for reading the safety switch status
 
void setup()                                      // This only runs once upon boot up of the Arduino itself.  Think if it as the "setup" commands.  We only use this because we have a 1 time run of the program.
{
  esc.attach(3);                                  // Set the servo info to be sent out pin 3 of the Arduino itself.
  pinMode(RXLED, OUTPUT);                         // Set RX LED as an output (TX LED is set as an output behind the scenes)
  pinMode(switchPin, INPUT);                      // Set the SPST safety switch pin and set it as an input
  TXLED0;                                         // TX LED is not tied to a normally controlled pin so a macro is needed, turn LED OFF
  digitalWrite(RXLED, HIGH);                      // set the RX LED OFF                          
    
    do						                                // This loop is to send a LowIdle signal so the ESC recognizes its connected and arms, but doesn't actually move. It is intent is to allow it to retry sending the minimum signal in case it finishes before the ESC boots
    {
      esc.writeMicroseconds(LowIdle);
      buttonState = digitalRead(switchPin);       // Checks the state of the SPST safety switch
      TXLED1;                                     // TX LED macro to turn the yellow blinky LED ON
      delay(20); 				                          // wait time in (milli-seconds)
    } while (buttonState == LOW);                 // keep looping inside this "do" statement as long as the SPST safety switch is open.  As soon as the SPST safety switch is closed...read what happens next...

    digitalWrite(RXLED, LOW);                     // set the RX LED ON as an indicator that the system is beginning the StartDelay sequence
        
    for(i = 0; i < StartDelay; i++)               // This is the start delay loop.  It's purpose is to allow people to get situated/ready for the run
    {
      esc.writeMicroseconds(LowIdle);                         // Send the base signal that lets the ESC know its connected. This hasn't changed
      buttonState = digitalRead(switchPin);       // Checks the state of the SPST safety switch
      if (buttonState == LOW){goto Shutdown;}     // If the switch is flicked back (open) then go directly to the "Shutdown" portion of the code
      digitalWrite(RXLED, HIGH);                  // set the RX LED OFF
      delay(500);                                 // wait .5 second (500 milli seconds) so the ESC has time to startup and so the LED blinks brighter 
      digitalWrite(RXLED, LOW);                   // set the RX LED ON
	    delay(500);				                          // wait .5 seconds (500 milli seconds) so the ESC has time to startup and so the LED blinks brighter
    }

esc.writeMicroseconds(MinSpeed);                  // Start the motor at the minimum speed
buttonState = digitalRead(switchPin);             // Checks the state of the SPST safety switch
if (buttonState == LOW){goto Shutdown;}           // If the switch is flicked back (open) then goto directly to the "Shutdown" portion of the code

    for(i = 0; i <(MaxMinDiff); i++)              // This sequence ramps the motor to max speed over a defined duration. "i" is just counter that starts at 0 and goes to MaxSpeed-Minspeed)  "i++" just increases "i" by one each time this loops
    {
	    esc.writeMicroseconds(MinSpeed++);          // Takes the MinSpeed and adds 1 to it and stores that new value as the new MinSpeed
      buttonState = digitalRead(switchPin);       // Checks the state of the SPST safety switch
      if (buttonState == LOW){goto Shutdown;}     // If the switch is flicked back (open) then goto directly to the "Shutdown" portion of the code
      digitalWrite(RXLED, LOW);                   // set the RX LED ON
      delay(RampStepDelay);                       // Self explanatory.  The low light yellow blink speed will effectively vary based on this line.
      digitalWrite(RXLED, HIGH);                  // set the RX LED OFF
    }

    do
    {
	    esc.writeMicroseconds(MaxSpeed);            // Run the motor at the defined maximum speed
      buttonState = digitalRead(switchPin);       // Checks the state of the SPST safety switch
      if (buttonState == LOW){goto Shutdown;}     // If the switch is flicked back (open) then goto directly to the "Shutdown" portion of the code
      digitalWrite(RXLED, LOW);                   // set the RX LED ON
      delay(500);                                 // The delays here add up to 1 second but allow the LED's to blink more brightly in the MaxSpeed cycle.
      digitalWrite(RXLED, HIGH);                  // set the RX LED OFF
	    delay(500);
    } while (buttonState == HIGH);

Shutdown:					                                // This is the Shutdown label of the code.  If the SPST safety switch is tripped (aft), the program exits and goes here.
  TXLED0;                                         // TX LED is not tied to a normally controlled pin so a macro is needed, turn LED OFF
  digitalWrite(RXLED, HIGH);                      // set the RX LED OFF  
  esc.writeMicroseconds(LowIdle);                 // Turn the motor off
}
 
void loop()                                       // This part runs continuosly forever and ever.  (and is empty for our purposes)
{

}
