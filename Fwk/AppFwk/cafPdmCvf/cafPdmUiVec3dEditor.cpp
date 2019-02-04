//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2019- Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#include "cafPdmUiVec3dEditor.h"

#include "cafFactory.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiDefaultObjectEditor.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUniqueIdValidator.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QPushButton>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiVec3dEditor::createEditorWidget(QWidget* parent)
{
    QWidget* containerWidget = new QWidget(parent);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    containerWidget->setLayout(layout);

    m_lineEditX = new QLineEdit(containerWidget);
    m_lineEditX->setMaximumWidth(40);
    m_lineEditY = new QLineEdit(containerWidget);
    m_lineEditY->setMaximumWidth(40);
    m_lineEditZ = new QLineEdit(containerWidget);
    m_lineEditZ->setMaximumWidth(40);

    connect(m_lineEditX, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(m_lineEditY, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));
    connect(m_lineEditZ, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()));

    m_pickButton = new QPushButton(containerWidget);
    m_pickButton->setText("Pick");
    m_pickButton->setCheckable(true);

    layout->addWidget(m_lineEditX);
    layout->addWidget(m_lineEditY);
    layout->addWidget(m_lineEditZ);
    layout->addWidget(m_pickButton);

    connect(m_pickButton, SIGNAL(toggled(bool)), this, SLOT(pickButtonToggled(bool)));
    return containerWidget;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QWidget* PdmUiVec3dEditor::createLabelWidget(QWidget* parent)
{
    m_label = new QLabel(parent);
    return m_label;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::configureAndUpdateUi(const QString& uiConfigName)
{
    if (!m_label.isNull())
    {
        PdmUiFieldEditorHandle::updateLabelFromField(m_label, uiConfigName);
    }

    cvf::Vec3d vecValue;
    PdmValueFieldSpecialization<cvf::Vec3d>::setFromVariant(uiField()->uiValue(), vecValue);

    PdmUiVec3dEditorAttribute attributes;
    caf::PdmUiObjectHandle* uiObject = uiObj(uiField()->fieldHandle()->ownerObject());
    if (uiObject)
    {
        uiObject->editorAttribute(uiField()->fieldHandle(), uiConfigName, &attributes);
    }

    bool isReadOnly = uiField()->isUiReadOnly(uiConfigName);

    m_pickButton->setVisible(!attributes.action.isNull());
    m_pickButton->setEnabled(!isReadOnly);

    std::vector<QLineEdit*> lineEdits = lineEditWidgets();
    for (size_t i = 0; i < lineEdits.size(); ++i)
    {
        QLineEdit* lineEdit = lineEdits[i];
        lineEdit->setText(QString("%1").arg(vecValue[(int) i]));
        lineEdit->setReadOnly(isReadOnly);
        if (isReadOnly)
        {
            lineEdit->setStyleSheet("QLineEdit {"
                "color: #808080;"
                "background-color: #F0F0F0;}");
        }
        else
        {
            lineEdit->setStyleSheet("");
        }
        lineEdit->setToolTip(uiField()->uiToolTip(uiConfigName));
        lineEdit->setValidator(new QDoubleValidator(lineEdit));

        bool                     fromMenuOnly = true;
        QList<PdmOptionItemInfo> enumNames    = uiField()->valueOptions(&fromMenuOnly);
        CAF_ASSERT(fromMenuOnly); // Not supported

        if (!enumNames.isEmpty() && fromMenuOnly == true)
        {
            int enumValue = uiField()->uiValue().toInt();

            if (enumValue < enumNames.size() && enumValue > -1)
            {
                lineEdit->setText(enumNames[enumValue].optionUiText());
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QMargins caf::PdmUiVec3dEditor::calculateLabelContentMargins() const
{
    QSize editorSize = m_lineEditX->sizeHint();
    QSize labelSize  = m_label->sizeHint();
    int   heightDiff = editorSize.height() - labelSize.height();

    QMargins contentMargins = m_label->contentsMargins();
    if (heightDiff > 0)
    {
        contentMargins.setTop(contentMargins.top() + heightDiff / 2);
        contentMargins.setBottom(contentMargins.bottom() + heightDiff / 2);
    }
    return contentMargins;    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::slotEditingFinished()
{
    std::vector<QLineEdit*> lineEdits = lineEditWidgets();
    cvf::Vec3d vectorValue;
    for (size_t i = 0; i < lineEdits.size(); ++i)
    {
        vectorValue[(int) i] = lineEdits[i]->text().toDouble();
    }

    QVariant v = PdmValueFieldSpecialization<cvf::Vec3d>::convert(vectorValue);
    this->setValueToField(v);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::PdmUiVec3dEditor::pickButtonToggled(bool checked)
{
    for (QLineEdit* lineEdit : lineEditWidgets())
    {
        if (checked)
        {
            lineEdit->setStyleSheet("QLineEdit {"
                "color: #000000;"
                "background-color: #FFDCFF;}");
            m_pickButton->setText("Stop Picking");
        }
        else
        {
            lineEdit->setStyleSheet("");
            m_pickButton->setText("Pick");
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::PdmUiVec3dEditor::isMultipleFieldsWithSameKeywordSelected(PdmFieldHandle* editorField) const
{
    std::vector<PdmFieldHandle*> fieldsToUpdate;
    fieldsToUpdate.push_back(editorField);

    // For current selection, find all fields with same keyword
    std::vector<PdmUiItem*> items;
    SelectionManager::instance()->selectedItems(items, SelectionManager::FIRST_LEVEL);

    for (size_t i = 0; i < items.size(); i++)
    {
        PdmUiFieldHandle* uiField = dynamic_cast<PdmUiFieldHandle*>(items[i]);
        if (!uiField) continue;

        PdmFieldHandle* field = uiField->fieldHandle();
        if (field && field != editorField && field->keyword() == editorField->keyword())
        {
            fieldsToUpdate.push_back(field);
        }
    }

    if (fieldsToUpdate.size() > 1)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QLineEdit*> caf::PdmUiVec3dEditor::lineEditWidgets()
{
    return { m_lineEditX, m_lineEditY, m_lineEditZ };
}

// Define at this location to avoid duplicate symbol definitions in 'cafPdmUiDefaultObjectEditor.cpp' in a cotire build. The
// variables defined by the macro are prefixed by line numbers causing a crash if the macro is defined at the same line number.
CAF_PDM_UI_FIELD_EDITOR_SOURCE_INIT(PdmUiVec3dEditor);
