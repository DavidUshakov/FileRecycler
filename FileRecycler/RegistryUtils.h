#pragma once
#include <wdm.h>
#define MAX_PATH 260

// RegistryUtils is a class for working with registry. 
class RegistryUtils 
{
public:
	// MARK: Registry queries doesn't work from DriverEntry
	NTSTATUS GetCurrentUserHomePath(__inout const PUNICODE_STRING _outHomePath);
};
