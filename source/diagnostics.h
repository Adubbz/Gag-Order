#pragma once

#include <stdbool.h>
#include <switch.h>

typedef enum 
{
    FAILED,
    UNKNOWN,
    NAGGED_NOSETTING,
    NAGGED_SETTING,
    UNNAGGED
} NagStatus;

void printDebugInfo();
NagStatus checkNagStatus(void);