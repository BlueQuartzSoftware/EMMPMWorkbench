/* ============================================================================
* Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of Michael A. Jackson nor the names of its contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "CrossCorrelationPlugin.h"
#include "CrossCorrelationInputUI.h"

Q_EXPORT_PLUGIN2(CrossCorrelationPlugin, CrossCorrelationPlugin);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CrossCorrelationPlugin::CrossCorrelationPlugin()
{
  m_InputWidget = new CrossCorrelationInputUI(NULL);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CrossCorrelationPlugin::~CrossCorrelationPlugin()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CrossCorrelationPlugin::getPluginName()
{
  return QString("Cross Correlation");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QWidget* CrossCorrelationPlugin::getInputWidget(QWidget* parent)
{
  m_InputWidget->setParent(parent);
  return m_InputWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int CrossCorrelationPlugin::startProcessing(QObject* caller)
{
  return m_InputWidget->processInputs(caller);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CrossCorrelationPlugin::getInputImage()
{
  return m_InputWidget->getCurrentImageFile();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CrossCorrelationPlugin::getProcessedImage()
{
  return m_InputWidget->getCurrentProcessedFile();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CrossCorrelationPlugin::writeSettings(QSettings* prefs)
{
  m_InputWidget->writeSettings(prefs);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CrossCorrelationPlugin::readSettings(QSettings* prefs)
{
  m_InputWidget->readSettings(prefs);
}