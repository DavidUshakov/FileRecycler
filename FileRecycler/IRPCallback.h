#pragma once
/*
    In this file are implemented callbacks for mini-filter driver.

*/
#include "DebugPrinter.h"

extern PFLT_FILTER gFilterHandle;
static ULONG_PTR OperationStatusCtx = 1;

EXTERN_C_START

FLT_PREOP_CALLBACK_STATUS
IrpCreatePreOperation
(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

/*
    IrpSetInformationFilePreOperation is used to move files to RecycleBin2 when there is a FILE_DISPOSITION call and DELETE_FILE flag is set to true.
*/
FLT_PREOP_CALLBACK_STATUS 
IrpSetInformationFilePreOperation
(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);


FLT_POSTOP_CALLBACK_STATUS
IrpSetFileInformationPostOperation
(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_POSTOP_CALLBACK_STATUS
IrpCreatePostOperation
(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

EXTERN_C_END