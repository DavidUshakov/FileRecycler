#pragma once
#include <fltKernel.h>
#include <dontuse.h>
#define MAX_PATH 260
#include "RegistryUtils.h"

class PathManager {
public:
	static PathManager* GetInstance();
	static void Delete();
	NTSTATUS GenerateReplacedFolderPath(__in const PUNICODE_STRING _folder, __inout PUNICODE_STRING _outPath);

	// _fileName - pure file name without slashes
	// _outPath - already initialized unicode string with buffer
	NTSTATUS GenerateRemovedFilePath(__in const PUNICODE_STRING _fileName, __inout PUNICODE_STRING _outPath);
	const PWCH GetRecycleBinPath() const;
	const PWCH GetRecycleBinVolumePath() const;
	const PWCH GetRecycleBinVolumeFolderPath() const;

	PathManager(__in const PathManager& _other) = delete;
	const PathManager& operator =(__in const PathManager& _other) = delete;

private:
	PathManager();
	RegistryUtils m_registryUtils;
	WCHAR m_appDataRoamingBuffer[MAX_PATH];
	WCHAR m_appDataRoamingRecycleBinBuffer[MAX_PATH];

	NTSTATUS InitializeFilePathes();
	NTSTATUS CreateRecycleBinFolder();
};