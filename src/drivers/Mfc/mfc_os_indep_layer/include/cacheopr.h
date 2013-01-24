//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// Copyright (c) Samsung Electronics. Co. LTD.  All rights reserved.
//
/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

*/


#ifndef __SAMSUNG_SYSLSI_APDEV_MFC_CACHE_OPR_H__
#define __SAMSUNG_SYSLSI_APDEV_MFC_CACHE_OPR_H__

// JYSHIN 20080723
#if __cplusplus
extern "C" {
#endif

void CleanInvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);
void CleanCacheRange(PBYTE StartAddress, PBYTE EndAddress);
void InvalidateCacheRange(PBYTE StartAddress, PBYTE EndAddress);

#if __cplusplus
}
#endif

#endif
