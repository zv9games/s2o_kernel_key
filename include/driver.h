#ifndef DRIVER_H
#define DRIVER_H

#include <ntddk.h>
#include <wdf.h>

// Declare DriverEntry
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

// Declare Event Functions
NTSTATUS
s2okernelkeyEvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
);

VOID
s2okernelkeyEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
);

#endif // DRIVER_H
