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

#ifndef AXISSETTINGSDIALOG_H_
#define AXISSETTINGSDIALOG_H_


#include <QtGui/QDialog>
#include <QtGui/QDoubleValidator>


//-- UIC generated Header
#include <ui_AxisSettingsDialog.h>

#define AXIS_PROPERTY_VALUE(w)\
  void set##w(double value) {\
    w->setText(QString::number(value)); }\
  double get##w() { \
    bool ok = false;\
    return w->text().toDouble(&ok); }


class AxisSettingsDialog : public QDialog, public Ui::AxisSettingsDialog
{
  Q_OBJECT;

  public:
    AxisSettingsDialog(QWidget *parent = 0);
    virtual ~AxisSettingsDialog();

    AXIS_PROPERTY_VALUE(XAxisMin)
    AXIS_PROPERTY_VALUE(XAxisMax)
    AXIS_PROPERTY_VALUE(YAxisMin)
    AXIS_PROPERTY_VALUE(YAxisMax)

    bool getShowCombinedGaussians() { return showCombinedGaussians->isChecked(); }
    void setShowCombinedGaussians(bool b) { showCombinedGaussians->setChecked(b); }

  protected slots:
    void on_XAxisMin_textEdited(const QString &s);
    void on_XAxisMax_textEdited(const QString &s);
    void on_YAxisMin_textEdited(const QString &s);
    void on_YAxisMax_textEdited(const QString &s);

  private:
    QDoubleValidator* m_XAxisMinValidator;
    QDoubleValidator* m_XAxisMaxValidator;
    QDoubleValidator* m_YAxisMinValidator;
    QDoubleValidator* m_YAxisMaxValidator;


    AxisSettingsDialog(const AxisSettingsDialog&); // Copy Constructor Not Implemented
    void operator=(const AxisSettingsDialog&); // Operator '=' Not Implemented
};

#endif /* AXISSETTINGSDIALOG_H_ */
