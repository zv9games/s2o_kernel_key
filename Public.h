/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_s2okernelkey,
    0x079c9ab5,0xbab9,0x428c,0xb7,0xd4,0x31,0x62,0x6b,0x2b,0x1a,0xb2);
// {079c9ab5-bab9-428c-b7d4-31626b2b1ab2}
