//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft end-user
// license agreement (EULA) under which you licensed this SOFTWARE PRODUCT.
// If you did not accept the terms of the EULA, you are not authorized to use
// this source code. For a copy of the EULA, please see the LICENSE.RTF on your
// install media.
//

/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Module Name:

    sha.h   SHA (logical) MDD

Abstract:

   Streams interface driver

Functions:

Notes:

--*/

#ifndef _SHA_DRV_H_
#define _SHA_DRV_H_

#include <pm.h>
#include "pmplatform.h"
#include "sha204_config.h"
//#include "s3c2440a.h"
#include <i2c.h>



#define SHA_SIG ' AHS'

#define VALID_CONTEXT( p ) \
   ( p && p->Sig && SHA_SIG == p->Sig )
typedef struct _SHA_DATA {
		uint8_t op_code;
		uint8_t param1;
		uint16_t param2;
		uint8_t datalen1;
		uint8_t *data1;
		uint8_t datalen2;
		uint8_t *data2;
		uint8_t datalen3;
		uint8_t *data3;
		uint8_t tx_size;
		uint8_t *tx_buffer;
		uint8_t rx_size;
		uint8_t *rx_buffer;
	}SHA_DATA,*PSHA_DATA;

#define INVALID_DATA_COUNT  (-1)
typedef struct _SHA_FASTCALL {
    PVOID Context;   // opaque context
    VOID (*SHA204_Init) (PVOID Context);
    VOID (*SHA204_Sleep)(PVOID Context);
	BOOL (*SHA204_WakeUp) (PVOID Context);
    unsigned char (*SHA204_Execute)(PVOID Context, PSHA_DATA data);
} SHA_FASTCALL, *PSHA_FASTCALL;


//
// I2C Bus Driver IOCTLS
//
#define FILE_DEVICE_SHA     FILE_DEVICE_CONTROLLER

// IN:  PI2C_IO_DESC
#define IOCTL_SHA204_INIT \
    CTL_CODE(FILE_DEVICE_SHA, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

// IN:  PI2C_IO_DESC
#define IOCTL_SHA204_SLEEP \
    CTL_CODE(FILE_DEVICE_SHA, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

// OUT: PI2C_FASTCALL
#define IOCTL_SHA204_GET_FASTCALL  \
    CTL_CODE(FILE_DEVICE_SHA, 6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SHA204_WAKEUP  \
    CTL_CODE(FILE_DEVICE_SHA, 7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SHA204_EXECUTE  \
	CTL_CODE(FILE_DEVICE_SHA, 8, METHOD_BUFFERED, FILE_ANY_ACCESS)

//
// SHA_CONTEXT
//
typedef struct _SHA_CONTEXT {

    DWORD   Sig;    // Signature

    CRITICAL_SECTION RegCS; // Register CS
    
    DWORD       OpenCount;

    DWORD       LastError;

    HANDLE      hProc;

    CEDEVICE_POWER_STATE    Dx;

} SHA_CONTEXT, *PSHA_CONTEXT;

//
// PDD functions
//
DWORD 
HW_Init(
    PSHA_CONTEXT pSHA
    );

DWORD 
HW_Deinit( 
    PSHA_CONTEXT pSHA
    );

DWORD 
HW_Open(
    PSHA_CONTEXT pSHA
    );

DWORD 
HW_Close(
    PSHA_CONTEXT pSHA
    );


BOOL
HW_PowerUp(
    PSHA_CONTEXT pSHA
   );

BOOL
HW_PowerDown(
    PSHA_CONTEXT pSHA
   );

BOOL
HW_PowerCapabilities(
    PSHA_CONTEXT pSHA,
    PPOWER_CAPABILITIES ppc
   );

BOOL
HW_PowerSet(
    PSHA_CONTEXT pSHA,
    PCEDEVICE_POWER_STATE pDx
   );

BOOL
HW_PowerGet(
    PSHA_CONTEXT pSHA,
    PCEDEVICE_POWER_STATE pDx
   );
#ifdef DEBUG
#define ZONE_ERR            DEBUGZONE(0)
#define ZONE_WRN            DEBUGZONE(1)
#define ZONE_INIT           DEBUGZONE(2)
#define ZONE_OPEN           DEBUGZONE(3)
#define ZONE_READ           DEBUGZONE(4)
#define ZONE_WRITE          DEBUGZONE(5)
#define ZONE_IOCTL          DEBUGZONE(6)
#define ZONE_IST            DEBUGZONE(7)
#define ZONE_POWER          DEBUGZONE(9)
//...
#define ZONE_TRACE          DEBUGZONE(15)
#endif
#endif _SHA_DRV_H_

