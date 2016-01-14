# ArduinoScheduler
Basic inactive aging scheduler.


**This page is under construction, please come back in a day or so.**
**2016/01/14**

###The Problem
I have written software for a pump house equipment & irigation control system.
This basically has several state machines to monitor the dam level and filling cycles, 
fill the tank used for water in the house, and manage borehole resting cycles etc.
This is then managed via the LCD/Keypad and or remotely from my house using a web interface.

The system is running on a Arduino Mega 2560 with a DFRobot LCD display, and some custom built electronics
to interface with valves, pump controllers and sensors.

All of this works quite well, except that I had sets of loops inside loops and interrupt triggered flags, to call all
the neccesary methods in sequence with the right priority and timing. If you add one method, the buttons would not
respond reliably, as the timing changed etc.

Something that is worth mentioning, is that this was my first Arduino Project. I did not foresee that the
project will become so complex, as I never imagnined an Arduino to be so powerfull, allowing me to keep adding functionality.

I needed to resolve this in a simple and neat fasion. I also wanted to do something lightweight that will work with the
minimum hardware, preferably even on an Arduino UNO for other projects.

I have written several usefull libraries for the above project, and the intention is to publish them here
over time with examples. This scheduler I think is really unique (on arduino) and powerfull.

###The Solution
I discussed this timing loop spagetti that evolved as my project has grown with my brother, who is
a very creative and clever developer. He has come accross some multitasking schedulers in his studies and he
sugested I implement an inactive ageging scheduler. I then did this, and the result was a scheduler that
surpassed all my expectations. Using this my software never misses a button press, makes adding a task (function)
easy and the cpu is not doing anytning for around 66 percent of the time.

I have several versions of this scheduler (they can be combined into one, however sometimes simple is better if
you have only 2k of RAM) and a more basic reauirement.

####Basic Concepts

* Every task is given a priority, a higher priority means more time spent doing this task.

* A task is a function, a function could call other functions as in any other arduino sketch.

* Since this scheduler is a non preemptive scheduler (See Teminology below) it calls one task (function) and wait for
  this task to complete. In a perfect world this task would finish instantaniously, as having delays, etc will make all
  other tasks wait as well and cause the system to be unresponsive.

  * In order to achieve the above it is reccomended to break the sketch into many simple tasks as an example:
    * task_checkButtonPress
    * task_readAnalog
    * task_blinkLED
    * task_buttonPressedLED
    * task_testCounter1
    * task_testCounter2
    * task_testCounter3
    * task_printValues
    
    This will enable us to control how much priority each task is given using the scheduler in a simple fasion.
    
* I also added a time based feature to the scheduler for tasks that only needs to be called after a certain time.
  As an example, we call task_printValues only once a second and testCounter3 only once every 5 seconds. This is done by using 
  the setWakeTime(&task, milliseconds) function. This function takes the task as first argument and time in 
  milliseconds as the second argument. Typically a task (function) calls this as the last statement, which then says:
  I don't want to be called for X milliseconds (or I want to sleep for X milliseconds). This can be used to achieve a result
  similar to a delay() call. HOWEVER it is important to understand the benifits. You code is not stopped of N milliseconds,
  as other tasks can continue and this one will only be called again after N milliseconds.
  
  
 ####Task List
 
 Example code:

' 
// PRIORITY, AGE, WAKETIME X 2, TASKPTR
uint16_t tasks[][5] = {
  {64 ,  0, 0, 0, (uint16_t) &task_CheckButtonPress},
  {4,    0, 0, 0, (uint16_t) &task_ReadAnalog},
  {32,   0, 0, 0, (uint16_t) &task_BlinkLED},
  {8,    0, 0, 0, (uint16_t) &task_ButtonPressedLED},
  {1,    0, 0, 0, (uint16_t) &task_TestCounter1},
  {4,    0, 0, 0, (uint16_t) &task_TestCounter2},
  {16,   0, 0, 0, (uint16_t) &task_TestCounter3},
  {64,   0, 0, 0, (uint16_t) &task_PrintValues},
  {0,    0, 0, 0, 0 } //Critical, indicates the end of task list!
};
'
  
The above array containing a list of tasks needs some explanation.

The declaration of *uint_16_t tasks[][5] =* implies an two dimentional array for N tasks (undefined number of rows) with 5 values each.

If we look at the first entry in the array:

{64 ,  0, 0, 0, (uint16_t) &task_CheckButtonPress}


  

### Terminology



