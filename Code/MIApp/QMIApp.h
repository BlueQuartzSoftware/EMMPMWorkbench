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

#ifndef QMIAPP_H_
#define QMIAPP_H_

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

//-- UIC generated Header
#include <ui_QMIApp.h>

#include "AIM/Common/AIMImage.h"

class QMIApp : public QMainWindow, private Ui::QMIApp
{
  Q_OBJECT;

  public:
    QMIApp(QWidget *parent = 0);
    virtual ~QMIApp();
    void initWithFile(const QString imageFile, QString mountImage);


    protected slots:
    void on_actionOpenFixedImage_triggered(); // Open a Data File
    void on_actionOpenMovingImage_triggered(); // Open a Data File

    void on_actionExit_triggered();
    void on_zoomCBox_currentIndexChanged();
    void on_modeComboBox_currentIndexChanged();
    void openFixedRecentFile();
    void openMovingRecentFile();

    void on_newInitAreaBtn_clicked();



      /**
       * @brief Updates the QMenu 'Recent Files' with the latest list of files. This
       * should be connected to the Signal QRecentFileList->fileListChanged
       * @param file The newly added file.
       */
      void updateFixedRecentFileList(const QString &file);
      void updateMovingRecentFileList(const QString &file);
  protected:
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
        void openFixedImageFile(QString fixedImageFile);

        void openMovingImageFile(QString movingImageFile);

        AIMImage::Pointer convertQImageToGrayScaleAIMImage(QImage image);

        qint32 initImageViews();

        AIMImage::Pointer loadImage(QString filePath);

        QImage angFileToQImage(QString filePath);

  private:
        QString m_FixedImageFile;
        QString m_MovingImageFile;

        QString m_OpenDialogLastDirectory;
        QList<QWidget*> m_WidgetList;

        QMIApp(const QMIApp&); // Copy Constructor Not Implemented
        void operator=(const QMIApp&); // Operator '=' Not Implemented

};

#endif /* QMIAPP_H_ */
