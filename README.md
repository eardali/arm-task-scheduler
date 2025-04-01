# Simple Task Scheduler for Arm
A simple, non-preemptive, cooperative task scheduler.  

# Usage
- Place `OS_TaskTimer()` in `SysTick_Handler` of the ARM system or any other 1ms period timer interrupt, this is the heart beat of the scheduler.  
- Call `OS_TaskExecution()` in `main()` within a `while(true)` loop after creation of necessary tasks. It executes tasks in list/queue in a sequential manner. Task executions times are continuously checked, as task execution time reaches to the global os time (see `OS_GetOsTime()`), it is executed.  
- A task can be inserted to list/queue using `OS_TaskCreate(...)` function. A task can be scheduled to be executed for a later time by the `defer_time` parameter.  
- When a task `STOPPED`, it is dropped from the list to save memory. If need to pause a task `SUSPEND` it, and change its state to `BLOCKED` to resume.  
- A task function must return its period (`uint32_t` value) which is used to update task next execution time and period info. Hence a task can dynamically arrange period/next execution time of itself.  

# References
Referenced from [avr-simple-scheduler](https://github.com/ferenc-nemeth/avr-simple-scheduler) by [ferenc-nemeth](https://github.com/ferenc-nemeth).  

# Licence
Distributed under the terms of the MIT license.  
