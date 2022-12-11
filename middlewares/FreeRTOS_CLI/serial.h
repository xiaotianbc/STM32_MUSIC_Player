/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * 1 tab == 4 spaces!
 */

#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

typedef void *xComPortHandle;

extern QueueHandle_t xRxedChars;


xComPortHandle xSerialPortInitMinimal(unsigned long ulWantedBaud,
                                      unsigned portBASE_TYPE uxQueueLength);

void vSerialPutString(xComPortHandle pxPort,
                      const signed char *const pcString,
                      unsigned short usStringLength);

signed portBASE_TYPE xSerialGetChar(xComPortHandle pxPort,
                                    signed char *pcRxedChar,
                                    TickType_t xBlockTime);

signed portBASE_TYPE xSerialPutChar(xComPortHandle pxPort,
                                    signed char cOutChar,
                                    TickType_t xBlockTime);

portBASE_TYPE xSerialWaitForSemaphore(xComPortHandle xPort);

void vSerialClose(xComPortHandle xPort);

#endif /* ifndef SERIAL_COMMS_H */
