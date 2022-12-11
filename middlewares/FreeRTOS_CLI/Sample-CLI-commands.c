/******************************************************************************
*
* http://www.FreeRTOS.org/cli
*
******************************************************************************/


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif

/*
 * The function that registers the commands that are defined within this file.
 */
void vRegisterSampleCLICommands(void);

/*
 * Implements the task-stats command.
 */
static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the run-time-stats command.
 */
#if(configGENERATE_RUN_TIME_STATS == 1)
static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif /* configGENERATE_RUN_TIME_STATS */

/*
 * Implements the echo-three-parameters command.
 */
static BaseType_t
prvThreeParameterEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the echo-parameters command.
 */
static BaseType_t prvParameterEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);


/*
 * 实现一个简单的打印hello world的命令.
 */
static BaseType_t
prvPrintHelloWorldNTimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//  生成随机数
static BaseType_t prvRandIntCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//  播放音乐
static BaseType_t prvPlayMusicCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

//  列出文件
static BaseType_t prvLsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);



/*
 * Implements the "query heap" command.
 */
#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

/*
 * Implements the "trace start" and "trace stop" commands;
 */
#if(configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1)
static BaseType_t prvStartStopTraceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xTaskStats =
        {
                "top", /* The command string to type. */
                "\r\ntop:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
                prvTaskStatsCommand, /* The function to run. */
                0 /* No parameters are expected. */
        };

/* Structure that defines the "echo_3_parameters" command line command.  This
takes exactly three parameters that the command simply echos back one at a
time. */
static const CLI_Command_Definition_t xThreeParameterEcho =
        {
                "echo-3-parameters",
                "\r\necho-3-parameters <param1> <param2> <param3>:\r\n Expects three parameters, echos each in turn\r\n",
                prvThreeParameterEchoCommand, /* The function to run. */
                3 /* Three parameters are expected, which can take any value. */
        };

/* Structure that defines the "echo_parameters" command line command.  This
takes a variable number of parameters that the command simply echos back one at
a time. */
static const CLI_Command_Definition_t xParameterEcho =
        {
                "echo-parameters",
                "\r\necho-parameters <...>:\r\n Take variable number of parameters, echos each in turn\r\n",
                prvParameterEchoCommand, /* The function to run. */
                -1 /* The user can enter any number of commands. */
        };


static const CLI_Command_Definition_t xPrintHelloWorldNTime =
        {
                "helloworldn",
                "\r\nhelloworldn:\r\n print n times hello world to user console\r\n",
                prvPrintHelloWorldNTimeCommand, /* The function to run. */
                1 /* 不接受额外的参数. */
        };

static const CLI_Command_Definition_t xRandInt =
        {
                "randint",
                "\r\nrandint <param1> <param2>:\r\n Expects two parameters, echos a random number behind this \r\n",
                prvRandIntCommand, /* The function to run. */
                2 /* 不接受额外的参数. */
        };


static const CLI_Command_Definition_t xPlayMusic =
        {
                "play",
                "\r\nplay <param1>:\r\n play a wav music: \r\n",
                prvPlayMusicCommand, /* The function to run. */
                1 /* 不接受额外的参数. */
        };

static const CLI_Command_Definition_t xLs =
        {
                "ls",
                "\r\nls:\r\n list files in sd card \r\n",
                prvLsCommand, /* The function to run. */
                0 /* 不接受额外的参数. */
        };

#if(configGENERATE_RUN_TIME_STATS == 1)
/* Structure that defines the "run-time-stats" command line command.   This
generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t xRunTimeStats =
{
    "run-time-stats", /* The command string to type. */
    "\r\nrun-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n",
    prvRunTimeStatsCommand, /* The function to run. */
    0 /* No parameters are expected. */
};
#endif /* configGENERATE_RUN_TIME_STATS */

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
/* Structure that defines the "query_heap" command line command. */
static const CLI_Command_Definition_t xQueryHeap =
{
    "query-heap",
    "\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n",
    prvQueryHeapCommand, /* The function to run. */
    0 /* The user can enter any number of commands. */
};
#endif /* configQUERY_HEAP_COMMAND */

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1
/* Structure that defines the "trace" command line command.  This takes a single
parameter, which can be either "start" or "stop". */
static const CLI_Command_Definition_t xStartStopTrace =
{
    "trace",
    "\r\ntrace [start | stop]:\r\n Starts or stops a trace recording for viewing in FreeRTOS+Trace\r\n",
    prvStartStopTraceCommand, /* The function to run. */
    1 /* One parameter is expected.  Valid values are "start" and "stop". */
};
#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */

/*-----------------------------------------------------------*/

//注册FreeRTOS-Cli的命令，如果有新的命令，需要在这里注册
void vRegisterSampleCLICommands(void) {
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand(&xTaskStats);
    //FreeRTOS_CLIRegisterCommand(&xThreeParameterEcho);
    // FreeRTOS_CLIRegisterCommand(&xParameterEcho);
    // FreeRTOS_CLIRegisterCommand(&xPrintHelloWorldNTime);
    FreeRTOS_CLIRegisterCommand(&xRandInt);
    FreeRTOS_CLIRegisterCommand(&xPlayMusic);
    FreeRTOS_CLIRegisterCommand(&xLs);

#if(configGENERATE_RUN_TIME_STATS == 1)
    {
        FreeRTOS_CLIRegisterCommand( &xRunTimeStats );
    }
#endif

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)
    {
        FreeRTOS_CLIRegisterCommand( &xQueryHeap );
    }
#endif

#if(configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1)
    {
        FreeRTOS_CLIRegisterCommand( &xStartStopTrace );
    }
#endif
}

/*-----------------------------------------------------------*/

static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *const pcHeader = "     State   Priority  Stack    #\r\n************************************************\r\n";
    BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    char cheap_size[20] = {0};  // display free heap size in menu top
    sprintf(cheap_size, "\r\nfree heap: %d B\r\n", xPortGetFreeHeapSize());
    strcpy(pcWriteBuffer, cheap_size);
    pcWriteBuffer += strlen(cheap_size);

    /* Generate a table of task stats. */
    strcpy(pcWriteBuffer, "Task");
    pcWriteBuffer += strlen(pcWriteBuffer);

    /* Minus three for the null terminator and half the number of characters in
    "Task" so the column lines up with the centre of the heading. */
    configASSERT(configMAX_TASK_NAME_LEN > 3);
    for (xSpacePadding = strlen("Task"); xSpacePadding < (configMAX_TASK_NAME_LEN - 3); xSpacePadding++) {
        /* Add a space to align columns after the task's name. */
        *pcWriteBuffer = ' ';
        pcWriteBuffer++;

        /* Ensure always terminated. */
        *pcWriteBuffer = 0x00;
    }
    strcpy(pcWriteBuffer, pcHeader);
    vTaskList(pcWriteBuffer + strlen(pcHeader));


    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

#if(configINCLUDE_QUERY_HEAP_COMMAND == 1)

static BaseType_t prvQueryHeapCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    sprintf( pcWriteBuffer, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", ( int ) xPortGetFreeHeapSize(), ( int ) xPortGetMinimumEverFreeHeapSize() );

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}

#endif /* configINCLUDE_QUERY_HEAP */
/*-----------------------------------------------------------*/

#if(configGENERATE_RUN_TIME_STATS == 1)

static BaseType_t prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char * const pcHeader = "  Abs Time      % Time\r\n****************************************\r\n";
BaseType_t xSpacePadding;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Generate a table of task stats. */
    strcpy( pcWriteBuffer, "Task" );
    pcWriteBuffer += strlen( pcWriteBuffer );

    /* Pad the string "task" with however many bytes necessary to make it the
    length of a task name.  Minus three for the null terminator and half the
    number of characters in	"Task" so the column lines up with the centre of
    the heading. */
    for( xSpacePadding = strlen( "Task" ); xSpacePadding < ( configMAX_TASK_NAME_LEN - 3 ); xSpacePadding++ )
    {
        /* Add a space to align columns after the task's name. */
        *pcWriteBuffer = ' ';
        pcWriteBuffer++;

        /* Ensure always terminated. */
        *pcWriteBuffer = 0x00;
    }

    strcpy( pcWriteBuffer, pcHeader );
    vTaskGetRunTimeStats( pcWriteBuffer + strlen( pcHeader ) );

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}

#endif /* configGENERATE_RUN_TIME_STATS */

/*-----------------------------------------------------------*/

static BaseType_t
prvThreeParameterEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate(足够的), so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    if (uxParameterNumber == 0) {
        /* The first time the function is called after the command has been
        entered just a header string is returned. */
        sprintf(pcWriteBuffer, "The three parameters were:\r\n");

        /* Next time the function is called the first parameter will be echoed
        back. */
        uxParameterNumber = 1U;

        /* There is more data to be returned as no parameters have been echoed
        back yet. */
        xReturn = pdPASS;
    } else {
        /* Obtain the parameter string. */
        //获取参数字符串，第一个参数是从哪里获取，默认是pcCommandString
        //第二个参数是获取第几个Parameter，然后把获取到的字符串长度放在第三个引用参数里
        pcParameter = FreeRTOS_CLIGetParameter
                (
                        pcCommandString,        /* The command string itself. */
                        uxParameterNumber,        /* Return the next parameter. */
                        &xParameterStringLength    /* Store the parameter string length. */
                );

        /* Sanity check something was returned. */
        configASSERT(pcParameter);

        /* Return the parameter string. */
        memset(pcWriteBuffer, 0x00, xWriteBufferLen);
        sprintf(pcWriteBuffer, "%d: ", (int) uxParameterNumber);
        strncat(pcWriteBuffer, pcParameter, (size_t) xParameterStringLength);
        strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));

        /* If this is the last of the three parameters then there are no more
        strings to return after this one. */
        if (uxParameterNumber == 3U) {
            /* If this is the last of the three parameters then there are no more
            strings to return after this one. */
            xReturn = pdFALSE;
            uxParameterNumber = 0;
        } else {
            /* There are more parameters to return after this one. */
            xReturn = pdTRUE;
            uxParameterNumber++;
        }
    }

    return xReturn;
}

/*-----------------------------------------------------------*/

static BaseType_t prvParameterEchoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 0;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    if (uxParameterNumber == 0) {
        /* The first time the function is called after the command has been
        entered just a header string is returned. */
        sprintf(pcWriteBuffer, "The parameters were:\r\n");

        /* Next time the function is called the first parameter will be echoed
        back. */
        uxParameterNumber = 1U;

        /* There is more data to be returned as no parameters have been echoed
        back yet. */
        xReturn = pdPASS;
    } else {
        /* Obtain the parameter string. */
        pcParameter = FreeRTOS_CLIGetParameter
                (
                        pcCommandString,        /* The command string itself. */
                        uxParameterNumber,        /* Return the next parameter. */
                        &xParameterStringLength    /* Store the parameter string length. */
                );

        if (pcParameter != NULL) {
            /* Return the parameter string. */
            memset(pcWriteBuffer, 0x00, xWriteBufferLen);
            sprintf(pcWriteBuffer, "%d: ", (int) uxParameterNumber);
            strncat(pcWriteBuffer, (char *) pcParameter, (size_t) xParameterStringLength);
            strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));

            /* There might be more parameters to return after this one. */
            xReturn = pdTRUE;
            uxParameterNumber++;
        } else {
            /* No more parameters were found.  Make sure the write buffer does
            not contain a valid string. */
            pcWriteBuffer[0] = 0x00;

            /* No more data to return. */
            xReturn = pdFALSE;

            /* Start over the next time this command is executed. */
            uxParameterNumber = 0;
        }
    }

    return xReturn;
}


static BaseType_t
prvPrintHelloWorldNTimeCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 1; //第一次就获取第一个参数

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter
            (
                    pcCommandString,        /* The command string itself. */
                    uxParameterNumber,        /* Return the next parameter. */
                    &xParameterStringLength    /* Store the parameter string length. */
            );

    char this_str[10];
    if (pcParameter != NULL) {
        memset(pcWriteBuffer, 0x00, xWriteBufferLen);
        //把第一个参数拷贝到this_str里
        strncpy(this_str, pcParameter, xParameterStringLength);
        int n = atoi(this_str);
        sprintf(pcWriteBuffer, "Your par is %d: \r\n", n);
        for (int i = 0; i < n; ++i) {
            strncat(pcWriteBuffer, "Hello world ", 12);
            strncat(pcWriteBuffer, (char *) pcParameter, (size_t) xParameterStringLength);
            strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));
        }
        strncat(pcWriteBuffer, "\r\n", strlen("\r\n"));
    }

    xReturn = pdFALSE;
    return xReturn;
}


static BaseType_t prvRandIntCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 1; //第一次就获取第一个参数

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    static int n1;
    static int n2;
    int nr;

    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter
            (
                    pcCommandString,        /* The command string itself. */
                    uxParameterNumber,        /* Return the next parameter. */
                    &xParameterStringLength    /* Store the parameter string length. */
            );

    char this_str[10];
    if (pcParameter != NULL) {
        memset(pcWriteBuffer, 0x00, xWriteBufferLen);
        //把第一个参数拷贝到this_str里
        strncpy(this_str, pcParameter, xParameterStringLength);

        if (uxParameterNumber == 1) {
            n1 = atoi(this_str);
            /* There are more parameters to return after this one. */
            xReturn = pdTRUE;
            uxParameterNumber++;
        } else {
            n2 = atoi(this_str);
            srand(xTaskGetTickCount());
            nr = rand() % (n2 - n1 + 1) + n1;
            sprintf(pcWriteBuffer, "rand int(%d - %d) is %d \r\n", n1, n2, nr);
            xReturn = pdFALSE;
            uxParameterNumber = 1;
        }
    }

    return xReturn;
}

#include "freeRTOS_app_task.h"
#include "board_fatfs_interface.h"
#include "printf.h"
#include "board_dac_sound.h"

static BaseType_t prvPlayMusicCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 1; //第一次就获取第一个参数

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    static int n1;
    static int n2;
    int nr;

    //获取参数字符串，第一个参数是从哪里获取，默认是pcCommandString
    //第二个参数是获取第几个Parameter，然后把获取到的字符串长度放在第三个引用参数里
    pcParameter = FreeRTOS_CLIGetParameter
            (
                    pcCommandString,        /* The command string itself. */
                    uxParameterNumber,        /* Return the next parameter. */
                    &xParameterStringLength    /* Store the parameter string length. */
            );

    static char this_str[30];
    if (pcParameter != NULL) {
        memset(this_str, 0x00, 30);
        //把第一个参数拷贝到this_str里
        strncpy(this_str, pcParameter, xParameterStringLength);

        xTaskCreate(Music_Player,  /* 任务入口函数 */
                    "playmusic",    /* 任务名字 */
                    4096,    /* 任务栈大小 */
                    this_str,        /* 任务入口函数参数 */
                    1,  /* 任务的优先级 */
                    NULL);  /* 任务控制块指针 */

    }


    xReturn = pdFALSE;
    return xReturn;
}

static BaseType_t prvLsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    const char *pcParameter;
    BaseType_t xParameterStringLength, xReturn;
    static UBaseType_t uxParameterNumber = 1; //第一次就获取第一个参数

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    (void) pcCommandString;
    (void) xWriteBufferLen;
    configASSERT(pcWriteBuffer);

    fatfs_ls(pcWriteBuffer);

    xReturn = pdFALSE;
    return xReturn;
}


/*-----------------------------------------------------------*/

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1

static BaseType_t prvStartStopTraceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
BaseType_t lParameterStringLength;

    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );

    /* Obtain the parameter string. */
    pcParameter = FreeRTOS_CLIGetParameter
                    (
                        pcCommandString,		/* The command string itself. */
                        1,						/* Return the first parameter. */
                        &lParameterStringLength	/* Store the parameter string length. */
                    );

    /* Sanity check something was returned. */
    configASSERT( pcParameter );

    /* There are only two valid parameter values. */
    if( strncmp( pcParameter, "start", strlen( "start" ) ) == 0 )
    {
        /* Start or restart the trace. */
        vTraceStop();
        vTraceClear();
        vTraceStart();

        sprintf( pcWriteBuffer, "Trace recording (re)started.\r\n" );
    }
    else if( strncmp( pcParameter, "stop", strlen( "stop" ) ) == 0 )
    {
        /* End the trace, if one is running. */
        vTraceStop();
        sprintf( pcWriteBuffer, "Stopping trace recording.\r\n" );
    }
    else
    {
        sprintf( pcWriteBuffer, "Valid parameters are 'start' and 'stop'.\r\n" );
    }

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}

#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */
