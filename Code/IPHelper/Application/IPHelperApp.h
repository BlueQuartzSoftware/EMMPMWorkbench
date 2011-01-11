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

#ifndef _IPHELPER_MAINWINDOW_H_
#define _IPHELPER_MAINWINDOW_H_

//-- Qt Includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtGui/QWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QResizeEvent>
#include <QtGui/QSortFilterProxyModel>

//-- UIC generated Header
#include <ui_IPHelperApp.h>


#include "AIM/Common/AIMImage.h"


class ImageGraphicsDelegate;
class ProcessQueueDialog;
class ProcessQueueController;
class QImageProcessingInterface;

/**
 * @class IPHelperApp IPHelperApp.h IPHelper/Application/IPHelperApp.h
 * @brief
 * @author Michael A. Jackson for BlueQuartz Software
 * @date Jan 10, 2011
 * @version 1.0
 */
class IPHelperApp : public QMainWindow, private Ui::IPHelperApp
{
  Q_OBJECT;

  public:
    IPHelperApp(QWidget *parent = 0);
    virtual ~IPHelperApp();
    void initWithFile(const QString imageFile, QString mountImage);

    MXA_INSTANCE_PROPERTY(AIMImage::Pointer, OriginalImage)
    MXA_INSTANCE_PROPERTY(AIMImage::Pointer, ProcessedImage)

    MXA_INSTANCE_PROPERTY(QString, CurrentImageFile)
    MXA_INSTANCE_PROPERTY(QString, CurrentProcessedFile)

    qint32 saveProcessedImage();

  signals:
    void cancelTask();
    void cancelProcessQueue();
    void parentResized();

  protected slots:
  //Manual Hookup Menu Actions
    void on_actionOpen_triggered(); // Open a Data File
    void on_actionOpen_Processed_Image_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionClose_triggered();
    void on_actionExit_triggered();

    /* slots for the buttons in the GUI */
    void on_processBtn_clicked();
    void on_aboutBtn_clicked();

    void on_compositeWithOriginal_stateChanged(int state);
    void on_modeComboBox_currentIndexChanged();

    void on_originalImageFileList_currentIndexChanged(int index);
    void on_processedImageFileList_currentIndexChanged(int index);

    void on_fixedZoomCombo_currentIndexChanged(int index);
    void on_processedZoomCombo_currentIndexChanged(int index);



    /**
     * @brief Qt Slot that fires in response to a click on a "Recent File' Menu entry.
     */
    void openRecentFile();

    void loadImageFile(const QString &filename);

    void loadProcessedImageFile(const QString  &filename);
    /**
     * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
     * should be connected to the Signal QRecentFileList->fileListChanged
     * @param file The newly added file.
     */
    void updateRecentFileList(const QString &file);

    void setInputUI();

    // These slots get called when the plugin starts and finishes processing
    void processingStarted();
    void processingFinished();

  protected:

    void loadPlugins();
    void populateMenus(QObject *plugin);
    void addToPluginMenu(QObject *plugin, const QString &text,
                               QMenu *menu, const char *member,
                               QActionGroup *actionGroup);

    QStringList generateInputFileList();

    /**
    *
    * @param event
    */
   void resizeEvent ( QResizeEvent * event );

   /**
    * @brief Implements the CloseEvent to Quit the application and write settings
    * to the preference file
    */
   void closeEvent(QCloseEvent *event);


   /**
    * @brief Reads the Preferences from the users pref file
    */
   void readSettings();

   /**
    * @brief Writes the preferences to the users pref file
    */
   void writeSettings();

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


    /**
     * @brief Opens an Image file
     * @param imageFile The path to the image file to open.
     */
    void openFile(QString imageFile);

    void openProcessedImage(QString mountImage);

    AIMImage::Pointer convertQImageToGrayScaleAIMImage(QImage image);

    qint32 initImageViews();

    AIMImage::Pointer loadImage(QString filePath);


  private:
    QStringList m_PluginDirs;
    QStringList pluginFileNames;
    QActionGroup* pluginActionGroup;
    QImageProcessingInterface* m_ActivePlugin;

    QGraphicsScene*             m_OriginalImageGScene;
    QGraphicsScene*             m_ProcessedImageGScene;

    ImageGraphicsDelegate*      m_OriginalGDelegate;
    ImageGraphicsDelegate*      m_ProcessedGDelegate;

    QVector<QImageProcessingInterface*> m_LoadedPlugins;

    QString                             m_OpenDialogLastDirectory;
    QList<QWidget*>                     m_WidgetList;
    QList<QPair<QString, QString> >     m_InputOutputFilePairList;

    float m_ZoomFactors[10];

    IPHelperApp(const IPHelperApp&); // Copy Constructor Not Implemented
    void operator=(const IPHelperApp&); // Operator '=' Not Implemented
};

#endif /* _IPHELPER_MAINWINDOW_H_ */
