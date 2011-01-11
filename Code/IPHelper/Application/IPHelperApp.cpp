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
#include "IPHelperApp.h"

//-- C++ includes
#include <iostream>

//-- Qt Includes

#include <QtCore/QPluginLoader>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/QFileInfoList>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>
#include <QtGui/QStringListModel>
#include <QtGui/QLineEdit>

// Our Project wide includes
#include "QtSupport/ApplicationAboutBoxDialog.h"
#include "QtSupport/QRecentFileList.h"
#include "QtSupport/QFileCompleter.h"
#include "QtSupport/ImageGraphicsDelegate.h"
#include "QtSupport/ProcessQueueController.h"
#include "QtSupport/ProcessQueueDialog.h"


#include "EIMApps/Common/EIMAppsVersion.h"

#include "IPHelper/plugins/QImageProcessingInterface.h"

#include "License/LicenseFiles.h"

#define READ_STRING_SETTING(prefs, var, emptyValue)\
  var->setText( prefs.value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }


#define READ_SETTING(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs.value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var->setValue(temp);


#define WRITE_STRING_SETTING(prefs, var)\
  prefs.setValue(#var , this->var->text());

#define WRITE_SETTING(prefs, var)\
  prefs.setValue(#var, this->var->value());

#define READ_BOOL_SETTING(prefs, var, emptyValue)\
  { QString s = prefs.value(#var).toString();\
  if (s.isEmpty() == false) {\
    bool bb = prefs.value(#var).toBool();\
  var->setChecked(bb); } else { var->setChecked(emptyValue); } }

#define WRITE_BOOL_SETTING(prefs, var, b)\
    prefs.setValue(#var, (b) );


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
IPHelperApp::IPHelperApp(QWidget *parent) :
QMainWindow(parent),
pluginActionGroup(NULL),
m_ActivePlugin(NULL),
m_OriginalImageGScene(NULL),
m_ProcessedImageGScene(NULL),
m_OriginalGDelegate(NULL),
m_ProcessedGDelegate(NULL),
#if defined(Q_WS_WIN)
    m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{
  setupUi(this);

  setupGui();
  loadPlugins();
  readSettings();

  QRecentFileList* recentFileList = QRecentFileList::instance();
  connect(recentFileList, SIGNAL (fileListChanged(const QString &)), this, SLOT(updateBaseRecentFileList(const QString &)));
  // Get out initial Recent File List
  this->updateRecentFileList(QString::null);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
IPHelperApp::~IPHelperApp()
{
  // TODO Auto-generated destructor stub
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionExit_triggered()
{
  this->close();
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void IPHelperApp::closeEvent(QCloseEvent *event)
{
  qint32 err = checkDirtyDocument();
  if (err < 0)
  {
    event->ignore();
  }
  else
  {
    writeSettings();
    event->accept();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::resizeEvent ( QResizeEvent * event )
{
  QSize osize = originalImageFrame->size();
//  std::cout << "originalImageFrame.size: " << osize.width() << " x " << osize.height() << std::endl;
  QSize psize = processedImageFrame->size();
//  std::cout << "processedImageFrame.size: " << psize.width() << " x " << psize.height() << std::endl;
  if (originalImageGView->isVisible() == true)
  {
    QRect sceneRect(0, 0, osize.width(), osize.height());
    processedImageFrame->setGeometry(sceneRect);
    processedImageGView->setGeometry(sceneRect);
    if (NULL != m_ProcessedImageGScene) m_ProcessedImageGScene->setSceneRect(sceneRect);
  }
  else if (processedImageGView->isVisible() == true)
  {
    QRect sceneRect(0, 0, psize.width(), psize.height());
    originalImageFrame->setGeometry(sceneRect);
    originalImageGView->setGeometry(sceneRect);

    if (NULL != m_OriginalImageGScene) m_OriginalImageGScene->setSceneRect(sceneRect);
  }
  emit parentResized();
}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void IPHelperApp::readSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif
  foreach (QImageProcessingInterface* plugin, m_LoadedPlugins) {
    plugin->readSettings(&prefs);
  }

}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void IPHelperApp::writeSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif

  foreach (QImageProcessingInterface* plugin, m_LoadedPlugins) {
    plugin->writeSettings(&prefs);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::setupGui()
{

#ifdef Q_WS_MAC
  // Adjust for the size of the menu bar which is at the top of the screen not in the window
  QSize mySize = size();
  mySize.setHeight( mySize.height() -30);
  resize(mySize);
#endif
  m_OriginalImageGScene = NULL;
  m_ProcessedImageGScene = NULL;

  openProcessedImageCB->setVisible(false);
  openOriginalImageCB->setVisible(false);

  modeComboBox->blockSignals(true);

  modeComboBox->insertItem(0, "Exclusion");
  modeComboBox->insertItem(1, "Difference");
  modeComboBox->insertItem(2, "Plus");
  modeComboBox->insertItem(3, "Multiply");
  modeComboBox->insertItem(4, "Screen");
  modeComboBox->insertItem(5, "Darken");
  modeComboBox->insertItem(6, "Lighten");
  modeComboBox->insertItem(7, "Color Dodge");
  modeComboBox->insertItem(8, "Color Burn");
  modeComboBox->insertItem(9, "Hard Light");
  modeComboBox->insertItem(10, "Soft Light");
#if 0
  modeComboBox->insertItem(11, "Source");
  modeComboBox->insertItem(12, "Destination");
  modeComboBox->insertItem(13, "Source Over");
  modeComboBox->insertItem(14, "Destination Over");
  modeComboBox->insertItem(15, "Source In");
  modeComboBox->insertItem(16, "Dest In");

  modeComboBox->insertItem(17, "Dest Out");
  modeComboBox->insertItem(18, "Source Atop");
  modeComboBox->insertItem(19, "Dest Atop");

  modeComboBox->insertItem(20, "Overlay");
#endif

  modeComboBox->setCurrentIndex(0);
  modeComboBox->blockSignals(false);

  compositeWithOriginal->setEnabled(false);
  processedZoomCombo->setEnabled(false);
  fixedZoomCombo->setEnabled(false);
  compositeWithOriginal->setChecked(false);
  modeComboBox->setEnabled(compositeWithOriginal->isChecked());

  connect (originalImageGView, SIGNAL(loadImageFileRequested(const QString &)),
           this, SLOT(loadImageFile(const QString &)), Qt::QueuedConnection);

  connect (processedImageGView, SIGNAL(loadImageFileRequested(const QString &)),
           this, SLOT(loadProcessedImageFile(const QString &)));

  pluginActionGroup = new QActionGroup(this);

  m_ZoomFactors[0] = 0.1f;
  m_ZoomFactors[1] = 0.25f;
  m_ZoomFactors[2] = 0.5f;
  m_ZoomFactors[3] = 1.0f;
  m_ZoomFactors[4] = 1.250f;
  m_ZoomFactors[5] = 1.500f;
  m_ZoomFactors[6] = 2.000f;
  m_ZoomFactors[7] = 4.000f;
  m_ZoomFactors[8] = 6.000f;
  m_ZoomFactors[9] = -1.0f;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_modeComboBox_currentIndexChanged()
{
  if (NULL == m_ProcessedGDelegate) { return; }
  int index = modeComboBox->currentIndex();
  switch(index)
  {
  case 0: m_ProcessedGDelegate->setExclusionMode(); break;
  case 1: m_ProcessedGDelegate->setDifferenceMode(); break;
  case 2: m_ProcessedGDelegate->setPlusMode(); break;
  case 3: m_ProcessedGDelegate->setMultiplyMode(); break;
  case 4: m_ProcessedGDelegate->setScreenMode(); break;
  case 5: m_ProcessedGDelegate->setDarkenMode(); break;
  case 6: m_ProcessedGDelegate->setLightenMode(); break;
  case 7: m_ProcessedGDelegate->setColorDodgeMode(); break;
  case 8: m_ProcessedGDelegate->setColorBurnMode(); break;
  case 9: m_ProcessedGDelegate->setHardLightMode(); break;
  case 10: m_ProcessedGDelegate->setSoftLightMode(); break;

  case 11: m_ProcessedGDelegate->setSourceMode(); break;
  case 12: m_ProcessedGDelegate->setDestinationMode(); break;
  case 13: m_ProcessedGDelegate->setSourceOverMode(); break;
  case 14: m_ProcessedGDelegate->setDestinationOverMode(); break;
  case 15: m_ProcessedGDelegate->setSourceInMode(); break;
  case 16: m_ProcessedGDelegate->setDestInMode(); break;
  case 17: m_ProcessedGDelegate->setDestOutMode(); break;
  case 18: m_ProcessedGDelegate->setSourceAtopMode(); break;
  case 19: m_ProcessedGDelegate->setDestAtopMode(); break;
  case 20: m_ProcessedGDelegate->setOverlayMode(); break;

  default:
    m_ProcessedGDelegate->setExclusionMode(); break;
  }

  m_ProcessedGDelegate->setOverlayImage(m_OriginalGDelegate->getCachedImage());
  m_ProcessedGDelegate->setCompositeImages( compositeWithOriginal->isChecked() );
  m_ProcessedGDelegate->updateGraphicsView();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
  {
    w->setEnabled(b);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_compositeWithOriginal_stateChanged(int state)
{
  if (NULL != m_OriginalGDelegate && NULL != m_ProcessedGDelegate) {
    modeComboBox->setEnabled(compositeWithOriginal->isChecked());
    if (NULL != m_OriginalGDelegate)
    {
      QImage cachedImage ( m_OriginalGDelegate->getCachedImage() );
      if (cachedImage.isNull() != true) {
        m_ProcessedGDelegate->setOverlayImage(cachedImage);
        m_ProcessedGDelegate->setCompositeImages( compositeWithOriginal->isChecked() );
        m_ProcessedGDelegate->updateGraphicsView();
      }
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool IPHelperApp::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool IPHelperApp::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  if (false == fileinfo.exists())
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return fileinfo.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
qint32 IPHelperApp::checkDirtyDocument()
{
  qint32 err = -1;
  {
    err = 1;
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::updateRecentFileList(const QString &file)
{
  // std::cout << "IPHelperMainWindow::updateRecentFileList" << std::endl;

  // Clear the Recent Items Menu
  this->menu_RecentFiles->clear();

  // Get the list from the static object
  QStringList files = QRecentFileList::instance()->fileList();
  foreach (QString file, files)
    {
      QAction* action = new QAction(this->menu_RecentFiles);
      action->setText(QRecentFileList::instance()->parentAndFileName(file));
      action->setData(file);
      action->setVisible(true);
      this->menu_RecentFiles->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(openRecentBaseImageFile()));
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::openRecentFile()
{
  //std::cout << "QRecentFileList::openRecentFile()" << std::endl;

  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
  {
    //std::cout << "Opening Recent file: " << action->data().toString().toStdString() << std::endl;
    QString file = action->data().toString();
    openFile( file );
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::openFile(QString imageFile)
{
  if ( true == imageFile.isEmpty() ) // User cancelled the operation
  {
    return;
  }
  this->initWithFile(imageFile, m_CurrentProcessedFile);

  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
  updateRecentFileList(imageFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_aboutBtn_clicked()
{
  ApplicationAboutBoxDialog about(EIMApps::LicenseList, this);
  QString an = QCoreApplication::applicationName();
  QString version("");
  version.append(EIMApps::Version::PackageComplete.c_str());
  about.setApplicationInfo(an, version);
  about.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_processBtn_clicked()
{
  int err = m_ActivePlugin->startProcessing(this);
  if (err == 0)
  {

  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::processingStarted()
{
  std::cout << "IPHelper::processingStarted()" << std::endl;
  processBtn->setText("Cancel");
  processBtn->setEnabled(false);
  this->statusBar()->showMessage("Processing Images...");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::processingFinished()
{
//  std::cout << "IPHelper::processingFinished()" << std::endl;
  /* Code that cleans up anything from the processing */
  processBtn->setText("Process");
  processBtn->setEnabled(true);
  this->statusBar()->showMessage("Processing Complete");

  // Get the image files from the plugin
  m_CurrentImageFile = m_ActivePlugin->getInputImage();
  m_CurrentProcessedFile = m_ActivePlugin->getProcessedImage();
  initWithFile(m_CurrentImageFile, m_CurrentProcessedFile);

  m_InputOutputFilePairList = m_ActivePlugin->getInputOutputFilePairs();
  originalImageFileList->blockSignals(true);
  processedImageFileList->blockSignals(true);
  originalImageFileList->clear();
  processedImageFileList->clear();
  for (int i = 0; i < m_InputOutputFilePairList.size(); ++i)
  {
    QPair<QString, QString> pair = m_InputOutputFilePairList.at(i);
    QFileInfo fi(pair.first);
    originalImageFileList->addItem(fi.fileName());
    fi = QFileInfo(pair.second);
    processedImageFileList->addItem(fi.fileName());
  }
  originalImageFileList->blockSignals(false);
  processedImageFileList->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_originalImageFileList_currentIndexChanged(int index)
{
  if (index < 0) return;
  openFile(m_InputOutputFilePairList.at(index).first);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_processedImageFileList_currentIndexChanged(int index)
{
  if (index < 0) return;
  openProcessedImage(m_InputOutputFilePairList.at(index).second);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionOpen_Processed_Image_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile = QFileDialog::getOpenFileName(this, tr("Open Processed Image File"),
    m_OpenDialogLastDirectory,
    tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)") );

  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openProcessedImage(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionOpen_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile = QFileDialog::getOpenFileName(this, tr("Open Image File"),
    m_OpenDialogLastDirectory,
    tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)") );

  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openFile(imageFile);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::loadImageFile(const QString &imageFile)
{
  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openFile(imageFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::loadProcessedImageFile(const QString  &imageFile)
{
  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openProcessedImage(imageFile);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionSave_triggered()
{
  saveProcessedImage();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionSave_As_triggered()
{
  m_CurrentProcessedFile = QString();
  saveProcessedImage();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_actionClose_triggered() {
  qint32 err = -1;
  err = checkDirtyDocument();
  if (err >= 0)
  {
    // Close the window. Files have been saved if needed
    if (QApplication::activeWindow() == this)
    {
      this->close();
    }
    else
    {
      QApplication::activeWindow()->close();
    }
  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::openProcessedImage(QString processedImage)
{
  if ( true == processedImage.isEmpty() ) // User cancelled the operation
  {
    return;
  }
  this->initWithFile(m_CurrentImageFile, processedImage);
  setWidgetListEnabled(true);
  if (compositeWithOriginal->isChecked()) {
    on_modeComboBox_currentIndexChanged();
  }
  updateRecentFileList(processedImage);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
qint32 IPHelperApp::saveProcessedImage()
{
  QImage image = m_ProcessedGDelegate->getCachedImage();
  if (m_ProcessedGDelegate->getCompositeImages() == true) {
    image = m_ProcessedGDelegate->getCompositedImage();
  }
  int err = 0;
  if (m_CurrentProcessedFile.isEmpty())
  {
    QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator() + "Processed.tif";
    outputFile = QFileDialog::getSaveFileName(this, tr("Save Processed Image As ..."), outputFile, tr("Images (*.tif *.bmp *.jpg *.png)"));
    if ( !outputFile.isEmpty() )
    {
      m_CurrentProcessedFile = outputFile;
    }
    else {
      return -1;
    }
  }

  bool ok = image.save(m_CurrentProcessedFile);
  if (ok == true) {
    processedImageTitle->setText(m_CurrentProcessedFile);
  }
  else
  {
    //TODO: Add in a GUI dialog to help explain the error or give suggestions.
    err = -1;
  }
  this->setWindowModified(false);
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIMImage::Pointer IPHelperApp::loadImage(QString filePath)
{
//  std::cout << " loadImage(): filePath: " << filePath.toStdString() << std::endl;
  QImage image;
  AIMImage::Pointer aimImage = AIMImage::NullPointer();
  if (filePath.isEmpty() == false)
  {
    image = QImage(filePath);
    if (image.isNull() == true)
    {
      QString m("Error loading image from ");
      m.append(filePath);
      qDebug(m.toAscii(), "%s");
      return aimImage;
    }
    QVector<QRgb> colorTable(256);
    for (quint32 i = 0; i < 256; ++i)
    {
      colorTable[i] = qRgb(i, i, i);
    }
    image.setColorTable(colorTable);
    aimImage = convertQImageToGrayScaleAIMImage(image);
    if (NULL == aimImage.RAW_PTR())
    {
      return aimImage;
    }
  }
  return aimImage;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
qint32 IPHelperApp::initImageViews()
{
  qint32 err = 0;
  QImage image;
  if (m_CurrentImageFile.isEmpty() == false)
  {
    image = QImage(m_CurrentImageFile);
    if (image.isNull() == true)
    {
      this->statusbar->showMessage("Error loading image from file");
      return -1;
    }
    QVector<QRgb> colorTable(256);
    for (quint32 i = 0; i < 256; ++i)
    {
      colorTable[i] = qRgb(i, i, i);
    }
    image.setColorTable(colorTable);


    // Create the QGraphicsScene Objects
    if (NULL == m_OriginalImageGScene)
    {
      m_OriginalImageGScene = new QGraphicsScene(this);
      QSize baseSize = originalImageFrame->baseSize();
      QRect sceneRect(0, 0, baseSize.width(), baseSize.height());
      originalImageFrame->setGeometry(sceneRect);
      originalImageGView->setGeometry(sceneRect);
      m_OriginalImageGScene->setSceneRect(sceneRect);
      originalImageGView->setScene(m_OriginalImageGScene);
    }
    if (NULL == m_OriginalGDelegate) {
      m_OriginalGDelegate = new ImageGraphicsDelegate(this);
      m_OriginalGDelegate->setDelegateName(QString("Original Image"));
      m_OriginalGDelegate->setGraphicsView(originalImageGView);
      m_OriginalGDelegate->setGraphicsScene(m_OriginalImageGScene);
      m_OriginalGDelegate->setMainWindow(this);
    }

    m_OriginalGDelegate->setCachedImage(image);
    m_OriginalGDelegate->setZoomIndex(fixedZoomCombo->currentIndex());
    m_OriginalGDelegate->updateGraphicsView(false);

    connect(this, SIGNAL(parentResized () ),
            m_OriginalGDelegate, SLOT(on_parentResized () ));
    // Create the m_OriginalImage and m_Processed Image Objects
    m_OriginalImage = convertQImageToGrayScaleAIMImage(image);
    if (NULL == m_OriginalImage.RAW_PTR())
    {
      return -1;
    }

  }


  // If we have NOT loaded a processed file AND we have a valid Original Image, then
  // create the Processed image based on the input image.
  QImage processedImage;
  if (false && m_CurrentProcessedFile.isEmpty() == true && NULL != m_OriginalImage.RAW_PTR() )
  {
    processedImage = image;
    m_ProcessedImage = convertQImageToGrayScaleAIMImage(processedImage);
    if (NULL == m_ProcessedImage.RAW_PTR())
    {
      return -1;
    }
  }
  else // We have an actual processed image file that the user wants us to read.
  {
    processedImage = QImage(m_CurrentProcessedFile);
    if (processedImage.isNull() == true)
    {
      this->statusbar->showMessage("Error loading Processed image from file");
      return -1;
    }
    // Convert it to an AIMImage in GrayScale
    m_ProcessedImage = convertQImageToGrayScaleAIMImage(processedImage);
    if (NULL == m_ProcessedImage.RAW_PTR() )
    {
      std::cout << "Error loading Processed image from file" << std::endl;
      return -1;
    }
  }

  if (NULL != m_ProcessedImage.RAW_PTR() )
  {
    // Create the QGraphicsScene Objects
    if (NULL == m_ProcessedImageGScene) {
      m_ProcessedImageGScene = new QGraphicsScene(this);
      QSize baseSize = processedImageFrame->baseSize();
      QRect sceneRect(0, 0, baseSize.width(), baseSize.height());
      processedImageFrame->setGeometry(sceneRect);
      processedImageGView->setGeometry(sceneRect);
      m_ProcessedImageGScene->setSceneRect(sceneRect);
      processedImageGView->setScene(m_ProcessedImageGScene);
      processedImageGView->setScene(m_ProcessedImageGScene);
    }

    if (NULL == m_ProcessedGDelegate) {
      m_ProcessedGDelegate = new ImageGraphicsDelegate(this);
      m_ProcessedGDelegate->setDelegateName(QString("Processed Image"));
      m_ProcessedGDelegate->setGraphicsView(processedImageGView);
      m_ProcessedGDelegate->setGraphicsScene(m_ProcessedImageGScene);
      m_ProcessedGDelegate->setMainWindow(this);
    }
    m_ProcessedGDelegate->setCachedImage(processedImage);
    m_ProcessedGDelegate->setZoomIndex(processedZoomCombo->currentIndex());
    m_ProcessedGDelegate->updateGraphicsView(false);

    connect(this, SIGNAL(parentResized () ),
            m_ProcessedGDelegate, SLOT(on_parentResized () ) );
  }

  if (compositeWithOriginal->isChecked())
  {
    on_compositeWithOriginal_stateChanged(Qt::Checked);
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::initWithFile(const QString imageFile, QString processedImage)
{
  QFileInfo fileInfo(imageFile);
  this->m_OpenDialogLastDirectory = fileInfo.path();

  m_CurrentImageFile = imageFile;
  m_CurrentProcessedFile = processedImage;
  if (m_CurrentImageFile.isEmpty())
  {
    this->compositeWithOriginal->setEnabled(false);
    fixedZoomCombo->setEnabled(false);
  }
  else
  {
    fixedZoomCombo->setEnabled(true);
  }
  qint32 err = initImageViews();

  if (err < 0)
  {
    this->statusBar()->showMessage("Error Loading Images");
    return;
  }
  this->originalImageTitle->setText(fileInfo.fileName());
  this->originalImageTitle->setToolTip(m_CurrentImageFile);

  if (m_CurrentProcessedFile.isEmpty())
  {
    this->processedImageTitle->setText("Unsaved Processed Image");
    this->setWindowModified(true);
    compositeWithOriginal->setEnabled(false);
    processedZoomCombo->setEnabled(false);
  }
  else
  {
    QFileInfo segInfo(m_CurrentProcessedFile);
    this->processedImageTitle->setText(segInfo.fileName());
    this->processedImageTitle->setToolTip(m_CurrentProcessedFile);
    if (m_CurrentImageFile.isEmpty() == false) { compositeWithOriginal->setEnabled(true); }
    processedZoomCombo->setEnabled(true);
  }
  this->statusBar()->showMessage("Input Image Loaded");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIMImage::Pointer IPHelperApp::convertQImageToGrayScaleAIMImage(QImage image)
{
  AIMImage::Pointer aimImage = AIMImage::New();
  quint8* oImage = aimImage->allocateImageBuffer(image.width(), image.height(), true);
  if (NULL == oImage)
  {
    return AIMImage::NullPointer();
  }

  // Copy the QImage into the AIMImage object, converting to gray scale as we go.
  qint32 height = image.height();
  qint32 width = image.width();
  QRgb rgbPixel;
  int gray;
  qint32 index;
  for (qint32 y=0; y<height; y++) {
    for (qint32 x =0; x<width; x++) {
      index = (y *  width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      oImage[index] = static_cast<unsigned char>(gray);
    }
  }
  return aimImage;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::loadPlugins()
 {
     foreach (QObject *plugin, QPluginLoader::staticInstances())
         populateMenus(plugin);

     m_PluginDirs.clear();
     m_PluginDirs << qApp->applicationDirPath();

     QDir aPluginDir = QDir(qApp->applicationDirPath());
     QString thePath;

 #if defined(Q_OS_WIN)
     if (aPluginDir.cd("plugins") ) {
      thePath = aPluginDir.absolutePath();
      m_PluginDirs << thePath;
     }
 #elif defined(Q_OS_MAC)
     if (aPluginDir.dirName() == "MacOS") {
         aPluginDir.cdUp();
         thePath = aPluginDir.absolutePath() + "/Plugins";
         m_PluginDirs << thePath;
         aPluginDir.cdUp();
         thePath = aPluginDir.absolutePath() + "/Plugins";
        // m_PluginDirs << thePath;
         aPluginDir.cdUp();
     }
    // aPluginDir.cd("Plugins");
     thePath = aPluginDir.absolutePath() + "/Plugins";
     m_PluginDirs << thePath;
#else
     if (aPluginDir.cd("plugins")) {
      thePath = aPluginDir.absolutePath();
      m_PluginDirs << thePath;
     }
#endif

  this->setWindowTitle("IPHelper - No Plugins Loaded");

foreach (QString pluginDirString, m_PluginDirs) {
 // std::cout << "Plugin Directory being Searched: " << pluginDirString.toStdString() << std::endl;
    aPluginDir = QDir(pluginDirString);
     foreach (QString fileName, aPluginDir.entryList(QDir::Files))
    {
   //   std::cout << "File: " << fileName.toStdString() << std::endl;
#ifdef QT_DEBUG
       if (fileName.endsWith( "_debug.plugin", Qt::CaseSensitive) )
#endif

#if defined (QT_NO_DEBUG)
       if (fileName.endsWith( ".plugin", Qt::CaseSensitive) )
#endif
       {
    //     std::cout << "File Extension matches.." << std::endl;
         QPluginLoader loader(aPluginDir.absoluteFilePath(fileName));
         QObject *plugin = loader.instance();
       //  std::cout << "plugin Pointer: " << plugin << std::endl;
         if (plugin && pluginFileNames.contains(fileName, Qt::CaseSensitive) == false)
         {
             populateMenus(plugin);
             pluginFileNames += fileName;
         }
         else
         {
           std::cout << "The plugin did not load with the following error\n   " << loader.errorString().toStdString() << std::endl;
         }
       }
     }

     menuPlugins->setEnabled(!pluginActionGroup->actions().isEmpty());
     // Load the first plugin found
     if (pluginActionGroup->actions().size() > 0) {
       pluginActionGroup->actions().at(0)->activate(QAction::Trigger);
     }
  }
 }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
 void IPHelperApp::populateMenus(QObject *plugin)
{
  std::cout << "Found Plugin..." << std::endl;
  QImageProcessingInterface* ipPlugin = qobject_cast<QImageProcessingInterface * > (plugin);
  if (ipPlugin)
  {
    m_LoadedPlugins.push_back(ipPlugin);
    qWarning(ipPlugin->getPluginName().toAscii(), "%s");
    addToPluginMenu(plugin, ipPlugin->getPluginName(), menuPlugins, SLOT(setInputUI()), pluginActionGroup);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::addToPluginMenu(QObject *plugin, const QString &text,
                                     QMenu *menu, const char *member,
                                     QActionGroup *actionGroup)
{
  QAction *action = new QAction(text, plugin);
  connect(action, SIGNAL(triggered()), this, member);
  menu->addAction(action);

  if (actionGroup)
  {
    action->setCheckable(true);
    actionGroup->addAction(action);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::setInputUI()
{
  // Get the current QWidget
  if (NULL != m_ActivePlugin) {
    QWidget* activeInputWidget = m_ActivePlugin->getInputWidget(this);
    inputsTab->layout()->removeWidget(activeInputWidget);
  }
  // Get the action Associated with the Plugin that was just activated
  QAction *action = qobject_cast<QAction*> (sender());
  // Get a pointer to the new active plugin instance
  m_ActivePlugin = qobject_cast<QImageProcessingInterface* > (action->parent());
  this->setWindowTitle(m_ActivePlugin->getPluginName() + " is now Active");

  // Get a pointer to the plugins Input Widget
  QWidget* inputWidget = m_ActivePlugin->getInputWidget(this);
  inputsTab->layout()->addWidget(inputWidget);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_fixedZoomCombo_currentIndexChanged(int index)
{
//  std::cout << "on_fixedZoomCombo_currentIndexChanged" << std::endl;
  m_OriginalGDelegate->setZoomIndex(index);
  m_OriginalGDelegate->updateGraphicsView(true);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void IPHelperApp::on_processedZoomCombo_currentIndexChanged(int index)
{
  m_ProcessedGDelegate->setZoomIndex(index);
  m_ProcessedGDelegate->updateGraphicsView(true);

}
