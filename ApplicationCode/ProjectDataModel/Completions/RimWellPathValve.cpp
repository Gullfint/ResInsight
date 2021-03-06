/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimWellPathValve.h"

#include "RiaDefines.h"
#include "RiaColorTables.h"
#include "RiaEclipseUnitTools.h"

#include "Riu3DMainWindowTools.h"

#include "RigWellPath.h"

#include "RimMultipleValveLocations.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimValveTemplate.h"
#include "RimWellPath.h"

#include "CompletionCommands/RicNewValveTemplateFeature.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiToolButtonEditor.h"

CAF_PDM_SOURCE_INIT(RimWellPathValve, "WellPathValve");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::RimWellPathValve()
{
    CAF_PDM_InitObject("WellPathValve", ":/ICDValve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_valveTemplate, "ValveTemplate", "Valve Template", "", "", "");
    CAF_PDM_InitField(&m_measuredDepth, "StartMeasuredDepth", 0.0, "Start MD", "", "", "");    
    CAF_PDM_InitFieldNoDefault(&m_multipleValveLocations, "ValveLocations", "Valve Locations", "", "", "");
    CAF_PDM_InitField(&m_createOrEditValveTemplate, "CreateTemplate", false, "New", "", "", "");
    
    m_measuredDepth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());
    m_multipleValveLocations = new RimMultipleValveLocations;
    m_multipleValveLocations.uiCapability()->setUiTreeHidden(true);
    m_multipleValveLocations.uiCapability()->setUiTreeChildrenHidden(true);
    m_createOrEditValveTemplate.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::HIDDEN);
    m_createOrEditValveTemplate.uiCapability()->setUiEditorTypeName(caf::PdmUiToolButtonEditor::uiEditorTypeName());
    nameField()->uiCapability()->setUiReadOnly(true);

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathValve::~RimWellPathValve()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::perforationIntervalUpdated()
{
    if (componentType() == RiaDefines::ICV)
    {
        const RimPerforationInterval* perfInterval = nullptr;
        this->firstAncestorOrThisOfType(perfInterval);
        double startMD = perfInterval->startMD();
        double endMD   = perfInterval->endMD();
        m_measuredDepth = cvf::Math::clamp(m_measuredDepth(), std::min(startMD, endMD), std::max(startMD, endMD));
    }
    else if (componentType() == RiaDefines::ICD || componentType() == RiaDefines::AICD)
    {
        m_multipleValveLocations->perforationIntervalUpdated();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::setMeasuredDepthAndCount(double startMD, double spacing, int valveCount)
{
    m_measuredDepth = startMD;
    double endMD = startMD + spacing * valveCount;
    m_multipleValveLocations->initFields(RimMultipleValveLocations::VALVE_COUNT, startMD, endMD, spacing, valveCount, {});
    m_multipleValveLocations->computeRangesAndLocations();
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::multipleValveGeometryUpdated()
{
    if (m_multipleValveLocations->valveLocations().empty()) return;

    m_measuredDepth = m_multipleValveLocations->valveLocations().front();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimWellPathValve::valveLocations() const
{
    std::vector<double> valveDepths;
    if (componentType() == RiaDefines::ICV)
    {
        valveDepths.push_back(m_measuredDepth);
    }
    else if (componentType() == RiaDefines::ICD || componentType() == RiaDefines::AICD)
    {
        valveDepths = m_multipleValveLocations->valveLocations();
    }
    return valveDepths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::orificeDiameter(RiaEclipseUnitTools::UnitSystem unitSystem) const
{
    if (m_valveTemplate())
    {
        double templateDiameter = m_valveTemplate()->orificeDiameter();
        return convertOrificeDiameter(templateDiameter, m_valveTemplate()->templateUnits(), unitSystem);
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::flowCoefficient() const
{
    if (m_valveTemplate())
    {
        double templateCoefficient = m_valveTemplate()->orificeDiameter();
        return templateCoefficient;
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate* RimWellPathValve::valveTemplate() const
{
    return m_valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::setValveTemplate(RimValveTemplate* valveTemplate)
{
    m_valveTemplate = valveTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::applyValveLabelAndIcon()
{
    if (componentType() == RiaDefines::ICV)
    {
        this->setUiIcon(QIcon(":/ICVValve16x16.png"));
        QString fullName = QString("%1: %2").arg(componentLabel()).arg(m_measuredDepth());
        this->setName(fullName);
    }
    else if (componentType() == RiaDefines::ICD)
    {
        this->setUiIcon(QIcon(":/ICDValve16x16.png"));
        QString fullName = QString("%1 %2: %3 - %4")
                               .arg(m_multipleValveLocations->valveLocations().size())
                               .arg(componentLabel())
                               .arg(m_multipleValveLocations->rangeStart())
                               .arg(m_multipleValveLocations->rangeEnd());
        this->setName(fullName);
    }
    else if (componentType() == RiaDefines::AICD)
    {
        this->setUiIcon(QIcon(":/AICDValve16x16.png"));
        QString fullName = QString("%1 %2: %3 - %4")
                               .arg(m_multipleValveLocations->valveLocations().size())
                               .arg(componentLabel())
                               .arg(m_multipleValveLocations->rangeStart())
                               .arg(m_multipleValveLocations->rangeEnd());
        this->setName(fullName);
    }
    else
    {
        this->setName("Unspecified Valve");
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathAicdParameters* RimWellPathValve::aicdParameters() const
{
    if (m_valveTemplate())
    {
        return m_valveTemplate()->aicdParameters();
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::convertOrificeDiameter(double                          orificeDiameterWellPathUnits,
                                                RiaEclipseUnitTools::UnitSystem wellPathUnits,
                                                RiaEclipseUnitTools::UnitSystem unitSystem)
{
    if (unitSystem == RiaEclipseUnitTools::UNITS_METRIC)
    {
        if (wellPathUnits == RiaEclipseUnitTools::UNITS_FIELD)
        {
            return RiaEclipseUnitTools::inchToMeter(orificeDiameterWellPathUnits);
        }
        else
        {
            return RiaEclipseUnitTools::mmToMeter(orificeDiameterWellPathUnits);
        }
    }
    else if (unitSystem == RiaEclipseUnitTools::UNITS_FIELD)
    {
        if (wellPathUnits == RiaEclipseUnitTools::UNITS_METRIC)
        {
            return RiaEclipseUnitTools::meterToFeet(RiaEclipseUnitTools::mmToMeter(orificeDiameterWellPathUnits));
        }
        else
        {
            return RiaEclipseUnitTools::inchToFeet(orificeDiameterWellPathUnits);
        }
    }
    CVF_ASSERT(false);
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<double, double>> RimWellPathValve::valveSegments() const
{
    RimPerforationInterval* perforationInterval = nullptr;
    this->firstAncestorOrThisOfType(perforationInterval);

    double startMD = perforationInterval->startMD();
    double endMD   = perforationInterval->endMD();
    std::vector<double> valveMDs = valveLocations();

    std::vector<std::pair<double, double>> segments;
    segments.reserve(valveMDs.size());

    for (size_t i = 0; i < valveMDs.size(); ++i)
    {
        double segmentStart = startMD;
        double segmentEnd = endMD;
        if (i > 0)
        {
            segmentStart = 0.5 * (valveMDs[i - 1] + valveMDs[i]);
        }
        if (i < valveMDs.size() - 1u)
        {
            segmentEnd = 0.5 * (valveMDs[i] + valveMDs[i + 1]);
        }
        segments.push_back(std::make_pair(segmentStart, segmentEnd));
    }
    return segments;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathValve::isEnabled() const
{
    RimPerforationInterval* perforationInterval = nullptr;
    this->firstAncestorOrThisOfType(perforationInterval);
    return perforationInterval->isEnabled() && isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimWellPathValve::componentType() const
{
    if (m_valveTemplate())
    {
        return m_valveTemplate()->type();
    }
    return RiaDefines::UNDEFINED_COMPONENT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentLabel() const
{
    if (componentType() == RiaDefines::ICD || componentType() == RiaDefines::AICD)
    {
        if (m_multipleValveLocations->valveLocations().size() > 1)
        {
            return "Valves";
        }
    }
    return "Valve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathValve::componentTypeLabel() const
{
    return "Valve";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPathValve::defaultComponentColor() const
{
    return RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::startMD() const
{
    if (componentType() == RiaDefines::ICV)
    {
        return m_measuredDepth;
    }
    else if (m_multipleValveLocations()->valveLocations().empty())
    {
        return m_multipleValveLocations->rangeStart();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().front();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathValve::endMD() const
{
    if (componentType() == RiaDefines::ICV)
    {
        return m_measuredDepth + 0.5;
    }
    else if (m_multipleValveLocations()->valveLocations().empty())
    {
        return m_multipleValveLocations->rangeEnd();
    }
    else
    {
        return m_multipleValveLocations()->valveLocations().back();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::templateUpdated()
{
    applyValveLabelAndIcon();

    RimPerforationInterval* perfInterval;
    this->firstAncestorOrThisOfTypeAsserted(perfInterval);
    perfInterval->updateAllReferringTracks();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathValve::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* project = nullptr;
    this->firstAncestorOrThisOfTypeAsserted(project);
    std::vector<RimValveTemplate*> allTemplates = project->allValveTemplates();
    for (RimValveTemplate* valveTemplate : allTemplates)
    {
        options.push_back(caf::PdmOptionItemInfo(valveTemplate->name(), valveTemplate));
    }
    
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_valveTemplate)
    {
        applyValveLabelAndIcon();
        this->updateConnectedEditors();
    }
    else if (changedField == &m_createOrEditValveTemplate)
    {
        m_createOrEditValveTemplate = false;
        if (m_valveTemplate == nullptr)
        {
            RicNewValveTemplateFeature::createNewValveTemplateForValveAndUpdate(this);
        }
        else
        {
            Riu3DMainWindowTools::selectAsCurrentItem(m_valveTemplate());
        }
    }

    RimPerforationInterval* perfInterval;
    this->firstAncestorOrThisOfTypeAsserted(perfInterval);
    perfInterval->updateAllReferringTracks();

    RimProject* proj;
    this->firstAncestorOrThisOfTypeAsserted(proj);
    proj->reloadCompletionTypeResultsInAllViews();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.skipRemainingFields(true);

    uiOrdering.add(&m_valveTemplate, { true, 2, 1 });

    {
        if (m_valveTemplate() == nullptr)
        {
            m_createOrEditValveTemplate.uiCapability()->setUiName("New");
        }
        else
        {
            m_createOrEditValveTemplate.uiCapability()->setUiName("Edit");
        }
        uiOrdering.add(&m_createOrEditValveTemplate, { false, 1, 0 });
    }

    if (componentType() == RiaDefines::ICV || componentType() == RiaDefines::ICD)
    {        
        if (componentType() == RiaDefines::ICV)
        {
            RimWellPath* wellPath;
            firstAncestorOrThisOfType(wellPath);
            if (wellPath)
            {
                if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_METRIC)
                {
                   m_measuredDepth.uiCapability()->setUiName("Measured Depth [m]");                
                }
                else if (wellPath->unitSystem() == RiaEclipseUnitTools::UNITS_FIELD)
                {
                    m_measuredDepth.uiCapability()->setUiName("Measured Depth [ft]");
                }
            }
            uiOrdering.add(&m_measuredDepth, { true, 3, 1 });
        }
    }

    if (componentType() == RiaDefines::ICD || componentType() == RiaDefines::AICD)
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Multiple Valve Locations");
        m_multipleValveLocations->uiOrdering(uiConfigName, *group);
    }

    if (m_valveTemplate() != nullptr)
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Parameters from Template");
        m_valveTemplate->uiOrdering("InsideValve", *group);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
      if (field == &m_measuredDepth)
      {
          caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

          if (myAttr)
          {
              double minimumValue = 0.0, maximumValue = 0.0;

              RimPerforationInterval* perforationInterval = nullptr;
              this->firstAncestorOrThisOfType(perforationInterval);

              if (perforationInterval)
              {
                  minimumValue = perforationInterval->startMD();
                  maximumValue = perforationInterval->endMD();
              }
              else
              {
                  RimWellPath* rimWellPath = nullptr;
                  this->firstAncestorOrThisOfTypeAsserted(rimWellPath);
                  RigWellPath* wellPathGeo = rimWellPath->wellPathGeometry();
                  if (!wellPathGeo) return;

                  if (wellPathGeo->m_measuredDepths.size() > 2)
                  {
                      minimumValue = wellPathGeo->measureDepths().front();
                      maximumValue = wellPathGeo->measureDepths().back();
                  }
              }
              myAttr->m_minimum = minimumValue;
              myAttr->m_maximum = maximumValue;
          }
      }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathValve::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    applyValveLabelAndIcon();
}
