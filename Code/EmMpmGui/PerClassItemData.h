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

#ifndef _PER_CLASS_ITEM_DATA_H
#define _PER_CLASS_ITEM_DATA_H

#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtGui/QColor>


class PerClassItemData : public QObject
{
    Q_OBJECT;

public:
    /**
     * @brief
     * @param parent
     */
    PerClassItemData(int label, double gamma, double minStdDev,
                   QString color, int finalLabel, QObject* parent = 0);
    virtual ~PerClassItemData();

    void readSettings(QSettings &prefs);
    void writeSettings(QSettings &prefs);

 public slots:

    void setLabel(int eClass);
    int getLabel();

    void setGamma(double g);
    double getGamma();

    void setMinStdDev(double m);
    double getMinStdDev();

    void setFinalLabel(int gv);
    int getFinalLabel();

    void setColor(QString color);
    QString getColor();

 signals:

  void firePerClassItemDataUpdated(PerClassItemData*);
  void firePerClassItemDataAboutToDelete(PerClassItemData*);
  void firePerClassItemDataDeleted(PerClassItemData*);
  void firePerClassItemDataSelected(PerClassItemData*);

  protected:


  private:
    int m_Label;
    double m_Gamma;
    double m_MinStdDev;
    QString m_Color;
    int m_FinalLabel;

};


#endif // _PER_CLASS_ITEM_DATA_H
