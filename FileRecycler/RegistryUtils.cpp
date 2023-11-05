#pragma once

#include "RegistryUtils.h"

NTSTATUS RegistryUtils::GetCurrentUserHomePath(__inout const PUNICODE_STRING _outHomePath)
{
	LONG status= STATUS_SUCCESS;
	RTL_QUERY_REGISTRY_TABLE queryTable[2];
	RtlZeroMemory(queryTable, sizeof(queryTable));

	if (!_outHomePath || !_outHomePath->Buffer || !_outHomePath->MaximumLength)
	{
		return STATUS_INVALID_PARAMETER;
	}

	queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_TYPECHECK;
	queryTable[0].DefaultType = (REG_SZ << RTL_QUERY_REGISTRY_TYPECHECK_SHIFT) | REG_NONE;
	queryTable[0].Name = L"HOMEPATH";
	queryTable[0].EntryContext = _outHomePath;
	status = RtlQueryRegistryValues(RTL_REGISTRY_USER, (PCWSTR)L"\\Volatile Environment", queryTable, 0, 0);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("RegistryUtils:GetCurrentUserHomePath failed to query registry values status = % 08x", status);
		return status;

	}
	return status;
}