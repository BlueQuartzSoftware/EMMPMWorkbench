/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
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

#include "PerClassItemData.h"

#include <iostream>

#include "QtCore/QStringList"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PerClassItemData::PerClassItemData(int label, double gamma, int grayLevel,
                                   QString color, QObject* parent) :
QObject(parent),
m_Label(label),
m_Gamma(gamma),
m_GrayLevel(grayLevel),
m_Color(color)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PerClassItemData::~PerClassItemData()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::setLabel(int eClass)
{
  m_Label = eClass;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PerClassItemData::getLabel()
{
  return m_Label;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::setGamma(double sigma)
{
  m_Gamma = sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double PerClassItemData::getGamma()
{
  return m_Gamma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::setGrayLevel(int gray)
{
  m_GrayLevel = gray;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PerClassItemData::getGrayLevel()
{
  return m_GrayLevel;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::setColor(QString color)
{
  m_Color = color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PerClassItemData::getColor()
{
  return m_Color;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::writeSettings(QSettings &prefs)
{
  QString group("PerClassItemData-");
  group.append(QString::number(m_Label));

  prefs.beginGroup(group);

  prefs.setValue("Label", m_Label);
  prefs.setValue("Gamma", m_Gamma);
  prefs.setValue("GrayLevel", m_GrayLevel);
  prefs.setValue("Color", m_Color);

  prefs.endGroup();
}

#define printvar(var)\
  std::cout << "m_" << #var << ": " << m_##var << std::endl;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassItemData::readSettings(QSettings &prefs)
{
  bool ok = false;

  QString group("PerClassItemData-");
  group.append(QString::number(m_Label));
  prefs.beginGroup(group);
  QStringList keys = prefs.allKeys();

  QVariant v = prefs.value("Label");
  m_Label = v.toInt(&ok);

  v = prefs.value("Gamma");
  m_Gamma = v.toDouble(&ok);
  v = prefs.value("GrayLevel");
  m_GrayLevel = v.toInt(&ok);
  v = prefs.value("Color");
  m_Color = v.toString();
#if 0
  printvar(Class);
  printvar(Mu);
  printvar(Sigma);
  printvar(GrayLevel);
#endif
  prefs.endGroup();
}

