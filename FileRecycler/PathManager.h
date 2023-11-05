#pragma once
#include <fltKernel.h>
#include <dontuse.h>
#define MAX_PATH 260
#include "RegistryUtils.h"

// PathManager - is a class that implements routine with creating RecycleBin2 folder and also allows the programer to generate file pathes to replace in callbacks
class PathManager {
public:
	static PathManager* GetInstance();
	static void Delete();
	// _folder - must be "\RecycleBin2\*some file or folder*"
	// _outPath - already initialized unicode string with buffer
	NTSTATUS GenerateReplacedFolderPath(__in const PUNICODE_STRING _folder, __inout PUNICODE_STRING _outPath);

	// _fileName - pure file name without slashes
	// _outPath - already initialized unicode string with buffer
	NTSTATUS GenerateRemovedFilePath(__in const PUNICODE_STRING _fileName, __inout PUNICODE_STRING _outPath);

	// returns "\\RecycleBin2"
	const PWCH GetRecycleBinPath() const;

	// returns "\\Device\\HarddiskVolume3\\"
	const PWCH GetRecycleBinVolumePath() const;

	// "\\Device\\HarddiskVolume3\\"
	const PWCH GetRecycleBinVolumeFolderPath() const;

	PathManager(__in const PathManager& _other) = delete;
	const PathManager& operator =(__in const PathManager& _other) = delete;

private:
	PathManager();
	RegistryUtils m_registryUtils;
	WCHAR m_appDataRoamingBuffer[MAX_PATH];
	WCHAR m_appDataRoamingRecycleBinBuffer[MAX_PATH];

	NTSTATUS InitializeFilePathes();
	NTSTATUS CreateRecycleBinFolderByUser();
	NTSTATUS CreateRecycleBinFolderInC();	
};