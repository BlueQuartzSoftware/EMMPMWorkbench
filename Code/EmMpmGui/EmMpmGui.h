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

#ifndef EMMPMGUI_H_
#define EMMPMGUI_H_

//-- Qt Includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QCloseEvent>
#include <QtGui/QMainWindow>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsScene>



//-- UIC generated Header
#include <ui_EmMpmGui.h>
class UserInitArea;
class UserInitAreaTableModel;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtPlotGrid;
class QwtPlotCurve;

//class HistogramItem;

class EmMpmGui : public QMainWindow, private Ui::EmMpmGui
{

    Q_OBJECT;

  public:
    EmMpmGui(QWidget *parent = 0);
    virtual ~EmMpmGui();



  public slots:

    void imageFileLoaded(const QString &filename);
    void userInitAreaAdded(UserInitArea* uia);
    void deleteUserInitArea(UserInitArea* uia);
    void userInitAreaUpdated(UserInitArea* uia);
    void on_fitToWindow_clicked();



  private slots:

  void on_actionClose_triggered();

  protected:
    /**
     * @brief Implements the CloseEvent to Quit the application and write settings
     * to the preference file
     */
    void closeEvent(QCloseEvent *event);

  /**
   * @brief Initializes some of the GUI elements with selections or other GUI related items
   */
  void setupGui();

  void plotImageHistogram();



  private:
  QString                     m_OpenDialogLastDirectory;
  QString                     m_CurrentImageFile;

  QList<UserInitArea*>                 m_UserInitAreas;
  UserInitAreaTableModel*         m_UserInitAreaTableModel;


  QwtPlotZoomer* m_zoomer;
  QwtPlotPicker* m_picker;
  QwtPlotPanner* m_panner;
  QwtPlotGrid*   m_grid;

  QwtPlotCurve*  m_histogram;
  QList<QwtPlotCurve*>  m_UIAGaussians;

  EmMpmGui(const EmMpmGui&); // Copy Constructor Not Implemented
  void operator=(const EmMpmGui&); // Operator '=' Not Implemented
};

#endif /* EMMPMGUI_H_ */
