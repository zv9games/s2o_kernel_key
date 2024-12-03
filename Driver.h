/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>

#include "device.h"
#include "queue.h"
#include "trace.h"

EXTERN_C_START

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD s2okernelkeyEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP s2okernelkeyEvtDriverContextCleanup;

#define IOCTL_PACKET_CAPTURE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PACKET_RELEASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

NTSTATUS s2okernelkeyCreateDevice(_Inout_ PWDFDEVICE_INIT DeviceInit);
NTSTATUS s2okernelkeyCreate(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS s2okernelkeyClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS s2okernelkeyDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS PacketCapture(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS PacketRelease(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

EXTERN_C_END
