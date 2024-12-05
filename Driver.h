/*++

Module Name:

    driver.h

Abstract:

    This file contains the function prototypes and definitions for the driver.

Environment:

    Kernel-mode Driver Framework

--*/

#ifndef DRIVER_H
#define DRIVER_H

#include <ntddk.h>
#include <wdf.h>

// Constants and Macros
#define IOCTL_PACKET_CAPTURE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PACKET_RELEASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_START_CAPTURE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_STOP_CAPTURE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PACKET    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REINJECT_PACKET CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Data Structures
typedef struct _PACKET_DATA {
    ULONG Length;
    UCHAR Data[1];  // Flexible array member
} PACKET_DATA, *PPACKET_DATA;

// Function Prototypes
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS s2okernelkeyEvtDeviceAdd(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit);
VOID s2okernelkeyEvtDriverContextCleanup(_In_ WDFOBJECT DriverObject);
NTSTATUS s2okernelkeyCreate(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS s2okernelkeyClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS s2okernelkeyDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS PacketCapture(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS PacketRelease(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
VOID ProcessPacketData(_In_ PPACKET_DATA packetData);
VOID ReinjectPacketData(_In_ PPACKET_DATA packetData);

// Additional helper functions
NTSTATUS StartPacketCapture();
NTSTATUS StopPacketCapture();
NTSTATUS GetCapturedPacket(_In_ PIRP Irp);

#endif // DRIVER_H
