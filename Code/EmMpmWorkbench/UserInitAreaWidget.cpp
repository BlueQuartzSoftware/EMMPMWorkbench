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

#include "UserInitAreaWidget.h"
#include "UserInitArea.h"
#include <QtGui/QColorDialog>
#include <iostream>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaWidget::UserInitAreaWidget(QWidget *parent) :
QWidget(parent),
m_uia(NULL)
{
  setupUi(this);
  m_WidgetList <<  m_Class << m_LowerRight << m_Mu;
  m_WidgetList << m_Sigma << m_UpperLeft;
  setWidgetListEnabled(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaWidget::~UserInitAreaWidget()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::setWidgetListEnabled(bool b)
{
//  foreach (QWidget* w, m_WidgetList)
//  {
//    w->setEnabled(b);
//  }

  QObjectList objects = this->children();
  foreach(QObject* o, objects)
  {
    QWidget* w = qobject_cast<QWidget*>(o);
    if (w) {
      w->setEnabled(b);
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::on_m_LineWidth_valueChanged(double v)
{
  if (m_uia != NULL)
  {
    m_uia->setLineWidth(m_LineWidth->value());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::setUserInitArea(UserInitArea* uia)
{
  this->m_uia = uia;
  if (uia == NULL)
  {
    setWidgetListEnabled(false);

    m_Class->setText(QString("0"));

    m_Mu->setText(0);
    m_Sigma->setText(0);
    m_UpperLeft->setText("-1, -1");
    m_LowerRight->setText("-1, -1");
    m_LineWidth->setValue(1);

    return;
  }
  setWidgetListEnabled(true);

  m_Class->setText(QString::number(uia->getEmMpmClass()));

  m_Mu->setText(QString::number(uia->getMu()));
  m_Sigma->setText(QString::number(uia->getSigma()));

  m_LineWidth->blockSignals(true);
  m_LineWidth->setValue(uia->getLineWidth());
  m_LineWidth->blockSignals(false);

  unsigned int x, y;
  uia->getUpperLeft(x, y);
  QString s = QString::number(x).append(", ").append(QString::number(y));
  m_UpperLeft->setText(s);

  uia->getLowerRight(x, y);
  s = QString::number(x).append(", ").append(QString::number(y));
  m_LowerRight->setText(s);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitArea* UserInitAreaWidget::getUserInitArea()
{
  return m_uia;
}


