/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RigCompletionDataGridCell.h"
#include "cafCmdFeature.h"
#include <cafVecIjk.h>
#include <memory>
#include <limits>

class LgrInfo;
class RigMainGrid;
class RimEclipseCase;
class RimSimWellInView;
class RimWellPath;
class RicExportLgrUi;
class QFile;
class QTextStream;


//==================================================================================================
///
//==================================================================================================
class RicCreateTemporaryLgrFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    virtual bool isCommandEnabled() override;
    virtual void onActionTriggered(bool isChecked) override;
    virtual void setupActionLook(QAction* actionToSetup) override;

private:
    static void createLgr(LgrInfo& lgrInfo, RigMainGrid* mainGrid);

    static std::vector<RimWellPath*> selectedWellPaths();
    static bool containsAnyNonMainGridCells(const std::vector<RigCompletionDataGridCell>& cells);
};