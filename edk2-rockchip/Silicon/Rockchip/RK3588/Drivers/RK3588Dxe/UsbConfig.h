/** @file
 *
 *  Copyright (c) 2025, Mario Bălănică <mariobalanica02@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __RK3588DXE_USB_CONFIG_H__
#define __RK3588DXE_USB_CONFIG_H__

//
// Don't declare these in the VFR file.
//
#ifndef VFR_FILE_INCLUDE
VOID
EFIAPI
ApplyUsbVariables (
  VOID
  );

VOID
EFIAPI
SetupUsbVariables (
  VOID
  );

#endif // VFR_FILE_INCLUDE

#endif // __RK3588DXE_USB_CONFIG_H__
