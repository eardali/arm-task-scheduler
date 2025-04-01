/**
 * @file    OS.h
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
 *          Cloned from reference repo and revise for alternative usage, March 2025. 
 *          A task returns a value which is used to determine next execution time, hence a task can dynamically adjust its period by itself.
 */ 

#ifndef OS_H_
#define OS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef NULL
#define NULL            (void *)0               /**< NULL ptr. */
#endif

#define OS_MAX_TASK_NUM ((uint8_t)25u)          /**< Maximal task number that can be registered. */
#define OS_MAX_TIME     ((uint32_t)86400000u)   /**< Maximal time that for task period (OS_MAX_TIME*time_ticks), 24h. */
#define OS_MIN_TIME     ((uint32_t)1u)          /**< Minimal time that for task period (OS_MIN_TIME*time_ticks). */

typedef uint32_t (*fncPtr)(void *);             /**< Function pointer for registering tasks. */

/**
 * States of the tasks.
 */
typedef enum
{
    SUSPENDED,                          /**< In the SUSPENDED state the task is ignored by the timer and executer. */
    BLOCKED,                            /**< In the BLOCKED state the task waits for the timer to put it into READY state. */
    READY,                              /**< In the READY state the task is ready to be called and executed in the main function. */
    STOPPED                             /**< In the STOPPED state the task is dropped from the task array. */
} OS_state;

/**
 * Variables of the tasks.
 */
typedef struct
{
    fncPtr      function;               /**< This is the task that gets called periodically. */
    uint32_t    task_period;            /**< The period we want to call task. */
    uint32_t    execute_time;           /**< Next execution time of the task, if os time reaches this value, then the task is put into READY state. */
    OS_state    state;                  /**< The current state of the task. */
    void * 		data_ptr;				/**< data to pass task function */
} OS_struct;

/**
 * Feedback and error handling for the task creation and queries.
 */
typedef enum
{
    OK = 0,                             /**< OK:    Everything went as expected. */
    NOK_NULL_PTR,                       /**< ERROR: Null pointer as a task. */
    NOK_TIME_LIMIT,                     /**< ERROR: The time period is more or less, than the limits. */
    NOK_CNT_LIMIT,                      /**< ERROR: Something horrible happened, consider to increase OS_MAX_TASK_NUM. */
    NOK_UNKNOWN
} OS_feedback;

/**
 * Task period time.
 */
typedef enum
{
    period_end,                         /**< task is stopped, ie last time execution, or a task which is executed once can return this */
    period_1ms = 1,                     /**< 1 ms */
    period_10ms = period_1ms * 10,      /**< 10 ms */
    period_100ms = period_1ms * 100,    /**< 100 ms */
    period_1s = period_1ms * 1000,      /**< 1 second */
    period_1m = period_1ms * 1000 * 60, /**< 1 minute */
    period_1h = period_1m * 60          /**< 1 hour */
} task_period;


OS_feedback OS_TaskCreate(fncPtr function, uint32_t default_task_period, OS_state default_state, void *function_data_ptr, uint32_t defer_time);
OS_feedback OS_TaskCreateSimple(fncPtr function);
OS_feedback OS_TaskScheduleSimple(fncPtr function, uint32_t defer_time);
bool OS_TaskIsInQueue(fncPtr function);
void OS_TaskTimer(void);
void OS_TaskExecution(void);
uint32_t OS_GetOsTime(void);
OS_state OS_GetTaskState(fncPtr function);
uint32_t OS_GetTaskPeriod(fncPtr function);
uint32_t OS_GetTaskExecuteTime(fncPtr function);
OS_feedback OS_SetTaskState(fncPtr function, OS_state new_state);
OS_feedback OS_SetTaskPeriod(fncPtr function, uint32_t new_task_period);
OS_feedback OS_SetTaskExecuteTime(fncPtr function, uint32_t new_execute_time);

#endif /* OS_H_ */
