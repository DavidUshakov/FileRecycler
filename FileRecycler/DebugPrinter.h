#pragma once
/*
    In this file is some general code for debug print.

*/
#include <fltKernel.h>
#include <dontuse.h>


#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

static ULONG gTraceFlags = 0;

#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

#define DBGMSG(level, prefix, msg) { \
            INT dbgLevel = level; \
            if (MAX_DEBUG_LEVEL <= (dbgLevel)) { \
                DbgPrint("%s %s (%d): ", prefix, __FILE__, __LINE__); \
                DbgPrint(msg); \
            } \
        }