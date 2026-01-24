/** @file
 *
 *  Copyright (c) 2025, Mario Bălănică <mariobalanica02@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __RK3588DXE_THERMAL_PROFILE_H__
#define __RK3588DXE_THERMAL_PROFILE_H__

#define THERMAL_PROFILE_CPUL_MIN_MHZ  408
#define THERMAL_PROFILE_CPUL_MAX_MHZ  1800
#define THERMAL_PROFILE_CPUB_MIN_MHZ  408
#define THERMAL_PROFILE_CPUB_MAX_MHZ  2400

//
// Don't declare these in the VFR file.
//
#ifndef VFR_FILE_INCLUDE
VOID
EFIAPI
ApplyThermalProfileVariables (
  VOID
  );

VOID
EFIAPI
SetupThermalProfileVariables (
  VOID
  );
#endif

#endif // __RK3588DXE_THERMAL_PROFILE_H__
