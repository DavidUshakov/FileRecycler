#pragma once

#include "DebugPrinter.h"

extern PFLT_FILTER gFilterHandle;
static ULONG_PTR OperationStatusCtx = 1;



EXTERN_C_START

VOID
FileRecyclerOperationStatusCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext
);

FLT_POSTOP_CALLBACK_STATUS
FileRecyclerPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

BOOLEAN
FileRecyclerDoRequestOperationStatus(
    _In_ PFLT_CALLBACK_DATA Data
);





FLT_PREOP_CALLBACK_STATUS
IrpCreatePreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

/*
    IrpSetInformationFilePreOperation is used to move files to RecycleBin2 when there is a FILE_DISPOSITION call and DELETE_FILE flag is set to true.
*/
FLT_PREOP_CALLBACK_STATUS IrpSetInformationFilePreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);


FLT_POSTOP_CALLBACK_STATUS
IrpSetFileInformationPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

FLT_POSTOP_CALLBACK_STATUS
IrpCreatePostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
);

EXTERN_C_END

