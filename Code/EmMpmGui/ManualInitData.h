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

#ifndef _MANUAL_INIT_H_
#define _MANUAL_INIT_H_

#include <QtCore/QObject>
#include <QtCore/QSettings>



class ManualInitData : public QObject
{
    Q_OBJECT;

public:
    /**
     * @brief
     * @param label
     * @param mu
     * @param sigma
     * @param gamma
     * @param grayLevel
     * @param parent
     */
    ManualInitData(int label, double mu, double sigma, double gamma,
                   int grayLevel, QObject* parent = 0);
    virtual ~ManualInitData();

    void readSettings(QSettings &prefs);
    void writeSettings(QSettings &prefs);

 public slots:

    void setEmMpmClass(int eClass);
    int getEmMpmClass();

    void setMu(double mu);
    double getMu();

    void setSigma(double sigma);
    double getSigma();

    void setGamma(double g);
    double getGamma();

    void setEmMpmGrayLevel(int gray);
    int getEmMpmGrayLevel();


 signals:

  void fireManualInitDataUpdated(ManualInitData*);
  void fireManualInitDataAboutToDelete(ManualInitData*);
  void fireManualInitDataDeleted(ManualInitData*);
  void fireManualInitDataSelected(ManualInitData*);

  protected:


  private:
    int m_Class;
    double m_Mu;
    double m_Sigma;
    double m_Gamma;
    int m_GrayLevel;
};


#endif // _MANUAL_INIT_H_
