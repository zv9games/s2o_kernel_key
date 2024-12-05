/* Summary of Completed Functions
Driver Entry (DriverEntry)

Device Add Event (s2okernelkeyEvtDeviceAdd)

Driver Context Cleanup (s2okernelkeyEvtDriverContextCleanup)

Handle Create Requests (s2okernelkeyCreate)

Handle Close Requests (s2okernelkeyClose)

Handle Device Control Requests (s2okernelkeyDeviceControl)

Packet Capture (PacketCapture)
*/


/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

// These are the libraries called at the beginning that contain all the tools used here.
#include <ntddk.h>
#define NDIS60_MINIPORT 1
#define NDIS_MINIPORT_DRIVER 1
#include <ndis.h>
#include "driver.h"
#include "driver.tmh"
#include "initguid.h"
#include "trace.h"


// This is memory allocation section. Defined here is data labeled with INIT is used once, and then discarded.
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, s2okernelkeyEvtDeviceAdd)
#pragma alloc_text (PAGE, s2okernelkeyEvtDriverContextCleanup)
#endif

// Constants and Macros
#define IOCTL_PACKET_CAPTURE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PACKET_RELEASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Data Structures
typedef struct _PACKET_DATA {
    ULONG Length;
    UCHAR Data[1];  // Flexible array member
} PACKET_DATA, *PPACKET_DATA;

// Driver Entry
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NDIS_STATUS status;
    NDIS_MINIPORT_DRIVER_CHARACTERISTICS miniportChars;
    NDIS_HANDLE driverHandle;

    // Initialize WPP Tracing
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Zero memory for miniportChars structure
    NdisZeroMemory(&miniportChars, sizeof(miniportChars));
    miniportChars.Header.Type = NDIS_OBJECT_TYPE_MINIPORT_DRIVER_CHARACTERISTICS;
    miniportChars.Header.Size = sizeof(NDIS_MINIPORT_DRIVER_CHARACTERISTICS);
    miniportChars.MajorNdisVersion = NDIS_MINIPORT_MAJOR_VERSION;
    miniportChars.MinorNdisVersion = NDIS_MINIPORT_MINOR_VERSION;
    miniportChars.InitializeHandlerEx = MiniportInitialize;
    miniportChars.HaltHandlerEx = MiniportHalt;

    status = NdisMRegisterMiniportDriver(
        DriverObject,
        RegistryPath,
        NULL,
        &miniportChars,
        &driverHandle
    );

    if (status != NDIS_STATUS_SUCCESS) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "NdisMRegisterMiniportDriver failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return STATUS_SUCCESS;
}




// Example MiniportInitialize function
NDIS_STATUS MiniportInitialize(
    _Out_ NDIS_HANDLE* MiniportAdapterHandle,
    _In_ NDIS_HANDLE  MiniportDriverContext,
    _In_ PNDIS_MINIPORT_INIT_PARAMETERS MiniportInitParameters
)
{
    // Initialization logic
    return NDIS_STATUS_SUCCESS;
}

// Example MiniportHalt function
VOID MiniportHalt(
    _In_ NDIS_HANDLE MiniportAdapterContext,
    _In_ NDIS_HALT_ACTION HaltAction
)
{
    // Cleanup logic
}


NTSTATUS
s2okernelkeyEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();  // Ensures the function runs at IRQL <= APC_LEVEL

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Initialize the device
    NTSTATUS status = s2okernelkeyCreateDevice(DeviceInit);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
s2okernelkeyCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    WDFDEVICE device;
    NTSTATUS status;

    // Create the device object
    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDeviceCreate failed %!STATUS!", status);
        return status;
    }

    // Additional device initialization code here

    return status;
}

NTSTATUS
PacketCapture(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputBufferLength = stack->Parameters.DeviceIoControl.InputBufferLength;

    // Validate buffer length
    if (inputBufferLength < sizeof(PACKET_DATA)) {
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "PacketCapture: Buffer too small.");
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Allocate memory for packet data
    PACKET_DATA* packetData = (PACKET_DATA*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PACKET_DATA), 'pktd');
    if (!packetData) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "PacketCapture: Memory allocation failed.");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Capture packet logic (Example: copying input buffer to allocated packet data)
    RtlCopyMemory(packetData, buffer, sizeof(PACKET_DATA));

    // Process captured packet data
    ProcessPacketData(packetData);

    // Free allocated memory
    ExFreePoolWithTag(packetData, 'pktd');

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PACKET_DATA);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PacketCapture: Packet captured and processed.");

    return STATUS_SUCCESS;
}

NTSTATUS
PacketRelease(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputBufferLength = stack->Parameters.DeviceIoControl.InputBufferLength;

    // Validate buffer length
    if (inputBufferLength < sizeof(PACKET_DATA)) {
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "PacketRelease: Buffer too small.");
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Allocate memory for packet data
    PACKET_DATA* packetData = (PACKET_DATA*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PACKET_DATA), 'pktd');
    if (!packetData) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "PacketRelease: Memory allocation failed.");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Copy buffer data to allocated packet data
    RtlCopyMemory(packetData, buffer, sizeof(PACKET_DATA));

    // Re-inject packet logic
    ReinjectPacketData(packetData);

    // Free allocated memory
    ExFreePoolWithTag(packetData, 'pktd');

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PACKET_DATA);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PacketRelease: Packet released and re-injected.");

    return STATUS_SUCCESS;
}

VOID
ReinjectPacketData(
    _In_ PPACKET_DATA packetData
)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Re-injecting packet data.");

    // Ensure 'pool' and 'filterHandle' are defined and initialized somewhere in your driver code
    extern NDIS_HANDLE pool;
    extern NDIS_HANDLE filterHandle;

    // 1. Allocate a NET_BUFFER_LIST structure
    PNET_BUFFER_LIST netBufferList = NdisAllocateNetBufferList(pool, 0, 0);
    if (!netBufferList) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Failed to allocate NET_BUFFER_LIST.");
        return;
    }

    // 2. Allocate a NET_BUFFER within the NET_BUFFER_LIST
    PNET_BUFFER netBuffer = NdisAllocateNetBuffer(netBufferList, NULL, 0, packetData->Length);
    if (!netBuffer) {
        NdisFreeNetBufferList(netBufferList);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "Failed to allocate NET_BUFFER.");
        return;
    }

    // 3. Copy packet data into the NET_BUFFER
    PVOID buffer = NdisGetDataBuffer(netBuffer, packetData->Length, NULL, 1, 0);
    if (buffer == NULL) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "NdisGetDataBuffer failed.");
        NdisFreeNetBufferList(netBufferList);
        return;
    }
    RtlCopyMemory(buffer, packetData->Data, packetData->Length);

    // 4. Indicate the packet to the networking stack for transmission
    NdisFSendNetBufferLists(filterHandle, netBufferList, 0, 0);

    // 5. Cleanup resources (handled by NDIS after transmission)
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Packet re-injected successfully.");
}



#define IOCTL_START_CAPTURE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STOP_CAPTURE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PACKET    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REINJECT_PACKET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

NTSTATUS
s2okernelkeyDeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_START_CAPTURE:
        // Start packet capture logic
        status = StartPacketCapture();
        break;
    case IOCTL_STOP_CAPTURE:
        // Stop packet capture logic
        status = StopPacketCapture();
        break;
    case IOCTL_GET_PACKET:
        // Get captured packet logic
        status = GetCapturedPacket(Irp);
        break;
    case IOCTL_REINJECT_PACKET:
        // Re-inject packet logic
        status = PacketRelease(DeviceObject, Irp);
        break;
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
StartPacketCapture()
{
    // Logic to start capturing packets
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "StartPacketCapture: Packet capture started.");
    return STATUS_SUCCESS;
}

NTSTATUS
StopPacketCapture()
{
    // Logic to stop capturing packets
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "StopPacketCapture: Packet capture stopped.");
    return STATUS_SUCCESS;
}

NTSTATUS
GetCapturedPacket(
    _In_ PIRP Irp
)
{
    // Logic to retrieve captured packet and copy it to the user buffer
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG outputBufferLength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    // Ensure we have enough space in the user buffer
    if (outputBufferLength < sizeof(PACKET_DATA)) {
        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information = 0;
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Assuming we have a packet in a global or context buffer
    extern PACKET_DATA* capturedPacket;  // Replace with actual logic to get captured packet
    RtlCopyMemory(buffer, capturedPacket, sizeof(PACKET_DATA));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PACKET_DATA);
    return STATUS_SUCCESS;
}

// Driver Context Cleanup
VOID
s2okernelkeyEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Perform any necessary cleanup here

    // Stop WPP Tracing
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
}

// Example MiniportInitialize function
NDIS_STATUS MiniportInitialize(
    _Out_ NDIS_HANDLE* MiniportAdapterHandle,
    _In_ NDIS_HANDLE MiniportDriverContext,
    _In_ PNDIS_MINIPORT_INIT_PARAMETERS MiniportInitParameters
)
{
    // Initialization logic
    return NDIS_STATUS_SUCCESS;
}

// Example MiniportHalt function
VOID MiniportHalt(
    _In_ NDIS_HANDLE MiniportAdapterContext,
    _In_ NDIS_HALT_ACTION HaltAction
)
{
    // Cleanup logic
}

