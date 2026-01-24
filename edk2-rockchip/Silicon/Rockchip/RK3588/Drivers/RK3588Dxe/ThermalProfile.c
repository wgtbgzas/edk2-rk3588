/** @file
*
*  Copyright (c) 2025, Mario Bălănică <mariobalanica02@gmail.com>
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/ArmScmiClockProtocol.h>
#include <VarStoreData.h>

#include "RK3588DxeFormSetGuid.h"
#include "ThermalProfile.h"

#define SCMI_CLK_CPUL    0
#define SCMI_CLK_CPUB01  2
#define SCMI_CLK_CPUB23  3

#define FREQ_1_MHZ  1000000

typedef struct {
  UINT64    Hz;
  UINT32    Microvolts;
} OPERATING_PERFORMANCE_POINT;

STATIC CONST OPERATING_PERFORMANCE_POINT  mCPULOppTable[] = {
  { 408000000,  675000 },
  { 600000000,  675000 },
  { 816000000,  675000 },
  { 1008000000, 675000 },
  { 1200000000, 712500 },
  { 1416000000, 762500 },
  { 1608000000, 850000 },
  { 1800000000, 950000 }
};

STATIC CONST OPERATING_PERFORMANCE_POINT  mCPUBOppTable[] = {
  { 408000000,  675000  },
  { 600000000,  675000  },
  { 816000000,  675000  },
  { 1008000000, 675000  },
  { 1200000000, 675000  },
  { 1416000000, 725000  },
  { 1608000000, 762500  },
  { 1800000000, 850000  },
  { 2016000000, 925000  },
  { 2208000000, 987500  },
  { 2256000000, 1000000 },
  { 2304000000, 1000000 },
  { 2352000000, 1000000 },
  { 2400000000, 1000000 }
};

typedef struct {
  UINT32                               ClockId;
  CONST OPERATING_PERFORMANCE_POINT    *Opp;
  UINT32                               OppCount;
} SCMI_OPP_TABLE;

STATIC CONST SCMI_OPP_TABLE  mScmiOppTable[] = {
  { SCMI_CLK_CPUL,   mCPULOppTable, ARRAY_SIZE (mCPULOppTable) },
  { SCMI_CLK_CPUB01, mCPUBOppTable, ARRAY_SIZE (mCPUBOppTable) },
  { SCMI_CLK_CPUB23, mCPUBOppTable, ARRAY_SIZE (mCPUBOppTable) }
};

STATIC
EFI_STATUS
EFIAPI
ScmiSetClockRate (
  IN UINT32  ClockId,
  IN UINT64  Hz
  )
{
  EFI_STATUS           Status;
  SCMI_CLOCK_PROTOCOL  *ClockProtocol;
  EFI_GUID             ClockProtocolGuid = ARM_SCMI_CLOCK_PROTOCOL_GUID;

  Status = gBS->LocateProtocol (
                  &ClockProtocolGuid,
                  NULL,
                  (VOID **)&ClockProtocol
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return ClockProtocol->RateSet (ClockProtocol, ClockId, Hz);
}

STATIC
EFI_STATUS
EFIAPI
ScmiGetClockRate (
  IN  UINT32  ClockId,
  OUT UINT64  *Hz
  )
{
  EFI_STATUS           Status;
  SCMI_CLOCK_PROTOCOL  *ClockProtocol;
  EFI_GUID             ClockProtocolGuid = ARM_SCMI_CLOCK_PROTOCOL_GUID;

  Status = gBS->LocateProtocol (
                  &ClockProtocolGuid,
                  NULL,
                  (VOID **)&ClockProtocol
                  );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return ClockProtocol->RateGet (ClockProtocol, ClockId, Hz);
}

STATIC
UINT64
EFIAPI
GetOppAtOrBelow (
  IN CONST OPERATING_PERFORMANCE_POINT  *OppTable,
  IN UINT32                             OppCount,
  IN UINT32                             CapMhz
  )
{
  UINT64  TargetHz;

  if (CapMhz == 0) {
    return OppTable[OppCount - 1].Hz;
  }

  TargetHz = (UINT64)CapMhz * FREQ_1_MHZ;
  for (UINTN Index = 0; Index < OppCount; Index++) {
    if (TargetHz <= OppTable[Index].Hz) {
      return OppTable[Index].Hz;
    }
  }

  return OppTable[OppCount - 1].Hz;
}

STATIC
VOID
EFIAPI
CapClusterClockRate (
  IN CONST SCMI_OPP_TABLE  *ScmiOppTable,
  IN UINT32                CapMhz
  )
{
  EFI_STATUS  Status;
  UINT64      CurrentHz;
  UINT64      TargetHz;

  Status = ScmiGetClockRate (ScmiOppTable->ClockId, &CurrentHz);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: ScmiGetClockRate failed. Status=%r\n", __FUNCTION__, Status));
    return;
  }

  TargetHz = GetOppAtOrBelow (ScmiOppTable->Opp, ScmiOppTable->OppCount, CapMhz);
  if (TargetHz >= CurrentHz) {
    return;
  }

  Status = ScmiSetClockRate (ScmiOppTable->ClockId, TargetHz);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "%a: ScmiSetClockRate failed. Status=%r\n", __FUNCTION__, Status));
  }
}

VOID
EFIAPI
ApplyThermalProfileVariables (
  VOID
  )
{
  UINT32  Mode;
  UINT32  CapCpulMhz;
  UINT32  CapCpubMhz;

  Mode = PcdGet32 (PcdThermalProfileMode);
  switch (Mode) {
    case THERMAL_PROFILE_MEDIUM:
      CapCpulMhz = 1416;
      CapCpubMhz = 1800;
      break;
    case THERMAL_PROFILE_SLOW:
      CapCpulMhz = 1008;
      CapCpubMhz = 1416;
      break;
    case THERMAL_PROFILE_CUSTOM:
      CapCpulMhz = PcdGet32 (PcdThermalProfileCustomCpulMhz);
      CapCpubMhz = PcdGet32 (PcdThermalProfileCustomCpubMhz);
      break;
    case THERMAL_PROFILE_MAX:
    default:
      return;
  }

  DEBUG ((DEBUG_INFO, "Thermal profile: mode=%u CPUL cap=%u MHz CPUB cap=%u MHz\n", Mode, CapCpulMhz, CapCpubMhz));

  CapClusterClockRate (&mScmiOppTable[0], CapCpulMhz);
  CapClusterClockRate (&mScmiOppTable[1], CapCpubMhz);
  CapClusterClockRate (&mScmiOppTable[2], CapCpubMhz);
}

VOID
EFIAPI
SetupThermalProfileVariables (
  VOID
  )
{
  UINTN       Size;
  UINT32      Var32;
  EFI_STATUS  Status;

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"ThermalProfileMode",
                  &gRK3588DxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdThermalProfileMode, PcdGet32 (PcdThermalProfileModeDefault));
    ASSERT_EFI_ERROR (Status);
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"ThermalProfileCustomCpulMhz",
                  &gRK3588DxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdThermalProfileCustomCpulMhz, PcdGet32 (PcdThermalProfileCustomCpulMhzDefault));
    ASSERT_EFI_ERROR (Status);
  }

  Size = sizeof (UINT32);
  Status = gRT->GetVariable (
                  L"ThermalProfileCustomCpubMhz",
                  &gRK3588DxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var32
                  );
  if (EFI_ERROR (Status)) {
    Status = PcdSet32S (PcdThermalProfileCustomCpubMhz, PcdGet32 (PcdThermalProfileCustomCpubMhzDefault));
    ASSERT_EFI_ERROR (Status);
  }
}
