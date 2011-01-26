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

#include "UserInitAreaDialog.h"
#include "UserInitArea.h"
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaDialog::UserInitAreaDialog(UserInitArea* uia, QWidget *parent) :
QDialog(parent),
m_uia(uia)
{
  setupUi(this);

  m_GrayLevel->setValue(uia->getEmMpmGrayLevel());
  m_Class->setValue(uia->getEmMpmClass());
  bool ok = false;
  m_Mu->setText(QString::number(uia->getMu()));
  m_Sigma->setText(QString::number(uia->getSigma()));
  m_Gamma->setValue(uia->getGamma());


  connect(m_GrayLevel, SIGNAL(valueChanged(int)),
          uia, SLOT(setEmMpmGrayLevel(int)), Qt::QueuedConnection);
  connect(m_Class, SIGNAL(valueChanged(int)),
          uia, SLOT(setEmMpmClass(int)), Qt::QueuedConnection);
  connect(m_Gamma, SIGNAL(valueChanged(double)),
          uia, SLOT(setGamma(double)), Qt::QueuedConnection);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaDialog::~UserInitAreaDialog()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//QColor UserInitAreaDialog::getSelectedColor()
//{
//  QStringList colorNames = QColor::colorNames();
//  return QColor(colorNames[colorPicker->currentIndex()]);
//}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaDialog::on_colorPicker_currentIndexChanged(int index)
{
  QStringList colorNames = QColor::colorNames();
  QColor c = QColor(colorNames[colorPicker->currentIndex()]);
  c.setAlpha(UIA::Alpha);
  m_uia->setColor( c );
}
