/*++

Module Name:

    driver.c

Abstract:

    This file contains the driver entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include <ntddk.h>
#include "driver.h"
#include "driver.tmh"
#include <initguid.h>
#include "trace.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, s2okernelkeyEvtDeviceAdd)
#pragma alloc_text (PAGE, s2okernelkeyEvtDriverContextCleanup)
#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;

    // Initialize WPP Tracing
    WPP_INIT_TRACING(DriverObject, RegistryPath);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Register cleanup callback
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = s2okernelkeyEvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, s2okernelkeyEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
        WPP_CLEANUP(DriverObject);
        return status;
    }

    // Set major function pointers
    DriverObject->MajorFunction[IRP_MJ_CREATE] = s2okernelkeyCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = s2okernelkeyClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = s2okernelkeyDeviceControl;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

NTSTATUS
s2okernelkeyEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    NTSTATUS status = s2okernelkeyCreateDevice(DeviceInit);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

    return status;
}

VOID
s2okernelkeyEvtDriverContextCleanup(
    _In_ WDFOBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    PAGED_CODE();

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

    // Stop WPP Tracing
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)DriverObject));
}

NTSTATUS
s2okernelkeyCreate(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "s2okernelkeyCreate: Device opened.");

    return STATUS_SUCCESS;
}

NTSTATUS
s2okernelkeyClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "s2okernelkeyClose: Device closed.");

    return STATUS_SUCCESS;
}

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
    case IOCTL_PACKET_CAPTURE:
        status = PacketCapture(DeviceObject, Irp);
        break;
    case IOCTL_PACKET_RELEASE:
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

NTSTATUS PacketCapture(
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

    // Allocate memory for packet data (Example)
    PACKET_DATA* packetData = (PACKET_DATA*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PACKET_DATA), 'pktd');
    if (!packetData) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "PacketCapture: Memory allocation failed.");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Capture packet logic here (Example)
    // For now, just copy the input buffer to the allocated packet data
    RtlCopyMemory(packetData, buffer, sizeof(PACKET_DATA));

    // Process captured packet data (Example)
    ProcessPacketData(packetData);

    // Free allocated memory
    ExFreePoolWithTag(packetData, 'pktd');

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PACKET_DATA);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PacketCapture: Packet captured and processed.");

    return STATUS_SUCCESS;
}

// Example processing function
VOID ProcessPacketData(PACKET_DATA* packetData)
{
    // Implement packet processing logic here
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Processing packet data.");
}


NTSTATUS PacketRelease(
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

    // Allocate memory for packet data (Example)
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

    // Re-inject packet logic here (Example)
    ReinjectPacketData(packetData);

    // Free allocated memory
    ExFreePoolWithTag(packetData, 'pktd');

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PACKET_DATA);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "PacketRelease: Packet released and re-injected.");

    return STATUS_SUCCESS;
}

// Example re-injection function
VOID ReinjectPacketData(PACKET_DATA* packetData)
{
    // Implement packet re-injection logic here
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "Re-injecting packet data.");
}
