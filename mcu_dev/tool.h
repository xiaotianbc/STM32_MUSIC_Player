//
// Created by xiaotian on 2022/9/14.
//

#pragma once

#include <stdint.h>

typedef enum {
    FAILED = 0, PASSED = !FAILED
} TestStatus;

TestStatus Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);
