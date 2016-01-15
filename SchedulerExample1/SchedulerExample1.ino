

/*
 * Version 0.1
 * Author: Andre (JJA) Smit
 * Design: Andre Smit & Albert (HA) Smit 
 * Date:   2016/01/15
 * 
 * This software was written as an example and for my own use. As such you choose
 * to use it at your own risk. The liability limit of this software is equal to 
 * what you paid for it. In other words nothing.
 * 
 * This is free software;There is NO warranty; not even for MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * I hereby make SchedulerExample1 available under the GNU GENERAL PUBLIC LICENSE Version 2, June 1991
 * 
 * This is an example of a scheduler with a fixed preconfigured priority per task.
 * Please refer to README.md file published with this file for more detail.
 * It is assumed you know how to wire an LED and resistor to Arduino, etc
 * This is not a beginners tutorial and neither a "conventional" Arduino program.
 * 
 * I am testing this example on an Arduino UNO R3, and using the same scheduler in
 * installed systems on an Arduino Mega 2560
 * 
 * Lastly the Arduino platform this was written for is an awesome tool. 
 * Many thanks to every AVR GCC & Arduino contributor.
 * 
 * Kind regards
 * Andre Smit
 * 
 */

//Some "Constants" used by Scheduler
#define PRIORITY 0
#define AGE      1
#define WAKETIME 2
#define FUNCPTR  4

//LEDs wire to digital output 12 & 13 with 1k or 2k2 resistor to ground
#define LED0 13
#define LED1 12

//Push button wired to Digital Input 3 and Ground
#define BUTTON0 3

typedef void (*funcptr_t)();


//String variables for counters
char cnt1Str[7] = "c1";
char cnt2Str[7] = "c2";
char cnt3Str[7] = "c3";

//Integer array to store analog values
int analogValue[6] = {0,0,0,0,0,0};

uint32_t buttonPressedTime = 0;

uint8_t ledState = 0;



void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);

  pinMode(BUTTON0, INPUT);
  digitalWrite(BUTTON0, HIGH); //Enabling internal pullup wire button to Pin and Ground(GND)
  
  scheduler();
}

void loop() {

  /*
   * The scheduler function called from setup() above has an infinite loop
   * Execution will therefore never reach this loop, which is fine as the scheduler
   * will call our function (task)
   */
}

//=================================================== Scheduler Task List =====================================================

// PRIORITY, AGE, WAKETIME[2], TASKPTR
uint16_t tasks[][5] = {
  {64 ,  0, 0, 0, (uint16_t) &task_CheckButtonPress},
  {8  ,  0, 0, 0, (uint16_t) &task_ReadAnalog},
  {1  ,  0, 0, 0, (uint16_t) &task_BlinkLED},
  {1  ,  0, 0, 0, (uint16_t) &task_ButtonPressedLED},
  {1  ,  0, 0, 0, (uint16_t) &task_TestCounter1},
  {4  ,  0, 0, 0, (uint16_t) &task_TestCounter2},
  {1  ,  0, 0, 0, (uint16_t) &task_TestCounter3},
  {1  ,  0, 0, 0, (uint16_t) &task_PrintValues},
  {0,    0, 0, 0, 0 }
};

/*
ALSO NOTE: Some tasks are called from 1Sec interrupt method

In addition to these tasks there is an interrupt routine running on Timer5 
This routine is used to measure ODOs et al in a clocked fasion.

*/


//=================================================== Scheduler Code Begins =====================================================

void scheduler()
{
  static uint8_t callIndx = 0;
  static funcptr_t toCall = 0;
    
  for(;;)
  {
    /*
       Inactive Ageing SCHEDULER....
    */
    int maxAge = 0;
    for (uint8_t indx = 0; tasks[indx][PRIORITY] != 0; indx++)
    {
      tasks[indx][AGE] += tasks[indx][PRIORITY];
      if (tasks[indx][AGE] > maxAge)
      {
        maxAge = tasks[indx][AGE];
        callIndx = indx;
      }
    }
    tasks[callIndx][AGE] = 0;

    //Skip task until WAKETIME reached
    uint32_t *x = (uint32_t*) &tasks[callIndx][WAKETIME];
    if ( millis() > *x )
    {
      toCall = (funcptr_t) tasks[callIndx][FUNCPTR];
      toCall();
    }

    millsRollover();
  }
}

inline void millsRollover()
{
     //Pause and recover tasks after millis() overflow
    if ( (millis() > 4294967200L) )
    {
      while (millis() > 128L) //waiting for it to become less than 128 / rollover
      {
        delayMicroseconds(8);
      }
      for (uint8_t indx = 0; tasks[indx][PRIORITY] != 0; indx++)
      {
        tasks[indx][WAKETIME] = 0;
        tasks[indx][WAKETIME + 1] = 0;
      }
    }
}

void setWakeTime(uint16_t task, uint32_t wakeTime)
{
  uint32_t *x;
  for (uint8_t indx = 0; tasks[indx][PRIORITY] != 0; indx++)
  {
    if ( task == tasks[indx][FUNCPTR] )
    {
      x = (uint32_t*) &tasks[indx][WAKETIME];
      *x = millis() + wakeTime;
    }
  }
}

//============================================== Scheduler Code Ends Task code Follows ==========================================

//Store time button was pressed
void task_CheckButtonPress()
{
  if( digitalRead(BUTTON0) == 0)
  {
    buttonPressedTime = millis();
    Serial.println("Button Pressed!");
    setWakeTime((uint16_t)&task_CheckButtonPress, 100); //Task is sleeping / will not be called for 100 miliseconds
  }
}

/*
 * ===============================================================================================================================
If button pressed less than 2000ms ago, switch on LED else switch off LED
It is assumed the button is connected to BUTTON0 input and connected to ground
It is assumed you have a led connected to DIGITAL line LED1
*/
void task_ButtonPressedLED()
{

  if( buttonPressedTime != 0 )
  {
    if( ( buttonPressedTime + 2000 ) > millis() )
    {
      digitalWrite(LED1, HIGH);
    }
    else
    {
      digitalWrite(LED1, LOW);
      buttonPressedTime = 0;    
    }
  }
  setWakeTime((uint16_t)&task_ButtonPressedLED, 100); //Task is sleeping / will not be called for 100 miliseconds
}


/*
 * ===============================================================================================================================
 Read Analog Values, average with previous value, store in array
 */
void task_ReadAnalog()
{
  for(int channel = 0; channel < 6; channel ++)
  {
    analogValue[channel] = (analogValue[channel] + analogRead(channel))/2;
  }
  setWakeTime((uint16_t)&task_ReadAnalog, 200); //Task is sleeping / will not be called for 300 miliseconds    
}

/*
 * ===============================================================================================================================
It is assumed you have a led connected to DIGITAL line LED0
*/
void task_BlinkLED()
{
  if(ledState == 1)
  {
    digitalWrite(LED0, HIGH);
    ledState=0;
    setWakeTime((uint16_t)&task_BlinkLED, 10); //Task is sleeping / will not be called for 250 miliseconds
  }
  else
  {
    digitalWrite(LED0, LOW);
    ledState=1;
    setWakeTime((uint16_t)&task_BlinkLED, 490); //Task is sleeping / will not be called for 250 miliseconds
  }
}

/*
 * ===============================================================================================================================
Increasing counter1, storing value in string
*/
void task_TestCounter1()
{
  static int cnt = 0;

  if(cnt>9999)
  {
    cnt=0;
    sprintf(cnt1Str, "%d", cnt);
  }
  else
  {
    cnt++;
    sprintf(cnt1Str, "%d", cnt);
  }
  delay(4); //Slowing down Counters to make demonstation easier
}

/*
 * ===============================================================================================================================
Increasing counter2, storing value in string
*/
void task_TestCounter2()
{
  static int cnt = 0;

  if(cnt>9999)
  {
    cnt=0;
    sprintf(cnt2Str, "%d", cnt);
  }
  else
  {
    cnt++;
    sprintf(cnt2Str, "%d", cnt);
  }
  delay(4); //Slowing down Counters to make demonstation easier
}


/*
 * ===============================================================================================================================
Increasing counter3, storing value in string
NOTE: see setWake at bottom
*/
void task_TestCounter3()
{
  static int cnt = 0;

  if(cnt>9999)
  {
    cnt=0;
    sprintf(cnt3Str, "%d", cnt);
  }
  else
  {
    cnt++;
    sprintf(cnt3Str, "%d", cnt);
  }
  setWakeTime((uint16_t)&task_TestCounter3, 5000); //Task is sleeping / will not be called for 5000 miliseconds
}

/*
 * ===============================================================================================================================
Printing Values roughly every second, 20ms allowed for other things this program do
NOTE: see setWake at bottom
*/
void task_PrintValues()
{
  Serial.print("Counter1: ");
  Serial.print(cnt1Str);
  Serial.print(" Counter2: ");
  Serial.print(cnt2Str);
  Serial.print(" Counter3: ");
  Serial.print(cnt3Str);
  Serial.print(" Analog Values: ");

  for(int channel = 0; channel < 6; channel ++)
  {
    Serial.print(analogValue[channel]);
    Serial.print(", ");
  }
  Serial.println();
  
  setWakeTime((uint16_t)&task_PrintValues, 980); //Task is sleeping / will not be called for 980 miliseconds
}




