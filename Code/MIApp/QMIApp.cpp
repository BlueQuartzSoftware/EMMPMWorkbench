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

#include "QMIApp.h"

#include "QtSupport/QRecentFileList.h"
#include "AngReader/AngReader.h"
#include "AngReader/OIMColoring.hpp"

//-- C++ includes
#include <iostream>

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
QMIApp::QMIApp(QWidget *parent) :
  QMainWindow(parent),
#if defined(Q_WS_WIN)
      m_OpenDialogLastDirectory("C:\\")
#else
      m_OpenDialogLastDirectory("~/")
#endif
{
  setupUi(this);

  setupGui();
  readSettings();

  QRecentFileList* recentFixedFileList = QRecentFileList::instance();
  connect(recentFixedFileList, SIGNAL (fileListChanged(const QString &)), this, SLOT(updateFixedRecentFileList(const QString &)));
  // Get our initial Recent File List
  this->updateFixedRecentFileList(QString::null);

  QRecentFileList* recentMovingFileList = QRecentFileList::instance();
  connect(recentMovingFileList, SIGNAL (fileListChanged(const QString &)), this, SLOT(updateMovingRecentFileList(const QString &)));
  // Get our initial Recent File List
  this->updateMovingRecentFileList(QString::null);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMIApp::~QMIApp()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_actionOpenFixedImage_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString file =
      QFileDialog::getOpenFileName(this, tr("Open Fixed Image"), m_OpenDialogLastDirectory, tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png *.ang)"));

  if (true == file.isNull())
  {
    return;
  }

  QFileInfo fi(file);
  file = QDir::toNativeSeparators(file);

  openFixedImageFile(file);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_actionOpenMovingImage_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString file =
      QFileDialog::getOpenFileName(this, tr("Open Moving Image"), m_OpenDialogLastDirectory, tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png *.ang)"));

  if (true == file.isNull())
  {
    return;
  }

  QFileInfo fi(file);
  file = QDir::toNativeSeparators(file);

  openMovingImageFile(file);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_actionExit_triggered()
{
  this->close();
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void QMIApp::closeEvent(QCloseEvent *event)
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
void QMIApp::resizeEvent(QResizeEvent * event)
{

}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void QMIApp::readSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif

}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void QMIApp::writeSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::setupGui()
{

#ifdef Q_WS_MAC
  // Adjust for the size of the menu bar which is at the top of the screen not in the window
  QSize mySize = size();
  mySize.setHeight(mySize.height() - 30);
  resize(mySize);
#endif
  //-----------------------------
  zoomCBox->blockSignals(true);
  zoomCBox->insertItem(0, "5%", 0.05);
  zoomCBox->insertItem(1, "10%", 0.1);
  zoomCBox->insertItem(2, "25%", 0.25);
  zoomCBox->insertItem(3, "50%", 0.5);
  zoomCBox->insertItem(4, "100%", 1.0);
  zoomCBox->insertItem(5, "150%", 1.5);
  zoomCBox->insertItem(6, "200%", 2.0);
  zoomCBox->insertItem(7, "400%", 4.0);
  zoomCBox->insertItem(8, "600%", 6.0);
  zoomCBox->insertItem(9, "Fit In Window", -1.0);
  zoomCBox->setCurrentIndex(4);
  zoomCBox->blockSignals(false);

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

  modeComboBox->insertItem(11, "Moving");
  modeComboBox->insertItem(12, "Fixed");
#if 0
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

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_modeComboBox_currentIndexChanged()
{
  int index = modeComboBox->currentIndex();
  switch(index)
  {
    case 0:
      compositionRenderer->setExclusionMode();
      break;
    case 1:
      compositionRenderer->setDifferenceMode();
      break;
    case 2:
      compositionRenderer->setPlusMode();
      break;
    case 3:
      compositionRenderer->setMultiplyMode();
      break;
    case 4:
      compositionRenderer->setScreenMode();
      break;
    case 5:
      compositionRenderer->setDarkenMode();
      break;
    case 6:
      compositionRenderer->setLightenMode();
      break;
    case 7:
      compositionRenderer->setColorDodgeMode();
      break;
    case 8:
      compositionRenderer->setColorBurnMode();
      break;
    case 9:
      compositionRenderer->setHardLightMode();
      break;
    case 10:
      compositionRenderer->setSoftLightMode();
      break;

    case 11:
      compositionRenderer->setSourceMode();
      break;
    case 12:
      compositionRenderer->setDestinationMode();
      break;
    case 13:
      compositionRenderer->setSourceOverMode();
      break;
    case 14:
      compositionRenderer->setDestinationOverMode();
      break;
    case 15:
      compositionRenderer->setSourceInMode();
      break;
    case 16:
      compositionRenderer->setDestInMode();
      break;
    case 17:
      compositionRenderer->setDestOutMode();
      break;
    case 18:
      compositionRenderer->setSourceAtopMode();
      break;
    case 19:
      compositionRenderer->setDestAtopMode();
      break;
    case 20:
      compositionRenderer->setOverlayMode();
      break;

    default:
      compositionRenderer->setExclusionMode();
      break;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_zoomCBox_currentIndexChanged()
{
  // std::cout << "on_zoomCBox_currentIndexChanged" << std::endl;
  QVariant zoom = zoomCBox->itemData(zoomCBox->currentIndex(), Qt::UserRole);
  compositionRenderer->setZoomFactor(zoom.toDouble());
  //  this->movingSliceBox->setZoomLevel(zoom.toDouble());
  //
  //  _initializeMovingImageOffset();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
    {
      w->setEnabled(b);
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool QMIApp::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool QMIApp::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
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
qint32 QMIApp::checkDirtyDocument()
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
void QMIApp::updateFixedRecentFileList(const QString &file)
{
  // std::cout << "IPHelperMainWindow::updateRecentFileList" << std::endl;

  // Clear the Recent Items Menu
  this->menu_FixedRecentFiles->clear();

  // Get the list from the static object
  QStringList files = QRecentFileList::instance()->fileList();
  foreach (QString file, files)
    {
      QAction* action = new QAction(this->menu_FixedRecentFiles);
      action->setText(QRecentFileList::instance()->parentAndFileName(file));
      action->setData(file);
      action->setVisible(true);
      this->menu_FixedRecentFiles->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(openFixedRecentFile()));
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::openFixedRecentFile()
{
  //std::cout << "QRecentFileList::openRecentFile()" << std::endl;

  QAction *action = qobject_cast<QAction * > (sender());
  if (action)
  {
    //std::cout << "Opening Recent file: " << action->data().toString().toStdString() << std::endl;
    QString file = action->data().toString();
    openFixedImageFile(file);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::updateMovingRecentFileList(const QString &file)
{
  // std::cout << "IPHelperMainWindow::updateRecentFileList" << std::endl;

  // Clear the Recent Items Menu
  this->menu_MovingRecentFiles->clear();

  // Get the list from the static object
  QStringList files = QRecentFileList::instance()->fileList();
  foreach (QString file, files)
    {
      QAction* action = new QAction(this->menu_MovingRecentFiles);
      action->setText(QRecentFileList::instance()->parentAndFileName(file));
      action->setData(file);
      action->setVisible(true);
      this->menu_MovingRecentFiles->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(openMovingRecentFile()));
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::openMovingRecentFile()
{
  //std::cout << "QRecentFileList::openRecentFile()" << std::endl;

  QAction *action = qobject_cast<QAction * > (sender());
  if (action)
  {
    //std::cout << "Opening Recent file: " << action->data().toString().toStdString() << std::endl;
    QString file = action->data().toString();
    openMovingImageFile(file);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::openFixedImageFile(QString imageFile)
{
  if (true == imageFile.isEmpty()) // User cancelled the operation
  {
    return;
  }
  this->initWithFile(imageFile, m_MovingImageFile);
  this->fixedImageFilename->setText(imageFile);

  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::openMovingImageFile(QString imageFile)
{
  if (true == imageFile.isEmpty()) // User cancelled the operation
  {
    return;
  }
  this->initWithFile(m_FixedImageFile, imageFile);
  this->movingImageFilename->setText(imageFile);
  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::initWithFile(const QString fixedImageFile, QString movingImageFile)
{
  QFileInfo fileInfo(fixedImageFile);
  this->m_OpenDialogLastDirectory = fileInfo.path();
  m_FixedImageFile = fixedImageFile;
  m_MovingImageFile = movingImageFile;

  QVector<QRgb > colorTable(256);
  for (quint32 i = 0; i < 256; ++i)
  {
    colorTable[i] = qRgb(i, i, i);
  }

  QImage image;
  if (m_FixedImageFile.isEmpty() == false)
  {
    QFileInfo fi(m_FixedImageFile);
    QString ext = fi.suffix();
    if (fi.exists() && fi.isFile() && (ext.compare("ang") == 0))
    {
      image = angFileToQImage(m_FixedImageFile);
    }
    else
    {
      image = QImage(m_FixedImageFile);
    }

    if (image.isNull() == true)
    {
      this->statusbar->showMessage("Error loading fixed image from file");
    }

    //fixedImage.setColorTable(colorTable);
    compositionRenderer->setFixedImage(QPixmap::fromImage(image));
  }

  if (m_MovingImageFile.isEmpty() == false)
  {
    QFileInfo fi(m_MovingImageFile);
    QString ext = fi.suffix();
    if (fi.exists() && fi.isFile() && (ext.compare("ang") == 0))
    {
      image = angFileToQImage(m_MovingImageFile);
    }
    else
    {
      image = QImage(m_MovingImageFile);
    }
    if (image.isNull() == true)
    {
      this->statusbar->showMessage("Error loading moving image from file");
    }

    //fixedImage.setColorTable(colorTable);
    compositionRenderer->setMovingImage(QPixmap::fromImage(image));
  }

  this->statusBar()->showMessage("Fixed Image Loaded");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void QMIApp::on_newInitAreaBtn_clicked()
{
  std::cout << "on_newInitAreaBtn_clicked" << std::endl;
  if (compositionRenderer != NULL)
  {
    compositionRenderer->setNewInitArea();
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QImage QMIApp::angFileToQImage(QString angFileToRead)
{
  // Prime some values by reading a single ANG file
  AngReader reader;
  reader.setUserOrigin(AngReader::UpperLeftOrigin);
  int err = 0;
  //  std::cout << " Reading File " << angFileToRead.toStdString() << std::endl;
  reader.setFileName(angFileToRead.toStdString());
  err = reader.readFile();
  if (1 != err)
  {
    return QImage();
  }
  int xpoints = reader.getNumEvenCols();
  int ypoints = reader.getNumRows();

  float* phi1F = reader.getPhi1Pointer();
  float* phiF = reader.getPhiPointer();
  float* phi2F = reader.getPhi2Pointer();

  float refDir0 = 0.0f;
  float refDir1 = 0.0f;
  float refDir2 = 1.0f; // ND Normal Direction

  unsigned int rgba = 0x00000000;
  //  unsigned char* rgbPtr = reinterpret_cast<unsigned char* > (&rgba);
  size_t index = 0;
  //- Create Color QImage
  QImage image(xpoints, ypoints, QImage::Format_RGB32);
  image.fill(0);
  for (int y = 0; y < ypoints; ++y)
  {
    for (int x = 0; x < xpoints; ++x)
    {
      //     index = (y *  xpoints) + x;
      rgba = 0;
      OIMColoring::GenerateIPFColor<float >(phi1F[index], phiF[index], phi2F[index], refDir0, refDir1, refDir2, reinterpret_cast<unsigned char* > (&rgba), 2, 1, 0);
      image.setPixel(x, y, rgba);
      ++index;
    }
  }
  return image;
}

