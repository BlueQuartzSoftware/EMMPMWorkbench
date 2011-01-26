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

#include <emmpm/public/EMMPM_Structures.h>

//-- UIC generated Header
#include <ui_EmMpmGui.h>


class UserInitArea;
class UserInitAreaTableModel;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;
class QwtPlotGrid;
class QwtPlotCurve;
class QwtPlotMarker;
class EMMPMTask;


#include "IPHelper/plugins/QImageProcessingInputFrame.h"

class EmMpmGui :  public QMainWindow, private Ui::EmMpmGui
{

    Q_OBJECT;


  public:
    EmMpmGui(QWidget *parent = 0);
    virtual ~EmMpmGui();
    void initWithFile(const QString imageFile, QString mountImage);


    int processInputs(QObject* parentGUI);

    /**
     * @brief Reads the Preferences from the users pref file
     */
    void readSettings();

    /**
     * @brief Writes the preferences to the users pref file
     */
    void writeSettings();

    typedef QPair<QString, QString>        InputOutputFilePair;
    typedef QList<InputOutputFilePair>     InputOutputFilePairList;

    MXA_INSTANCE_PROPERTY(QString, CurrentImageFile)
    MXA_INSTANCE_PROPERTY(QString, CurrentProcessedFile)
    MXA_INSTANCE_PROPERTY(QSortFilterProxyModel*, ProxyModel)
    MXA_INSTANCE_PROPERTY(QList<QWidget* >, WidgetList)
    MXA_INSTANCE_PROPERTY(bool, OutputExistsCheck)
    MXA_INSTANCE_PROPERTY(ProcessQueueController*, QueueController)
    MXA_INSTANCE_PROPERTY(ProcessQueueDialog*, QueueDialog)
    MXA_INSTANCE_PROPERTY(QString, OpenDialogLastDirectory)
    MXA_INSTANCE_PROPERTY(InputOutputFilePairList, InputOutputFilePairList)

    char* copyStringToNewBuffer(const QString &fname);
    void copyGrayValues( EMMPM_Data* inputs);
    void copyInitCoords( EMMPM_Data* inputs);
    void copyIntializationValues(EMMPM_Data* inputs);
    void copyGammaValues(EMMPM_Data* inputs);


    signals:
    void cancelTask();
    void cancelProcessQueue();

  public slots:

  // Manual hookup slots to get signals from the graphics view
    void baseImageFileLoaded(const QString &filename);
    void overlayImageFileLoaded(const QString &filename);
    void userInitAreaAdded(UserInitArea* uia);
    void deleteUserInitArea(UserInitArea* uia);
    void userInitAreaUpdated(UserInitArea* uia);
    void on_fitToWindow_clicked();


  protected slots:
  //Manual Hookup Menu Actions
    void on_actionOpenBaseImage_triggered(); // Open a Data File
    void on_actionOpenOverlayImage_triggered();
    void on_actionSaveCanvas_triggered();
    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionClose_triggered();



    /* slots for the buttons in the GUI */
    void on_processBtn_clicked();

    /* slots for the buttons in the GUI */
    void on_compositeModeCB_currentIndexChanged();
    void on_imageDisplayCombo_currentIndexChanged();

    /**
     * @brief Qt Slot that fires in response to a click on a "Recent File' Menu entry.
     */
    void openRecentBaseImageFile();

    /**
     * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
     * should be connected to the Signal QRecentFileList->fileListChanged
     * @param file The newly added file.
     */
    void updateBaseRecentFileList(const QString &file);

    // -----------------------------------------------------------------------------
    //  Input Tab Widgets
    // -----------------------------------------------------------------------------
    void on_inputImageFilePathBtn_clicked();
    void on_outputImageButton_clicked();

    void on_processFolder_stateChanged(int state  );
    void on_sourceDirectoryBtn_clicked();
    void on_outputDirectoryBtn_clicked();

    void on_inputImageFilePath_textChanged(const QString &string);
    void on_outputImageFile_textChanged(const QString & text);
    void on_sourceDirectoryLE_textChanged(const QString & text);
    void on_outputDirectoryLE_textChanged(const QString & text);

    void on_filterPatternLineEdit_textChanged();

    void on_outputPrefix_textChanged();
    void on_outputSuffix_textChanged();
    void on_outputImageType_currentIndexChanged(int index);

    // Over rides from Parent Class
    /* Slots to receive events from the ProcessQueueController */
    void queueControllerFinished();
    // These slots get called when the plugin starts and finishes processing
    void processingStarted();
    void processingFinished();
  protected:

    void addProcess(EMMPMTask* name);
    /**
    * @brief Implements the CloseEvent to Quit the application and write settings
    * to the preference file
    */
   void closeEvent(QCloseEvent *event);


   /**
    * @brief Initializes some of the GUI elements with selections or other GUI related items
    */
   void setupGui();

   /**
    * @brief Checks the currently open file for changes that need to be saved
    * @return
    */
   qint32 checkDirtyDocument();

   /**
    * @brief Enables or Disables all the widgets in a list
    * @param b
    */
   void setWidgetListEnabled(bool b);

   /**
    * @brief Verifies that a path exists on the file system.
    * @param outFilePath The file path to check
    * @param lineEdit The QLineEdit object to modify visuals of (Usually by placing a red line around the QLineEdit widget)
    */
   bool verifyPathExists(QString outFilePath, QLineEdit* lineEdit);

   /**
    * @brief Verifies that a parent path exists on the file system.
    * @param outFilePath The parent file path to check
    * @param lineEdit The QLineEdit object to modify visuals of (Usually by placing a red line around the QLineEdit widget)
    */
   bool verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit);


   void populateFileTable(QLineEdit* sourceDirectoryLE, QListView *fileListView);

    /**
     * @brief Opens an Image file
     * @param imageFile The path to the image file to open.
     */
    void openBaseImageFile(QString imageFile);

    void openOverlayImage(QString mountImage);

    qint32 initImageViews();

    void plotImageHistogram();

    QStringList generateInputFileList();

  private:

  QList<UserInitArea*>        m_UserInitAreas;
  UserInitAreaTableModel*     m_UserInitAreaTableModel;


  QwtPlotZoomer* m_zoomer;
  QwtPlotPicker* m_picker;
  QwtPlotPanner* m_panner;
  QwtPlotGrid*   m_grid;

  QwtPlotCurve*  m_histogram;
//  QList<QwtPlotCurve*>  m_UIAGaussians;
  QList<QwtPlotMarker*>  m_UIAMarkers;

  EmMpmGui(const EmMpmGui&); // Copy Constructor Not Implemented
  void operator=(const EmMpmGui&); // Operator '=' Not Implemented
};

#endif /* EMMPMGUI_H_ */
