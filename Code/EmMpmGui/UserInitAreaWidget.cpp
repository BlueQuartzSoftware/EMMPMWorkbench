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
  m_WidgetList << m_Gamma << m_Class << m_GrayLevel << m_LowerRight << m_Mu;
  m_WidgetList << m_Sigma << m_UpperLeft << colorButton;
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
void UserInitAreaWidget::on_m_Gamma_valueChanged(double v)
{
  if (m_uia != NULL)
  {
    m_uia->setGamma(m_Gamma->value());
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::on_m_Class_valueChanged(int v)
{
  if (m_uia != NULL)
  {
    m_uia->setEmMpmClass(m_Class->value());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::on_m_GrayLevel_valueChanged(int v)
{
    if (m_uia != NULL)
    {
      m_uia->setEmMpmGrayLevel(m_GrayLevel->value());
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
    m_GrayLevel->setValue(0);
    m_Class->setValue(0);
    bool ok = false;
    m_Mu->setText(0);
    m_Sigma->setText(0);
    m_Gamma->setValue(0);
    m_UpperLeft->setText("-1, -1");
    m_LowerRight->setText("-1, -1");
    colorButton->setStyleSheet(QString(""));
    m_LineWidth->setValue(1);
    return;
  }
  setWidgetListEnabled(true);
  m_GrayLevel->setValue(uia->getEmMpmGrayLevel());
  m_Class->setValue(uia->getEmMpmClass());
  bool ok = false;
  m_Mu->setText(QString::number(uia->getMu()));
  m_Sigma->setText(QString::number(uia->getSigma()));
  m_Gamma->setValue(uia->getGamma());
  m_LineWidth->setValue(uia->getLineWidth());

  QColor c = m_uia->getColor();
  QString cssColor = "border: 1px solid #101010; background-color: ";
  cssColor.append(c.name());
  colorButton->setStyleSheet(cssColor);

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


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaWidget::on_colorButton_clicked()
{
  if (NULL != m_uia)
  {
    QColorDialog d;
    d.setCurrentColor(m_uia->getColor());
    int r = d.exec();
    if (r > 0)
    {
      QColor c = d.selectedColor();
      c.setAlpha(UIA::Alpha);
      m_uia->setColor(c);

      QString cssColor = "border: 1px solid #101010; background-color: ";
      cssColor.append(c.name());
      colorButton->setStyleSheet(cssColor);
    }
  }
}

