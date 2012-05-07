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

#include "ManualInitData.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ManualInitData::ManualInitData(int label, double mu, double sigma, double gamma,
                               int grayLevel, QObject* parent) :
QObject(parent),
m_Class(label),
m_Mu(mu),
m_Sigma(sigma),
m_Gamma(gamma),
m_GrayLevel(grayLevel)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ManualInitData::~ManualInitData()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::setEmMpmClass(int eClass)
{
  m_Class = eClass;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ManualInitData::getEmMpmClass()
{
  return m_Class;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::setMu(double mu)
{
  m_Mu = mu;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ManualInitData::getMu()
{
  return m_Mu;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::setSigma(double sigma)
{
  m_Sigma = sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ManualInitData::getSigma()
{
  return m_Sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::setGamma(double sigma)
{
  m_Gamma = sigma;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double ManualInitData::getGamma()
{
  return m_Gamma;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::setEmMpmGrayLevel(int gray)
{
  m_GrayLevel = gray;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ManualInitData::getEmMpmGrayLevel()
{
  return m_GrayLevel;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::writeSettings(QSettings &prefs)
{
  QString group("ManualInitData-");
  group.append(QString::number(m_Class));

  prefs.beginGroup(group);

  prefs.setValue("Class", m_Class);
  prefs.setValue("Mu", m_Mu);
  prefs.setValue("Sigma", m_Sigma);
  prefs.setValue("GrayLevel", m_GrayLevel);

  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitData::readSettings(QSettings &prefs)
{
  bool ok = false;

  QString group("ManualInitData-");
  group.append(QString::number(m_Class));
  prefs.beginGroup(group);

  m_Class = prefs.value("Class").toInt(&ok);
  m_Mu = prefs.value("Mu").toDouble(&ok);
  m_Sigma = prefs.value("Sigma").toDouble(&ok);
  m_GrayLevel = prefs.value("GrayLevel").toInt(&ok);
  prefs.endGroup();
}

