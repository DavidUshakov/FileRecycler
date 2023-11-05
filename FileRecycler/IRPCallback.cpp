#pragma once

#include "IRPCallback.h"
#include "RegistryUtils.h"
#include "PathManager.h"
#include "Memory.h"

/*************************************************************************
    MiniFilter callback routines.
*************************************************************************/
FLT_PREOP_CALLBACK_STATUS
IrpCreatePreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    LONG status= STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    PAGED_CODE();

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL || ExGetPreviousMode() == KernelMode)
    {
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    __try {

        PUNICODE_STRING pTargetFileName = &Data->Iopb->TargetFileObject->FileName;
        if (!pTargetFileName || !pTargetFileName->Buffer)
        {
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        PathManager* pathManager = PathManager::GetInstance();
        if (!pathManager)
        {
            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: failed to get pathManager parameter\n"));
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        // Proceeding replace recycle bin path
        if (wcsstr(pTargetFileName->Buffer, pathManager->GetRecycleBinPath()) == pTargetFileName->Buffer)
        {
            PFLT_FILE_NAME_INFORMATION targetFileNameInfo;
            status = FltGetFileNameInformation(Data,
                FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY, &targetFileNameInfo);

            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: Processing create for file %wZ (Cbd = %p, FileObject = %p)\n",
                    &targetFileNameInfo->Name,
                    Data,
                    FltObjects->FileObject));

            status = FltParseFileNameInformation(targetFileNameInfo);
            if (!NT_SUCCESS(status))
            {

                PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                    ("IrpCreatePreOperation: Failed to parse name information for file %wZ (Cbd = %p, FileObject = %p)\n",
                        &targetFileNameInfo->Name,
                        Data,
                        FltObjects->FileObject));
                return FLT_PREOP_COMPLETE;
            }

            if (wcsstr(targetFileNameInfo->Volume.Buffer, pathManager->GetRecycleBinVolumeFolderPath()) != targetFileNameInfo->Volume.Buffer)
            {
                return FLT_PREOP_COMPLETE;
            }


            UNICODE_STRING replacePath;
            replacePath.Buffer = (PWCHAR)ExAllocatePoolWithTag(PagedPool, MAX_PATH, POOL_TAG);
            Guard<WCHAR> replacePathGuard(replacePath.Buffer);
            if (!replacePath.Buffer)
            {
                PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                    ("IrpCreatePreOperation: Failed to allocate memory for file: %wZ (Cbd = %p, FileObject = %p)\n",
                        &targetFileNameInfo->Name,
                        Data,
                        FltObjects->FileObject));
                return FLT_PREOP_SUCCESS_WITH_CALLBACK;
            }

            replacePath.MaximumLength = MAX_PATH;
            replacePath.Length = 0;
            status = pathManager->GenerateReplacedFolderPath(pTargetFileName, &replacePath);
            if (!NT_SUCCESS(status))
            {

                PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                    ("IrpCreatePreOperation: Failed to generate replaced folder path for file %wZ (Cbd = %p, FileObject = %p)\n",
                        &targetFileNameInfo->Name,
                        Data,
                        FltObjects->FileObject));
                return FLT_PREOP_COMPLETE;
            }

            status = IoReplaceFileObjectName(Data->Iopb->TargetFileObject,
                replacePath.Buffer,
                replacePath.Length);
            if (!NT_SUCCESS(status))
            {

                PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                    ("IrpCreatePreOperation: Failed to replace file object name for file %wZ (Cbd = %p, FileObject = %p)\n",
                        &targetFileNameInfo->Name,
                        Data,
                        FltObjects->FileObject));
                return FLT_PREOP_COMPLETE;
            }

            Data->IoStatus.Status = STATUS_REPARSE;
            Data->IoStatus.Information = IO_REPARSE;
            FltSetCallbackDataDirty(Data);

            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: Replaced file path from:%wZ to: %wZ \n",
                    &targetFileNameInfo->Name,
                    &replacePath));

            return FLT_PREOP_COMPLETE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();

        PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
            ("IrpCreatePreOperation: FltSetInformationFile failed exception code status=%08x\n", status));

    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}


FLT_PREOP_CALLBACK_STATUS IrpSetInformationFilePreOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    PAGED_CODE();

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL || ExGetPreviousMode() == KernelMode)
    {
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    LONG status= STATUS_SUCCESS;

    __try {

        if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass != FileDispositionInformation)
        {
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }
        
        UNICODE_STRING pFileName = Data->Iopb->TargetFileObject->FileName;
        if (!pFileName.Buffer)
        {
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        PathManager* pathManager = PathManager::GetInstance();

        PFLT_FILE_NAME_INFORMATION fileNameInfo;
        status = FltGetFileNameInformation(Data,
            FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_FILESYSTEM_ONLY, &fileNameInfo);

        PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
            ("IrpCreatePreOperation: Processing create for file %wZ (Cbd = %p, FileObject = %p)\n",
                &fileNameInfo->Name,
                Data,
                FltObjects->FileObject));

        status = FltParseFileNameInformation(fileNameInfo);
        if (!NT_SUCCESS(status))
        {

            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: Failed to parse name information for file %wZ (Cbd = %p, FileObject = %p)\n",
                    &fileNameInfo->Name,
                    Data,
                    FltObjects->FileObject));
            return FLT_PREOP_COMPLETE;
        }

        // hardcoded folder. The files is being deleted will move to RecycleBin folder from this folder and its subfolders only.
        if (wcsstr(fileNameInfo->Name.Buffer, L"\\Device\\HarddiskVolume3\\Users\\") != fileNameInfo->Name.Buffer)
        {
            return FLT_PREOP_COMPLETE;
        }

        PFILE_DISPOSITION_INFORMATION pTargetFileDispositionInformation = (PFILE_DISPOSITION_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;

        if (pTargetFileDispositionInformation->DeleteFile == FALSE)
        {
            return FLT_PREOP_COMPLETE;
        }

        // Cancel deletion for the file
        pTargetFileDispositionInformation->DeleteFile = FALSE;
        Data->IoStatus.Status = STATUS_REPARSE;
        Data->IoStatus.Information = IO_REPARSE;
        FltSetCallbackDataDirty(Data);

        // Generate a new path for the deletee file inside of RecycleBin2
        UNICODE_STRING removedFilePath;
        removedFilePath.Buffer= (PWCHAR)ExAllocatePoolWithTag(PagedPool, MAX_PATH, POOL_TAG);
        Guard<WCHAR> removedFilePathGuard(removedFilePath.Buffer);
        if (!removedFilePath.Buffer)
        {
            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: Failed to allocate memory for file: %wZ (Cbd = %p, FileObject = %p)\n",
                    &fileNameInfo->Name,
                    Data,
                    FltObjects->FileObject));
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        removedFilePath.Length = 0;
        removedFilePath.MaximumLength = MAX_PATH;
        status = pathManager->GenerateRemovedFilePath(&fileNameInfo->FinalComponent, &removedFilePath);
        if (!NT_SUCCESS(status))
        {
            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: PathManager::GenerateRemovedFilePath failed exception code status=%08x\n", status));
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        ULONG fileRenameInformationLength = sizeof(FILE_RENAME_INFORMATION) + (removedFilePath.Length);
        PFILE_RENAME_INFORMATION  fileRenameinformation = (PFILE_RENAME_INFORMATION)ExAllocatePoolWithTag(PagedPool, fileRenameInformationLength, POOL_TAG);
        Guard<FILE_RENAME_INFORMATION> fileRenameinformationGuard(fileRenameinformation);
        if (!fileRenameinformation)
        {
            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: Failed to allocate memory for file: %wZ (Cbd = %p, FileObject = %p)\n",
                    &fileNameInfo->Name,
                    Data,
                    FltObjects->FileObject));
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }

        fileRenameinformation->ReplaceIfExists = TRUE;
        fileRenameinformation->RootDirectory = 0;
        fileRenameinformation->FileNameLength = removedFilePath.Length;
        memcpy(fileRenameinformation->FileName, removedFilePath.Buffer, removedFilePath.Length);
        status = FltSetInformationFile(FltObjects->Instance, FltObjects->FileObject, fileRenameinformation, fileRenameInformationLength, FileRenameInformation);

        if (!NT_SUCCESS(status))
        {
            PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
                ("IrpCreatePreOperation: FltSetInformationFile failed for file: %wZ (Cbd = %p, FileObject = %p)\n",
                    &fileNameInfo->Name,
                    Data,
                    FltObjects->FileObject));
            return FLT_PREOP_SUCCESS_WITH_CALLBACK;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();

        PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
            ("IrpCreatePreOperation: FltSetInformationFile failed exception code status=%08x\n", status));

    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS
IrpSetFileInformationPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);
    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_POSTOP_CALLBACK_STATUS
IrpCreatePostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,
        ("FileRecycler!IrpCreatePostOperation: Entered\n"));

    return FLT_POSTOP_FINISHED_PROCESSING;
}