/** @file
 *
 *  Copyright (c) 2025, Mario Bălănică <mariobalanica02@gmail.com>
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <VarStoreData.h>

#include "RK3588DxeFormSetGuid.h"
#include "UsbConfig.h"

#define USB2_ENABLE_VAR_NAME   L"RockchipUsb2Enable"
#define XHCI_ENABLE_VAR_NAME   L"RockchipXhciEnable"

VOID
EFIAPI
ApplyUsbVariables (
  VOID
  )
{
  /* nothing to do here */
}

VOID
EFIAPI
SetupUsbVariables (
  VOID
  )
{
  UINTN       Size;
  UINT8       Var8;
  EFI_STATUS  Status;

  Size = sizeof (UINT8);

  Status = gRT->GetVariable (
                  USB2_ENABLE_VAR_NAME,
                  &gRK3588DxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var8
                  );
  if (EFI_ERROR (Status)) {
    Var8  = USB2_INIT_ENABLED;
    Status = gRT->SetVariable (
                    USB2_ENABLE_VAR_NAME,
                    &gRK3588DxeFormSetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    Size,
                    &Var8
                    );
    ASSERT_EFI_ERROR (Status);
  }

  Status = gRT->GetVariable (
                  XHCI_ENABLE_VAR_NAME,
                  &gRK3588DxeFormSetGuid,
                  NULL,
                  &Size,
                  &Var8
                  );
  if (EFI_ERROR (Status)) {
    Var8  = XHCI_INIT_ENABLED;
    Status = gRT->SetVariable (
                    XHCI_ENABLE_VAR_NAME,
                    &gRK3588DxeFormSetGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    Size,
                    &Var8
                    );
    ASSERT_EFI_ERROR (Status);
  }
}
