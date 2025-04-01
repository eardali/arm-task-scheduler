/**
 * @file    OS.c
 * @author  Ferenc Nemeth
 * @date    21 Jul 2018
 * @brief   This is a really simple, non-preemptive task scheduler.
 *          You can register tasks with their runnable function and the periodic time you want to call them.
 *          With a help of a timer the tasks get into READY state after every time period (except if they are SUSPENDED) and
 *          they get called and executed in the main()'s inifinte loop. After they are finished everything starts over.
 *          This Scheduler helps you to keep your tasks and timing organized.
 *
 *          Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth/
 *          
 *          Copyright (c) 2025 github.com/eardali 
 *          
 *          Clone from reference repo and revise for alternative usage, March 2025
 *          A task returns a value which is used to determine next execution time, hence a task can dynamically adjust its period by itself.
 */ 

#include "OS.h"

static OS_struct    task_array[OS_MAX_TASK_NUM];    /**< Variables and information for every single task. */
static uint8_t      task_count = 0u;                /**< Tail index of the task array, not necessarily to be number of tasks, some task may be dropped. */
static uint32_t     os_time = 0;                    /**< os clock variable, increases every 1ms */

/**
 * Find and return task position if task in list
 * If task is not in list, returns and invalid position
 */
static uint8_t OS_TaskFind(fncPtr function){
    for(uint8_t i = 0; i < task_count; i++){
        if(task_array[i].function == function){
            return i;
        }
    }
    return OS_MAX_TASK_NUM+1; //task in not in array, return an invalid position
}

/**
 * Find and return a proper position to insert task to the list
 * If there is empty position which is place of previously STOPPED (dropped) task, return this position
 * Otherwise, return the tail of the task list
 */
static uint8_t OS_TaskInsertPosition(void){
	uint8_t i;
    for(i = 0; i <= task_count; i++){
        if(task_array[i].function == NULL){
            if(i == task_count){ //if no empty position till to the last task, add to the end of list, update task count
                task_count++;
            }
            break;
        }     
    }
    return i;
}

/**
 * @brief   This function registers the tasks.
 *          If there is empty position in task array, inserts task to that proper position (other than the end of list).
 *          At the beginning there is an error check for registration.
 *          If everything is good, the input values are saved.
 * @param   function: The task we want to call periodically.
 * @param   default_task_period: The time it gets called periodically, this is actually updated by return value of the task function.
 * @param   default_state: The state it starts (recommended state: BLOCKED).
 * @param   function_data_ptr: Data to be delivered to the task function (NULL if no data).
 * @param   defer_time: Delay time for the first execution of task function (0 if no need to delay).
 * @return  OS_feedback: Feedback about the success or cause of error of the registration.
 */
OS_feedback OS_TaskCreate(fncPtr function, uint32_t default_task_period, OS_state default_state, void *function_data_ptr, uint32_t defer_time)
{
    OS_feedback ret = NOK_UNKNOWN;

    /* Time limit. */
    if ((OS_MIN_TIME > default_task_period) || (OS_MAX_TIME < default_task_period))
    {
        ret = NOK_TIME_LIMIT;
    }
    /* Task number limit. */
    else if (OS_MAX_TASK_NUM <= task_count)
    {
        ret = NOK_CNT_LIMIT;
    }
    /* Everything is fine, save. */
    else
    {
        uint8_t position = OS_TaskFind(function); //check if task laready in list
        if(position > OS_MAX_TASK_NUM){ // a new task, insert to empty position in task array
            position = OS_TaskInsertPosition();
            task_array[position].function = function;
            task_array[position].task_period = default_task_period;
            task_array[position].state = default_state;
            task_array[position].execute_time = os_time + defer_time;
            task_array[position].data_ptr = function_data_ptr;
        }else{ //task already in array, update values
            task_array[position].task_period = default_task_period;
            task_array[position].state = default_state;
            task_array[position].execute_time = os_time + defer_time;
            task_array[position].data_ptr = function_data_ptr;
        }
        ret = OK;
    }
    return ret;
}

/**
 * @brief   Simply create a task with basic default parameters.
 *          A BLOCKED task is added to task list with 1ms period, no input data and 0 defer time.
 * @param   function: The task we want to call periodically.
 * @return  OS_feedback: Feedback about the success or cause of error of the registration.
 */
OS_feedback OS_TaskCreateSimple(fncPtr function)
{
    OS_feedback ret;
    ret = OS_TaskCreate(function, period_1ms, BLOCKED, NULL, 0);
    return ret;
}

/**
 * @brief   Simply schedule a task with default parameters.
 *          A BLOCKED task is added to task list with 1ms period and no input data, it will be first executed after given defer time.
 * @param   function: The task we want to call periodically.
 * @param   defer_time: Delay time for the first execution of task function (0 if no need to delay).
 * @return  OS_feedback: Feedback about the success or cause of error of the registration.
 */
OS_feedback OS_TaskScheduleSimple(fncPtr function, uint32_t defer_time)
{
    OS_feedback ret;
    ret = OS_TaskCreate(function, period_1ms, BLOCKED, NULL, defer_time);
    return ret;
}

/**
 * @brief   Check if a task is already in array.
 * @param   function: The task pointer which we want to check.
 * @return  true (1) if task is in array, else false (0).
 */
bool OS_TaskIsInQueue(fncPtr function){
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM)
        return true;
    else
        return false;
}

/**
 * @brief   This function keeps track of the tasks' time and puts them into READY state.
 *          If any task state is STOPPED, it is cleared from the task array, to save memory.
 *          This location can be used during new task creation.
 *          This function SHALL be called in a timer interrupt with a 1ms period.
 * @param   void
 * @return  void
 */
void OS_TaskTimer(void)
{
    os_time = os_time + period_1ms;
    for (uint8_t i = 0u; i < task_count; i++)
    {
        /* Ignore SUSPENDED tasks. */
        if (task_array[i].state != SUSPENDED)
        {
            if(task_array[i].state == BLOCKED){
                /* Put it into READY state. */
                if (task_array[i].execute_time <= os_time)
                {
                    task_array[i].execute_time += task_array[i].task_period * period_1ms; //update next execution time
                    task_array[i].state	= READY;
                }
            }else if(task_array[i].state == STOPPED){
                //Remove stopped tasks from list
            	task_array[i].function = NULL;
            	task_array[i].task_period = 0;
            	task_array[i].execute_time = 0;
                task_array[i].state = SUSPENDED; //if task stopped, set as suspended and ignore 
                task_array[i].data_ptr = NULL;

            }
        }
    }
}

/**
 * @brief   This function calls the READY tasks and then puts them back into BLOCKED state.
 *          If return value of task indicates last time execution, its state is arranged as STOPPED, hence can be dropped from task list
 *          This function SHALL be called in the infinite loop.
 * @param   void
 * @return  void
 */
void OS_TaskExecution(void)
{
    uint32_t period;
    for (uint8_t i = 0u; i < task_count; i++)
    {
        /* If it is ready, then call it.*/
        if (READY == task_array[i].state)
        {
            period = task_array[i].function(task_array[i].data_ptr); //execute task
            if(period != task_array[i].task_period){ //if task period is changed by return value, update those
                task_array[i].task_period = period; //update execution period based on function return value
                task_array[i].execute_time = OS_GetOsTime() + period; //also update next execution time
            }
            if(period == period_end)
                task_array[i].state = STOPPED; //if task returns end, stop task
            else
                task_array[i].state = BLOCKED;
        }
    }
}

/**
 * @brief   Returns current time value of the task scheduler (OS).
 * @param   void
 * @return  OS time tick count (clock).
 */
uint32_t OS_GetOsTime(void){
    return os_time;
}

/**
 * @brief   Returns the state of the task.
 * @param   function: Function pointer of the task.
 * @return  OS_state: State of the task.
 */
OS_state OS_GetTaskState(fncPtr function)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM)
        return task_array[position].state;
    else //no such a task
        return SUSPENDED;
}

/**
 * @brief   Returns the period of the task.
 * @param   function: Function pointer of the task.
 * @return  The task period.
 *          If task is not in list, returns 0.
 */
uint32_t OS_GetTaskPeriod(fncPtr function)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM)
        return task_array[position].task_period;
    else
        return 0;
}

/**
 * @brief   Returns the next execution time of the task.
 * @param   function: Function pointer of the task.
 * @return  Next execution time of the task, when os time reaches this values, task executed.
 *          If task is not in list, returns 0.
 */
uint32_t OS_GetTaskExecuteTime(fncPtr function)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM)
        return task_array[position].execute_time;
    else
        return 0;
}

/**
 * @brief   Manually changes the task state.
 * @param   function: Function pointer of the task.
 * @param   new_state: The new state of the task.
 * @return  OS_feedback: OK (0) if successful.
 */
OS_feedback OS_SetTaskState(fncPtr function, OS_state new_state)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM){
        task_array[position].state = new_state;
        return OK;
    }else{
        return NOK_NULL_PTR;
    }
}

/**
 * @brief   Manually changes the task period.
 * @param   function: Function pointer of the task.
 * @param   new_task_period: The new execution period of the task.
 * @return  OS_feedback: OK (0) if successful.
 */
OS_feedback OS_SetTaskPeriod(fncPtr function, uint32_t new_task_period)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM){
        task_array[position].task_period = new_task_period;
        return OK;
    }else{
        return NOK_NULL_PTR;
    }
}

/**
 * @brief   Manually changes the task execution time.
 * @param   function: Function pointer of the task.
 * @param   new_execute_time: The new execution time of the task.
 * @return  OS_feedback: OK (0) if successful.
 */
OS_feedback OS_SetTaskExecuteTime(fncPtr function, uint32_t new_execute_time)
{
    uint8_t position;
    position = OS_TaskFind(function);
    if(position < OS_MAX_TASK_NUM){
        task_array[position].execute_time = new_execute_time;
        return OK;
    }else{
        return NOK_NULL_PTR;
    }
}
