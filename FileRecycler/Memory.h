#pragma once

/*
    This file contains new and delete operators and also a memory Guard.

*/

// original: https://github.com/microsoft/Windows-driver-samples/blob/75f6610aab567835c056bf761f93132ea8e07307/avstream/avshws/device.cpp#L460

#pragma warning(disable:4595) // operator non-member operator new or delete functions may not be declared inline


#include "DebugPrinter.h"
#define POOL_TAG 'Tag1'

FORCEINLINE PVOID operator new
(
	size_t          iSize,
	_When_((poolType& NonPagedPoolMustSucceed) != 0,
		__drv_reportError("Must succeed pool allocations are forbidden. "
			"Allocation failures cause a system crash"))
	POOL_TYPE       poolType
	)
{
	return ExAllocatePoolZero(poolType, iSize, POOL_TAG);
}

FORCEINLINE PVOID operator new
(
	size_t          iSize,
	_When_((poolType& NonPagedPoolMustSucceed) != 0,
		__drv_reportError("Must succeed pool allocations are forbidden. "
			"Allocation failures cause a system crash"))
	POOL_TYPE       poolType,
	ULONG           tag
	)
{
	return ExAllocatePoolZero(poolType, iSize, tag);
}

FORCEINLINE PVOID
operator new[](
	size_t          iSize,
	_When_((poolType& NonPagedPoolMustSucceed) != 0,
		__drv_reportError("Must succeed pool allocations are forbidden. "
			"Allocation failures cause a system crash"))
	POOL_TYPE       poolType,
	ULONG           tag
	)
{
	return ExAllocatePoolZero(poolType, iSize, tag);
}

/*++

Routine Description:

    Array delete() operator.

Arguments:

    pVoid -
        The memory to free.

Return Value:

    None

--*/
FORCEINLINE void
__cdecl
operator delete[](
    PVOID pVoid
    )
{
    if (pVoid)
    {
        ExFreePool(pVoid);
    }
}

/*++

Routine Description:

    Sized delete() operator.

Arguments:

    pVoid -
        The memory to free.

    size -
        The size of the memory to free.

Return Value:

    None

--*/
FORCEINLINE void __cdecl operator delete
(
    void* pVoid,
    size_t /*size*/
    )
{
    if (pVoid)
    {
        ExFreePool(pVoid);
    }
}

/*++

Routine Description:

    Sized delete[]() operator.

Arguments:

    pVoid -
        The memory to free.

    size -
        The size of the memory to free.

Return Value:

    None

--*/
FORCEINLINE void __cdecl operator delete[]
(
    void* pVoid,
    size_t /*size*/
    )
{
    if (pVoid)
    {
        ExFreePool(pVoid);
    }
}

FORCEINLINE void __cdecl operator delete
(
    PVOID pVoid
    )
{
    if (pVoid) {
        ExFreePool(pVoid);
    }
}



#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteIoStatus(
    IN PIO_STATUS_BLOCK Address
)

{

    if (Address >= (IO_STATUS_BLOCK* const)MM_USER_PROBE_ADDRESS) {
        Address = (IO_STATUS_BLOCK* const)MM_USER_PROBE_ADDRESS;
    }

    ((volatile IO_STATUS_BLOCK*)Address)->Status = Address->Status;
    ((volatile IO_STATUS_BLOCK*)Address)->Information = Address->Information;
    return;
}

#else

#define ProbeForWriteIoStatus(Address) {                                     \
    if ((Address) >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {       \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile IO_STATUS_BLOCK *)(Address) = *(volatile IO_STATUS_BLOCK *)(Address); \
}

#endif

template <typename T>
struct Guard
{
    T* m_pData;
    inline Guard(T* _pData) : m_pData(_pData)
    {
    }

    inline ~Guard()
    {
        if (m_pData)
        {
            ExFreePoolWithTag(m_pData, POOL_TAG);
        }
    }

};

