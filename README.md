# ArduinoScheduler
Basic inactive ageing scheduler.

###Introduction
This project is an example of how to use my scheduler. This was developed to resolve some timing issues experienced in bigger Arduino
projects. These issues are discussed under *Why I did it* below. The scheduler makes it easy to run different Arduino tasks (functions)
in parallel with different priorities. Time slots are dynamically allocated according to priority and distributed evenly between the tasks.

This is a non-preemptive scheduler. As such it will wait for each task to complete before running the next one. This is also called a
co-operative scheduler because all tasks need to work together. This means that if one task hogs the processor, the others don't get a
chance to run. Tasks should be built to finish as quickly as possible, all waiting is to be taken care of by the scheduler. Ideally you
should not have to call delay() ever.

###SchedulerExample1

This is a basic example of using my scheduler with task priorities and sleep time.

It runs the following tasks in parallel:

* It flashes a LED in one task
* Detects a button press
* Shows whether a button has been pressed using a LED
* Samples analogue inputs
* Increments counter1
* Increments counter2
* Prints values of all above actions

####Test setup
I used and Arduino UNO R3 to test this example. I have two LED's wired to digital out D12 and D13 with appropriate resistors
and polarity (2k2) to ground. I also have a push button wired between digital in D3 and ground(GND). Lastly I monitor the
USB port using the Arduino environment Serial monitor. I have the *baud rate* at *115200* baud in the example. This could be
modified in the Serial.begin() statement.

I compiled the code in Arduino 1.6.6 - GCC 4.8.1 on OpenSuSE Linux. I have used the same scheduler in my own projects in Arduino
1.5 as well.

####What is a Scheduler
A lot have been written about schedulers in places like Wikipedia and Information Technology materials.
I do not intend to try and improve on that.

A scheduler is typically a task with the highest priority in an operating system like Linux, Unix, BSD, IOS and others.
This task controls and distributes processor time slots to tasks as required. In a fancy scheduler (preemptive scheduler)
the task gets a slot, then gets frozen and the next task gets a slot, etc. Schedulers aim to be fair and give time slots
to tasks as they need them. Enough said, apart from the fact that this scheduler presented here is NOT preemptive.

This scheduler here only calls tasks with a fixed priority as predefined, and explained below. We need to carefully
plan the scheduling and give more time slots to tasks that need them. For example we don't want to miss a button press,
and we need to give it more priority. We can also break a logical function in two, as in the SchedulerExample1 for the
task_checkButtonPress() and task_buttonPressedLED() functions. The reasoning behind this is that we need to monitor the
button press in task_checkButtonPress() with high priority to make sure we don't miss it. We do not need to switch the LED
in task_buttonPressedLED() instantaneously, as a human being takes many milliseconds to observe an event. If the LED switches
a few microseconds earlier or later, it makes no difference.

####Basic Concepts

* Every task is given a priority, a higher priority means more time spent doing this task.

* A task is a function, a function could call other functions as in any other Arduino sketch.

* Since this scheduler is a non preemptive scheduler (See Terminology below) it calls one task (function) and wait for
  this task to complete. In a perfect world this task would finish instantaneously, as having delays, etc will make all
  other tasks wait as well and cause the system to be unresponsive.

  * In order to achieve the above it is recommended to break the sketch into many simple tasks as an example:
    * task_checkButtonPress
    * task_readAnalog
    * task_blinkLED
    * task_buttonPressedLED
    * task_testCounter1
    * task_testCounter2
    * task_testCounter3
    * task_printValues
    
    This will enable us to control how much priority each task is given using the scheduler in a simple fashion.
    
* I also added a time based feature to the scheduler for tasks that only needs to be called after a certain time.
  As an example, we call task_printValues only once a second and testCounter3 only once every 5 seconds. This is done by using 
  the setWakeTime(&task, milliseconds) function. This function takes the task as first argument and time in 
  milliseconds as the second argument. Typically a task (function) calls this as the last statement, which then says:
  I don't want to be called for X milliseconds (or I want to sleep for X milliseconds). This can be used to achieve a result
  similar to a delay() call. HOWEVER it is important to understand the benefits. You code is not stopped of N milliseconds,
  as other tasks can continue and this one will only be called again after N milliseconds.
  
  
####Task List
 
 Example code:

``` 
// PRIORITY, AGE, WAKETIME X 2, TASKPTR
uint16_t tasks[][5] = {
  {64,  0, 0, 0, (uint16_t) &task_CheckButtonPress},
  {1,   0, 0, 0, (uint16_t) &task_ReadAnalog},
  {1,   0, 0, 0, (uint16_t) &task_BlinkLED},
  {1,   0, 0, 0, (uint16_t) &task_ButtonPressedLED},
  {1,   0, 0, 0, (uint16_t) &task_TestCounter1},
  {4,   0, 0, 0, (uint16_t) &task_TestCounter2},
  {1,   0, 0, 0, (uint16_t) &task_TestCounter3},
  {1,   0, 0, 0, (uint16_t) &task_PrintValues},
  {0,   0, 0, 0, 0 } //Critical, indicates the end of task list!
};
```

  
The above array containing a list of tasks needs some explanation.

The declaration of ***uint_16_t tasks[][5] =*** specifies a two dimensional array for N tasks (undefined number of rows)
with 5 values each.

If we look at the first entry in the array:

{64 ,  0, 0, 0, (uint16_t) &task_CheckButtonPress}


The values:

* 64 - specifies a task priority of 64 (values between 1 and 64 is recommended)
* 0 - reserved for scheduler use, should be 0
* 0 - reserved for scheduler use, should be 0
* 0 - reserved for scheduler use, should be 0
* (unit16_t) &task_CheckButtonPress - is a pointer (address of) task_CheckButtonPress function to call for this task.


#### SchedulerExample1 - all the hairy detail....
* As stated earlier, the highest priority is assigned to monitor the button in order not to miss a button press.
* Counter1 has a priority of 1 and Counter2 has a priority of 4, this will then show that Counter2 
  is called more often and therefore gets more execution time slots.
* Counter3 has the same priority as Counter1 however it calls setWakeTime(...) at the end of each execution
  and does not get called again for 5 seconds. Important to understand is unlike a delay(5000) it does not pause
  the program for 5 seconds, but only this one task! WARNING: don't call sleep for one task from another task,
  as this can cause funny behaviour very difficult to debug. WARNING: cut an paste is evil.
* task_CheckButtonPress only checks if the button is still pressed after 100ms, but if it has not been pressed
  it will be called every several microseconds (as per scheduler priority) to make sure we don't miss a button press.
* task_ButtonPressedLED() keeps LED1 on for 2000ms after the button press was detected. It also sleeps 100ms every
  cycle, as this helps to not waste scheduler task execution time slots. This implies our LED will switch on and off up
  to 100ms late. This is however not a real issue. This is a good example of the things to be taken into account using a
  fixed priority scheduler and task sleep cycles.
* task_PrintValues() is used to print the values of counter and analogue sample averages. It then calls setWakeTime(...)
  and sleeps 980ms to wake up roughly 1 second later.
* It is also important to understand that the sleep time set by setWakeTime, implies the minimum time slept. The task will
  only be called in its next time slot after the time has expired. This is one of the reasons that all tasks should be kept
  short and sweet, as any single task running for many milliseconds, delays the next one, etc.
  
  
####Possible Variants
* The scheduler as presented works well without the setWakeTime() component as well, and has obviously even less of an overhead
when implemented as such, however this component is a small price to pay for a lot of flexibility. I will present such an example
in future, for the lean and mean projects out there.
* I have also played around with a sleep task with auto tuning priority and duration, this then gives me time spent in tasks and 
  time spent sleeping (doing NOPs in a loop). This then calculates time slept in a second and time doing tasks which is displayed
  as a ratio. I will put this up here in future as time allows.
* I still need to experiment, but it should be possible to put the Arduino in sleep mode when nothing needs to be executed
  (all tasks are sleeping), and thus save power which could prove useful for some applications like loggers sampling temperature
  or humidity every minute and saving data to an SD card every 10 minutes?




###Why I did it.
I have written software for a pump house equipment & irrigation control system.
This basically has several state machines to monitor the dam level, fill the tank used for
water in the house, and manage borehole resting cycles etc.
This is then managed via the LCD/Keypad and or remotely from my house using a web interface.

The system is running on a Arduino Mega 2560 with a DFRobot LCD display, and some custom built electronics
to interface with valves, pump controllers and sensors.

All of this works quite well, except that I had sets of loops inside loops and interrupt triggered flags, to call all
the necessary methods in sequence with the right priority and timing. If you add one method, the buttons would not
respond reliably, due to the timing that changed etc.

Something that is worth mentioning, is that this was my first Arduino Project. I did not foresee that the
project will become so complex, as I never imagined an Arduino to be so powerful, allowing me to keep adding functionality.

I needed to resolve this in a simple and neat fashion. I also wanted to do something lightweight that will work with the
minimum hardware, preferably even on an Arduino UNO for other projects.

I have written several useful libraries for the above project, and the intention is to publish them here
over time with examples. This scheduler I think is really unique (on Arduino) and powerfully. It made the web
interface, buttons and display updates on the above project much more responsive and the actual code less complex.

I discussed this timing loop spaghetti that evolved as my project has grown with my brother, who is
a very creative and clever developer. He has come across some multitasking schedulers in his studies and he
suggested I implement an inactive ageing scheduler. I then did this, and the result was a scheduler that
surpassed all my expectations. The software never misses a button press, makes adding a task (function)
easy and the CPU is not doing anything for around 66 percent of the time.

I have several versions of this scheduler for different applications (they can be combined into one, however
sometimes simple is better if you have only 2k of RAM) and a basic requirement.

The scheduler in SchedulerExample1 is quite basic, but works very well in practice. It is only a few lines of code
and uses minimal resources.


###Real Schedulers
This is an extremely simple scheduler, with no active priority or time slot tuning. Therefore an inactive ageing scheduler.
There are much better algorithms out there, for an AVR it is however useful to have a small footprint and low overhead.
If complex software fascinates you, read up on the scheduler used in Linux, it is considered one of the best out there.


###Arduino
Lastly the Arduino platform this was written for is an awesome tool. 
Many thanks to every AVR GCC & Arduino contributor.

See: http://www.arduino.cc




