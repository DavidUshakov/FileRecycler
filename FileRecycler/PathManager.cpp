#include "PathManager.h"
#include "DebugPrinter.h"
#include "Memory.h"
static PathManager* g_pPathManager{ 0 };
LONG isPathManagerSet{ 0 };
PathManager::PathManager() : m_registryUtils(), m_appDataRoamingBuffer{ 0 }, m_appDataRoamingRecycleBinBuffer{ 0 }
{
	ULONG status = STATUS_SUCCESS;
	status = InitializeFilePathes();
	if (!NT_SUCCESS(status))
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!PathManager: InitializeFilePathes Failed, status=%08x\n",
				status));
	}

	status = CreateRecycleBinFolder();
	if (!NT_SUCCESS(status))
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!PathManager: CreateRecycleBinFolder Failed, status=%08x\n",
				status));
	}
}

// Don't call it from DriverEntry because in that case it fails to get values from registry.
PathManager* PathManager::GetInstance()
{
	if (!InterlockedExchange(&isPathManagerSet, 1))
	{
		if (!g_pPathManager)
		{
			InterlockedExchangePointer((PVOID*)&g_pPathManager, new (PagedPool, POOL_TAG) PathManager());
			if (!g_pPathManager)
			{
				PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
					("PathManager!GetInstance: Failed to create an object.\n"));
			}
		}
		
	}

	return g_pPathManager;
}

void PathManager::Delete()
{
	if (g_pPathManager)
	{
		delete (g_pPathManager);
	}

	g_pPathManager = 0;
}

NTSTATUS PathManager::GenerateReplacedFolderPath(__in const PUNICODE_STRING _folder, __inout PUNICODE_STRING _outPath)
{
	ULONG status = STATUS_SUCCESS;

	if (!g_pPathManager)
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!GenerateReplacedFolderPath: g_pPathManager is deleted\n"));
		return STATUS_INVALID_PARAMETER;
	}

	if (!_folder || !_folder->Buffer || !_outPath || !_outPath->Buffer)
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!GenerateReplacedFolderPath: Failed, invaid input params. _folder:%ll, _folder->Buffer:%ll, _outPath:%ll, _outPath->Buffer:%ll\n",
				_folder, _folder->Buffer, _outPath, _outPath->Buffer));
		return STATUS_INVALID_PARAMETER;
	}

	status = RtlAppendUnicodeToString(_outPath, m_appDataRoamingBuffer);
	if (!NT_SUCCESS(status))
	{
		goto ErrorRtlAppendUnicodeToString;
	}

	status = RtlAppendUnicodeStringToString(_outPath, _folder);
	if (!NT_SUCCESS(status))
	{
		goto ErrorRtlAppendUnicodeToString;
	}

	return status;

ErrorRtlAppendUnicodeToString:
	PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
		("PathManager!GenerateReplacedFolderPath: RtlAppendUnicodeToString Failed, status=%08x\n",
			status));

	return status;
}

NTSTATUS PathManager::GenerateRemovedFilePath(__in const PUNICODE_STRING _fileName, __inout PUNICODE_STRING _outPath)
{
	ULONG status = STATUS_SUCCESS;

	if (!g_pPathManager)
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!GenerateRemovedFilePath: g_pPathManager is deleted\n"));
		return STATUS_INVALID_PARAMETER;
	}

	if (!_fileName || !_fileName->Buffer || !_outPath || !_outPath->Buffer)
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!GenerateReplacedFolderPath: Failed, invaid input params. _fileName:%ll, _fileName->Buffer:%ll, _outPath:%ll, _outPath->Buffer:%ll\n",
				_fileName, _fileName->Buffer, _outPath, _outPath->Buffer));
		return STATUS_INVALID_PARAMETER;

	}

	status = RtlAppendUnicodeToString(_outPath, m_appDataRoamingBuffer);
	if (!NT_SUCCESS(status))
	{
		goto ErrorRtlAppendUnicodeToString;
	}

	status = RtlAppendUnicodeToString(_outPath, this->GetRecycleBinPath());
	if (!NT_SUCCESS(status))
	{
		goto ErrorRtlAppendUnicodeToString;
	}

	status = RtlAppendUnicodeToString(_outPath, L"\\");
	if (!NT_SUCCESS(status))
	{
		goto ErrorRtlAppendUnicodeToString;
	}

	status = RtlAppendUnicodeStringToString(_outPath, _fileName);
	if (!NT_SUCCESS(status))
	{

		goto ErrorRtlAppendUnicodeToString;
	}

	return status;

ErrorRtlAppendUnicodeToString:
	PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
		("PathManager!GenerateReplacedFolderPath: RtlAppendUnicodeToString Failed, status=%08x\n",
			status));

	return status;
}

const PWCH PathManager::GetRecycleBinPath() const
{
	return L"\\RecycleBin2";
}

const PWCH PathManager::GetRecycleBinVolumePath() const
{
	return L"\\Device\\HarddiskVolume3";
}

const PWCH PathManager::GetRecycleBinVolumeFolderPath() const
{
	return L"\\Device\\HarddiskVolume3\\";
}

NTSTATUS PathManager::InitializeFilePathes()
{
	ULONG status = STATUS_SUCCESS;

	// Generating pathes
	WCHAR currentUserHomePathBuffer[MAX_PATH] = { 0 };
	UNICODE_STRING currentUserHome;
	UNICODE_STRING appDataRoaming;
	UNICODE_STRING appDataRoamingRecycleBin;
	RtlInitUnicodeString(&currentUserHome, currentUserHomePathBuffer);
	currentUserHome.MaximumLength = MAX_PATH;
	status = m_registryUtils.GetCurrentUserHomePath(&currentUserHome);
	if (!NT_SUCCESS(status))
	{
		PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
			("PathManager!InitializeFilePathes: GetCurrentUserHomePath Failed, status=%08x\n",
				status));
		return status;
	}

	RtlInitUnicodeString(&appDataRoaming, m_appDataRoamingBuffer);
	appDataRoaming.MaximumLength = MAX_PATH;
	status = RtlAppendUnicodeToString(&appDataRoaming, GetRecycleBinVolumePath());
	if (!NT_SUCCESS(status))
	{

		goto RtlAppendUnicodeToStringError;
	}

	status = RtlAppendUnicodeStringToString(&appDataRoaming, &currentUserHome);
	if (!NT_SUCCESS(status))
	{
		goto RtlAppendUnicodeToStringError;
	}

	status = RtlAppendUnicodeToString(&appDataRoaming, L"\\AppData\\Roaming");
	if (!NT_SUCCESS(status))
	{
		goto RtlAppendUnicodeToStringError;
	}

	RtlInitUnicodeString(&appDataRoamingRecycleBin, m_appDataRoamingRecycleBinBuffer);
	appDataRoamingRecycleBin.MaximumLength = MAX_PATH;
	status = RtlAppendUnicodeStringToString(&appDataRoamingRecycleBin, &appDataRoaming);
	if (!NT_SUCCESS(status))
	{
		goto RtlAppendUnicodeToStringError;
	}

	status = RtlAppendUnicodeToString(&appDataRoamingRecycleBin, GetRecycleBinPath());
	if (!NT_SUCCESS(status))
	{
		goto RtlAppendUnicodeToStringError;
	}

	status = RtlAppendUnicodeToString(&appDataRoamingRecycleBin, L"\\");
	if (!NT_SUCCESS(status))
	{
		goto RtlAppendUnicodeToStringError;
	}

	return status;

RtlAppendUnicodeToStringError:
	PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
		("PathManager!InitializeFilePathes: RtlAppendUnicodeToString Failed, status=%08x\n",
			status));

	return status;

}

NTSTATUS PathManager::CreateRecycleBinFolder()
{
	ULONG status = STATUS_SUCCESS;

	UNICODE_STRING appDataRoamingRecycleBin;
	RtlInitUnicodeString(&appDataRoamingRecycleBin, m_appDataRoamingRecycleBinBuffer);
	appDataRoamingRecycleBin.MaximumLength = MAX_PATH;
	HANDLE fileHandle;
	IO_STATUS_BLOCK iosb;
	OBJECT_ATTRIBUTES ObjectAttributes;

	InitializeObjectAttributes(
		&ObjectAttributes,
		&appDataRoamingRecycleBin,
		(OBJ_CASE_INSENSITIVE |
			OBJ_KERNEL_HANDLE),
		NULL,
		NULL
	);

	status = ZwCreateFile(
		&fileHandle,
		GENERIC_READ | SYNCHRONIZE,
		&ObjectAttributes,
		&iosb,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		FILE_CREATE,
		FILE_DIRECTORY_FILE,
		NULL,
		0
	);

	if (NT_SUCCESS(status))
	{
		ZwClose(fileHandle);
	}
	else if (status == STATUS_OBJECT_NAME_COLLISION)
	{
		status = STATUS_SUCCESS;
	}

	return status;
}
