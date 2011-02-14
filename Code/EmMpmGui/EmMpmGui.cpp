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

#include "EmMpmGui.h"

#include <iostream>
#include <sstream>
#include <limits>
#include <fstream>

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

//-- Qwt Includes
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_interval_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

#include <emmpm/public/InitializationFunctions.h>
#include <emmpm/public/ProgressFunctions.h>
#include <emmpm/tiff/EMTiffIO.h>
#include <emmpm/public/EMMPM_Structures.h>
#include <emmpm/public/EMMPM.h>

//
#include "EmMpmGuiVersion.h"
#include "UserInitAreaTableModel.h"
#include "EMMPMTask.h"
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
EmMpmGui::EmMpmGui(QWidget *parent) :
QMainWindow(parent),
m_zoomer(NULL),
m_picker(NULL),
m_panner(NULL),
m_grid(NULL),
m_histogram(NULL),
m_ProxyModel(NULL),
m_OutputExistsCheck(false),
m_QueueController(NULL),
m_QueueDialog(NULL),
#if defined(Q_WS_WIN)
m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{
  setupUi(this);
  setupGui();

  readSettings();

  QRecentFileList* recentFileList = QRecentFileList::instance();
  connect(recentFileList, SIGNAL (fileListChanged(const QString &)), this, SLOT(updateBaseRecentFileList(const QString &)));
  // Get out initial Recent File List
  this->updateBaseRecentFileList(QString::null);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EmMpmGui::~EmMpmGui()
{

}


// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void EmMpmGui::closeEvent(QCloseEvent *event)
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
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void EmMpmGui::readSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif

  QString val;
  bool ok;
  qint32 i;
  prefs.beginGroup("EMMPMPlugin");
  READ_STRING_SETTING(prefs, m_Beta, "5.5");
  READ_STRING_SETTING(prefs, m_Gamma, "0.1");
  READ_SETTING(prefs, m_MpmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_EmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_NumClasses, ok, i, 2, Int);
  READ_BOOL_SETTING(prefs, useSimulatedAnnealing, true);
  READ_BOOL_SETTING(prefs, processFolder, false);
 // READ_STRING_SETTING(prefs, inputImageFilePath, "");
  READ_STRING_SETTING(prefs, outputImageFile, "");
  READ_STRING_SETTING(prefs, sourceDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputPrefix, "");
  READ_STRING_SETTING(prefs, outputSuffix, "");
  prefs.endGroup();
  on_processFolder_stateChanged(processFolder->isChecked());

  if (this->sourceDirectoryLE->text().isEmpty() == false)
  {
    this->populateFileTable(this->sourceDirectoryLE, this->fileListView);
  }
}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void EmMpmGui::writeSettings()
{
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif
  prefs.beginGroup("EMMPMPlugin");
  WRITE_STRING_SETTING(prefs, m_Beta);
  WRITE_STRING_SETTING(prefs, m_Gamma);
  WRITE_SETTING(prefs, m_MpmIterations);
  WRITE_SETTING(prefs, m_EmIterations);
  WRITE_SETTING(prefs, m_NumClasses);
  WRITE_BOOL_SETTING(prefs, useSimulatedAnnealing, useSimulatedAnnealing->isChecked());
  WRITE_BOOL_SETTING(prefs, processFolder, processFolder->isChecked());
  WRITE_STRING_SETTING(prefs, inputImageFilePath);
  WRITE_STRING_SETTING(prefs, outputImageFile);
  WRITE_STRING_SETTING(prefs, sourceDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputPrefix);
  WRITE_STRING_SETTING(prefs, outputSuffix);
  prefs.endGroup();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::setupGui()
{
#ifdef Q_WS_MAC
  // Adjust for the size of the menu bar which is at the top of the screen not in the window
  QSize mySize = size();
  mySize.setHeight( mySize.height() -30);
  resize(mySize);
#endif

  m_GraphicsView->setEmMpmGui(this);
  compositeModeCB->blockSignals(true);

  compositeModeCB->insertItem(0, "Exclusion");
  compositeModeCB->insertItem(1, "Difference");
  compositeModeCB->insertItem(2, "Plus");
  compositeModeCB->insertItem(3, "Multiply");
  compositeModeCB->insertItem(4, "Screen");
  compositeModeCB->insertItem(5, "Darken");
  compositeModeCB->insertItem(6, "Lighten");
  compositeModeCB->insertItem(7, "Color Dodge");
  compositeModeCB->insertItem(8, "Color Burn");
  compositeModeCB->insertItem(9, "Hard Light");
  compositeModeCB->insertItem(10, "Soft Light");
#if 0
  compositeModeCB->insertItem(11, "Source");
  compositeModeCB->insertItem(12, "Destination");
  compositeModeCB->insertItem(13, "Source Over");
  compositeModeCB->insertItem(14, "Destination Over");
  compositeModeCB->insertItem(15, "Source In");
  compositeModeCB->insertItem(16, "Dest In");

  compositeModeCB->insertItem(17, "Dest Out");
  compositeModeCB->insertItem(18, "Source Atop");
  compositeModeCB->insertItem(19, "Dest Atop");

  compositeModeCB->insertItem(20, "Overlay");
  compositeModeCB->insertItem(21, "Clear");
#endif

  compositeModeCB->setCurrentIndex(0);
  compositeModeCB->blockSignals(false);
  compositeModeCB->setEnabled(false);


  QHeaderView* headerView = new QHeaderView(Qt::Horizontal, m_UserInitTable);
  headerView->setResizeMode(QHeaderView::Interactive);
  m_UserInitTable->setHorizontalHeader(headerView);
  m_UserInitAreaTableModel = new UserInitAreaTableModel;
  m_GraphicsView->setUserInitAreaTableModel(m_UserInitAreaTableModel);
  m_UserInitTable->setModel(m_UserInitAreaTableModel);
  headerView->show();



  connect (m_GraphicsView, SIGNAL(fireBaseImageFileLoaded(const QString &)),
           this, SLOT(baseImageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (m_GraphicsView, SIGNAL(fireOverlayImageFileLoaded(const QString &)),
           this, SLOT(overlayImageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (addUserInitArea, SIGNAL(toggled(bool)),
           m_GraphicsView, SLOT(addUserInitArea(bool)));

  connect (m_GraphicsView, SIGNAL(fireUserInitAreaAdded(UserInitArea*)),
           this, SLOT(userInitAreaAdded(UserInitArea*)), Qt::QueuedConnection);

  connect (zoomIn, SIGNAL(clicked()),
           m_GraphicsView, SLOT(zoomIn()), Qt::QueuedConnection);
  connect(zoomOut, SIGNAL(clicked()),
          m_GraphicsView, SLOT(zoomOut()), Qt::QueuedConnection);
  connect (zoomCB, SIGNAL(currentIndexChanged(int)),
           m_GraphicsView, SLOT(setZoomIndex(int)), Qt::QueuedConnection);
  connect(fitToWindow, SIGNAL(clicked()),
          m_GraphicsView, SLOT(fitToWindow()), Qt::QueuedConnection);

  connect(imageDisplayCombo, SIGNAL(currentIndexChanged(int)),
          m_GraphicsView, SLOT(setImageDisplayType(int)));

  QFileCompleter* com = new QFileCompleter(this, false);
  inputImageFilePath->setCompleter(com);
  QObject::connect(com, SIGNAL(activated(const QString &)), this, SLOT(on_inputImageFilePath_textChanged(const QString &)));

  QFileCompleter* com4 = new QFileCompleter(this, false);
  outputImageFile->setCompleter(com4);
  QObject::connect(com4, SIGNAL(activated(const QString &)), this, SLOT(on_outputImageFile_textChanged(const QString &)));

  QFileCompleter* com2 = new QFileCompleter(this, true);
  sourceDirectoryLE->setCompleter(com2);
  QObject::connect(com2, SIGNAL(activated(const QString &)), this, SLOT(on_sourceDirectoryLE_textChanged(const QString &)));

  QFileCompleter* com3 = new QFileCompleter(this, true);
  outputDirectoryLE->setCompleter(com3);
  QObject::connect(com3, SIGNAL(activated(const QString &)), this, SLOT(on_outputDirectoryLE_textChanged(const QString &)));


  m_QueueDialog = new ProcessQueueDialog();
  m_QueueDialog->setVisible(false);

  // Configure the Histogram Plot
  m_HistogramPlot->setCanvasBackground(QColor(Qt::white));
  m_HistogramPlot->setTitle("Histogram");
  //  m_HistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Scale Value");
  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->enableYMin(true);
  m_grid->setMajPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinPen(QPen(Qt::lightGray, 0, Qt::DotLine));
//  m_grid->attach(m_HistogramPlot);

#if 0
  m_zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, m_HistogramPlot->canvas());
  m_zoomer->setRubberBand(QwtPicker::RectRubberBand);
  m_zoomer->setRubberBandPen(QColor(Qt::green));
  m_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
  m_zoomer->setTrackerPen(QColor(Qt::blue));

  m_panner = new QwtPlotPanner(m_HistogramPlot->canvas());
  m_panner->setMouseButton(Qt::MidButton);

  m_picker
  = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection | QwtPicker::DragSelection, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_HistogramPlot->canvas());
  m_picker->setRubberBandPen(QColor(Qt::green));
  m_picker->setRubberBand(QwtPicker::CrossRubberBand);
  m_picker->setTrackerPen(QColor(Qt::blue));
#endif

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
char* EmMpmGui::copyStringToNewBuffer(const QString &fname)
{
  std::string::size_type size = fname.size() + 1;
  char* buf = NULL;
  if (size > 1)
  {
    buf = (char*)malloc(size);
    ::memset(buf, 0, size);
    strncpy(buf, fname.toAscii().constData(), size - 1);
  }
  return buf;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyGrayValues( EMMPM_Data* inputs)
{
  QList<UserInitArea*> uias = m_UserInitAreaTableModel->getUserInitAreas();
  int size = uias.count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = uias[r];
    inputs->grayTable[uia->getEmMpmClass()] = uia->getEmMpmGrayLevel();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyInitCoords( EMMPM_Data* inputs)
{
  QList<UserInitArea*> uias = m_UserInitAreaTableModel->getUserInitAreas();
  int size = uias.count();
  UserInitArea* uia = NULL;
  unsigned int* cPtr = inputs->initCoords[0];
  for (int r = 0; r < size; ++r)
  {
    uia = uias[r];
    cPtr = inputs->initCoords[uia->getEmMpmClass()];
    uia->getUpperLeft( cPtr[0], cPtr[1]);
    uia->getLowerRight( cPtr[2], cPtr[3] );
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyIntializationValues(EMMPM_Data* inputs)
{
  QList<UserInitArea*> uias = m_UserInitAreaTableModel->getUserInitAreas();
  int size = uias.count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = uias[r];
    inputs->m[r] = uia->getMu();
    inputs->v[r] = uia->getSigma();
 //   std::cout << "Initializing with Mu:" << inputs->m[r] << "  Sigma: " << inputs->v[r] << std::endl;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyGammaValues(EMMPM_Data* inputs)
{
  QList<UserInitArea*> uias = m_UserInitAreaTableModel->getUserInitAreas();
  int size = uias.count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = uias[r];
    inputs->w_gamma[r] = uia->getGamma();
 //   std::cout << "Initializing with Gamma:" << inputs->w_gamma[r] << std::endl;
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMUpdateStats(EMMPM_Data* data)
{
  // Check to make sure we are at the end of an em loop
  if ( data->inside_mpm_loop == 0 && NULL != data->outputImage)
  {
    char buff[256];
    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/emmpm_out_%d.tif", data->currentEMLoop);
    int err = EMMPM_WriteGrayScaleImage(buff, data->rows, data->columns, "Intermediate Image", data->outputImage);
    if (err < 0)
    {
      std::cout << "Error writing intermediate tiff image." << std::endl;
    }

    std::cout << "Class\tMu\tSigma" << std::endl;
    for (int l = 0; l < data->classes; l++)
    {
      //    snprintf(msgbuff, 256, "%d\t%.3f\t%.3f", l, data->m[l], data->v[l]);
      //    EMMPM_ShowProgress(msgbuff, data->progress);
      std::cout << l << "\t" << data->m[l] << "\t" << data->v[l] << "\t" << std::endl;
    }

    double hist[EMMPM_MAX_CLASSES][256];
    // Generate a gaussian curve for each class based off the mu and sigma for that class
    for (int c = 0; c < data->classes; ++c)
    {
      double mu = data->m[c];
      double sig = data->v[c];
      double twoSigSqrd = sig * sig * 2.0f;
      double constant = 1.0f / (sig * sqrtf(2.0f * M_PI));
      for (size_t x = 0; x < 256; ++x)
      {
        hist[c][x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
      }
    }

    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/emmpm_hist_%d.csv", data->currentEMLoop);
    std::ofstream file(buff, std::ios::out | std::ios::binary);
    if (file.is_open())
    {
      for (size_t x = 0; x < 256; ++x)
      {
        file << x;
        for (int c = 0; c < data->classes; ++c)
        {
          file << ", " << hist[c][x];
        }
        file << std::endl;
      }
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_processBtn_clicked()
{

  /* If the 'processFolder' checkbox is checked then we need to check for some
   * additional inputs
   */
  if (this->processFolder->isChecked())
  {
    if (this->sourceDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Input Parameter Error"), tr("Source Directory must be set."), QMessageBox::Ok);
      return;
    }

    if (this->outputDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Parameter Error"), tr("Output Directory must be set."), QMessageBox::Ok);
      return;
    }

    if (this->fileListView->model()->rowCount() == 0)
    {
      QMessageBox::critical(this, tr("Parameter Error"), tr("No image files are available in the file list view."), QMessageBox::Ok);
      return;
    }

    QDir outputDir(this->outputDirectoryLE->text());
    if (outputDir.exists() == false)
    {
      bool ok = outputDir.mkpath(".");
      if (ok == false)
      {
        QMessageBox::critical(this, tr("Output Directory Creation"), tr("The output directory could not be created."), QMessageBox::Ok);
        return;
      }
    }

  }
  else
  {
    QFileInfo fi(inputImageFilePath->text());
    if (fi.exists() == false)
    {
      QMessageBox::critical(this, tr("Fixed Image File Error"), tr("Fixed Image does not exist. Please check the path."), QMessageBox::Ok);
      return;
    }

    if (outputImageFile->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Image File Error"), tr("Please select a file name for the registered image to be saved as."), QMessageBox::Ok);
      return;
    }
    QFile file(outputImageFile->text());
    if (file.exists() == true)
    {
      int ret = QMessageBox::warning(this, tr("QEM/MPM"),
                                     tr("The Output File Already Exists\nDo you want to over write the existing file?"),
                                     QMessageBox::No | QMessageBox::Default,
                                     QMessageBox::Yes,
                                     QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel)
      {
        return;
      }
      else if (ret == QMessageBox::Yes)
      {
        setOutputExistsCheck(true);
      }
      else
      {
        QString outputFile = getOpenDialogLastDirectory() + QDir::separator() + "Untitled.tif";
        outputFile = QFileDialog::getSaveFileName(this, tr("Save Output File As ..."), outputFile, tr("TIF (*.tif)"));
        if (!outputFile.isNull())
        {
          setCurrentProcessedFile("");
          setOutputExistsCheck(true);
        }
        else // The user clicked cancel from the save file dialog

        {
          return;
        }
      }
    }
  }

  getQueueDialog()->clearTable();
  if (getQueueController() != NULL)
  {
    getQueueController()->deleteLater();
  }
  ProcessQueueController* queueController = new ProcessQueueController(this);
  setQueueController(queueController);
  bool ok;

  InputOutputFilePairList filepairs;

  if (this->processFolder->isChecked() == false)
  {
    EMMPMTask* task = new EMMPMTask(NULL);
    EMMPM_Data* data = task->getEMMPM_Data();
    data->in_beta = m_Beta->text().toFloat(&ok);
    //data->in_gamma = m_Gamma->text().toFloat(&ok);
    for(int i = 0; i < EMMPM_MAX_CLASSES; i++) {
      data->w_gamma[i] =  m_Gamma->text().toFloat(&ok);
    }
    data->mpmIterations = m_MpmIterations->value();
    data->emIterations = m_EmIterations->value();
    data->classes = m_NumClasses->value();
    data->simulatedAnnealing = (useSimulatedAnnealing->isChecked()) ? 1 : 0;
    if (m_UserInitAreaTableModel->rowCount() == 0)
    {
      data->initType = EMMPM_Basic;
      int n = data->classes - 1;
      for (int value = 0; value < data->classes; ++value)
      {
        data->grayTable[value] = value * 255 / n;
      }
    }
    else
    {
      data->initType = EMMPM_Manual;
      copyGrayValues(data);
      copyInitCoords(data);
      copyIntializationValues(data);
      copyGammaValues(data);
    }
    data->input_file_name = copyStringToNewBuffer(inputImageFilePath->text());
    data->output_file_name = copyStringToNewBuffer(outputImageFile->text());
    task->setInputFilePath(inputImageFilePath->text());
    task->setOutputFilePath(outputImageFile->text());
    queueController->addTask(static_cast<QThread* > (task));
    connect(task, SIGNAL(updateImageAvailable(QImage)),
            m_GraphicsView, SLOT(setOverlayImage(QImage)));
    this->addProcess(task);
  }
  else
  {
    QStringList fileList = generateInputFileList();
    int32_t count = fileList.count();
    for (int32_t i = 0; i < count; ++i)
    {
      //  std::cout << "Adding input file:" << fileList.at(i).toStdString() << std::endl;
      EMMPMTask* task = new EMMPMTask(NULL);
      EMMPM_Data* data = task->getEMMPM_Data();
      data->emIterations = m_EmIterations->value();
      data->mpmIterations = m_MpmIterations->value();
      data->in_beta = m_Beta->text().toFloat(&ok);
      //data->in_gamma = m_Gamma->text().toFloat(&ok);
      for(int j = 0; j < EMMPM_MAX_CLASSES; j++) {
        data->w_gamma[j] =  m_Gamma->text().toFloat(&ok);
      }
      data->classes = m_NumClasses->value();
      data->simulatedAnnealing = (useSimulatedAnnealing->isChecked()) ? 1 : 0;
      if (m_UserInitAreaTableModel->rowCount() == 0)
      {
        data->initType = EMMPM_Basic;
        int n = data->classes - 1;
        for (int value = 0; value < data->classes; ++value)
        {
          data->grayTable[value] = value * 255 / n;
        }
      }
      else
      {
        data->initType = EMMPM_Manual;
        copyGrayValues(data);
        copyInitCoords(data);
        copyIntializationValues(data);
        copyGammaValues(data);
      }

      task->setInputFilePath(sourceDirectoryLE->text() + QDir::separator() + fileList.at(i));
      QFileInfo fileInfo(fileList.at(i));
      QString basename = fileInfo.completeBaseName();
      QString extension = fileInfo.suffix();
      QString filepath = outputDirectoryLE->text();
      filepath.append(QDir::separator());
      filepath.append(outputPrefix->text());
      filepath.append(basename);
      filepath.append(outputSuffix->text());
      filepath.append(".");
      filepath.append(outputImageType->currentText());
      task->setOutputFilePath(filepath);

      data->input_file_name = copyStringToNewBuffer(task->getInputFilePath());
      data->output_file_name = copyStringToNewBuffer(task->getOutputFilePath());

      filepairs.append(InputOutputFilePair(task->getInputFilePath(), task->getOutputFilePath()));
      queueController->addTask(static_cast<QThread* > (task));
      if (i == 0)
      {
        connect(task, SIGNAL(updateImageAvailable(QImage)),
                m_GraphicsView, SLOT(setOverlayImage(QImage)));
      }
      this->addProcess(task);
    }

  }
  setInputOutputFilePairList(filepairs);

  // When the event loop of the controller starts it will signal the ProcessQueue to run
  connect(queueController, SIGNAL(started()), queueController, SLOT(processTask()));
  // When the QueueController finishes it will signal the QueueController to 'quit', thus stopping the thread
  connect(queueController, SIGNAL(finished()), this, SLOT(queueControllerFinished()));

  connect(queueController, SIGNAL(started()), this, SLOT(processingStarted()));
  connect(queueController, SIGNAL(finished()), this, SLOT(processingFinished()));

//  getQueueDialog()->setParent(this);
  getQueueDialog()->setVisible(true);

  queueController->start();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::addProcess(EMMPMTask* task)
{
  getQueueDialog()->addProcess(task);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::processingStarted()
{
//  std::cout << "EmMpmGui::processingStarted()" << std::endl;
  processBtn->setText("Cancel");
  processBtn->setEnabled(false);
  this->statusBar()->showMessage("Processing Images...");
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::processingFinished()
{
//  std::cout << "IPHelper::processingFinished()" << std::endl;
  /* Code that cleans up anything from the processing */
  processBtn->setText("Segment");
  processBtn->setEnabled(true);
  this->statusBar()->showMessage("Processing Complete");

  // Get the image files from the plugin

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::queueControllerFinished()
{
  getQueueDialog()->setVisible(false);
  if (this->processFolder->isChecked() == false)
  {
    setCurrentImageFile (inputImageFilePath->text());
    setCurrentProcessedFile(outputImageFile->text());
    m_GraphicsView->loadOverlayImageFile(outputImageFile->text());
  }
  else
  {
    QStringList fileList = generateInputFileList();

    setCurrentImageFile (sourceDirectoryLE->text() + QDir::separator() + fileList.at(0) );
    std::cout << "Setting current Image file: " << getCurrentImageFile().toStdString() << std::endl;
    QFileInfo fileInfo(fileList.at(0));
    QString basename = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    QString filepath = outputDirectoryLE->text();
    filepath.append(QDir::separator());
    filepath.append(outputPrefix->text());
    filepath.append(basename);
    filepath.append(outputSuffix->text());
    filepath.append(".");
    filepath.append(outputImageType->currentText());
    setCurrentProcessedFile(filepath);
    std::cout << "Setting processed Image file: " << filepath.toStdString() << std::endl;
  }

  setWidgetListEnabled(true);

  getQueueController()->deleteLater();
  setQueueController(NULL);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_processFolder_stateChanged(int state)
{
  bool enabled = true;
  if (state == Qt::Unchecked)
  {
    enabled = false;
  }

  sourceDirectoryLE->setEnabled(enabled);
  sourceDirectoryBtn->setEnabled(enabled);
  outputDirectoryLE->setEnabled(enabled);
  outputDirectoryBtn->setEnabled(enabled);
  outputPrefix->setEnabled(enabled);
  outputSuffix->setEnabled(enabled);
  filterPatternLabel->setEnabled(enabled);
  filterPatternLineEdit->setEnabled(enabled);
  fileListView->setEnabled(enabled);
  outputImageTypeLabel->setEnabled(enabled);
  outputImageType->setEnabled(enabled);

  inputImageFilePath->setEnabled(!enabled);
  inputImageFilePathBtn->setEnabled(!enabled);

  outputImageFile->setEnabled(!enabled);
  outputImageButton->setEnabled(!enabled);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_filterPatternLineEdit_textChanged()
{
  // std::cout << "filterPattern: " << std::endl;
  getProxyModel()->setFilterFixedString(filterPatternLineEdit->text());
  getProxyModel()->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputPrefix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputSuffix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputImageType_currentIndexChanged(int index)
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_sourceDirectoryBtn_clicked()
{
  QString aDir = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), getOpenDialogLastDirectory(), QFileDialog::ShowDirsOnly
          | QFileDialog::DontResolveSymlinks);
  setOpenDialogLastDirectory(aDir);
  if (!getOpenDialogLastDirectory().isNull())
  {
    this->sourceDirectoryLE->setText(getOpenDialogLastDirectory() );
  }
  populateFileTable(sourceDirectoryLE, fileListView);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputDirectoryBtn_clicked()
{
  bool canWrite = false;
  QString aDir = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), getOpenDialogLastDirectory(),
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  setOpenDialogLastDirectory(aDir);
  if (!getOpenDialogLastDirectory().isNull())
  {
    QFileInfo fi(aDir);
    canWrite = fi.isWritable();
    if (canWrite) {
      this->outputDirectoryLE->setText(getOpenDialogLastDirectory());
    }
    else
    {
      QMessageBox::critical(this, tr("Output Directory Selection Error"),
                            tr("The Output directory is not writable by your user. Please make it writeable by changing the permissions or select another directory"),
                            QMessageBox::Ok);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_inputImageFilePathBtn_clicked()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile =
      QFileDialog::getOpenFileName(this, tr("Select Fixed Image"), getOpenDialogLastDirectory(), tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)"));

  if (true == imageFile.isEmpty())
  {
    return;
  }
  inputImageFilePath->setText(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputImageButton_clicked()
{
  QString outputFile = getOpenDialogLastDirectory() + QDir::separator() + "Untitled.tif";
  outputFile = QFileDialog::getSaveFileName(this, tr("Save Output File As ..."), outputFile, tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)"));
  if (outputFile.isEmpty())
  {
    return;
  }

  QFileInfo fi(outputFile);
  QFileInfo fi2(fi.absolutePath());
  if (fi2.isWritable() == true) {
    outputImageFile->setText(outputFile);
  }
  else
  {
    QMessageBox::critical(this, tr("Output Image File Error"),
                          tr("The parent directory of the output image is not writable by your user. Please make it writeable by changing the permissions or select another directory"),
                          QMessageBox::Ok);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_inputImageFilePath_textChanged(const QString & text)
{
  verifyPathExists(inputImageFilePath->text(), inputImageFilePath);
  openBaseImageFile(text);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputImageFile_textChanged(const QString & text)
{
  //  verifyPathExists(outputImageFile->text(), movingImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_sourceDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE);
  this->populateFileTable(sourceDirectoryLE, fileListView);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_outputDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(outputDirectoryLE->text(), outputDirectoryLE);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_imageDisplayCombo_currentIndexChanged()
{
  if (imageDisplayCombo->currentIndex() == EmMpm_Constants::CompositedImage)
  {
    compositeModeCB->setEnabled(true);
  }
  else
  {
    compositeModeCB->setEnabled(false);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_compositeModeCB_currentIndexChanged()
{
  int index = compositeModeCB->currentIndex();
  m_GraphicsView->setCompositeMode(index);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
  {
    w->setEnabled(b);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EmMpmGui::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EmMpmGui::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
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
qint32 EmMpmGui::checkDirtyDocument()
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
void EmMpmGui::updateBaseRecentFileList(const QString &file)
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
      connect(action, SIGNAL(triggered()), this, SLOT(openRecentBaseImageFile()));
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::openRecentBaseImageFile()
{
  //std::cout << "QRecentFileList::openRecentFile()" << std::endl;

  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
  {
    //std::cout << "Opening Recent file: " << action->data().toString().toStdString() << std::endl;
    QString file = action->data().toString();
    openBaseImageFile( file );
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::openBaseImageFile(QString imageFile)
{
  if ( true == imageFile.isEmpty() ) // User cancelled the operation
  {
    return;
  }
  m_GraphicsView->loadBaseImageFile(imageFile);

  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
  updateBaseRecentFileList(imageFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionOpenBaseImage_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile = QFileDialog::getOpenFileName(this, tr("Open Image File"),
    m_OpenDialogLastDirectory,
    tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)") );

  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openBaseImageFile(imageFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionOpenOverlayImage_triggered()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile = QFileDialog::getOpenFileName(this, tr("Open Segmented Image File"),
    m_OpenDialogLastDirectory,
    tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)") );

  if ( true == imageFile.isEmpty() )
  {
    return;
  }
  openOverlayImage(imageFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionSaveCanvas_triggered()
{
  QImage image = m_GraphicsView->getOverlayImage();
  if (imageDisplayCombo->currentIndex() == EmMpm_Constants::CompositedImage)
  {
    image = m_GraphicsView->getCompositedImage();
  }
  int err = 0;
  if (outputImageFile->text().isEmpty())
  {
    QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator() + "Segmented.tif";
    outputFile = QFileDialog::getSaveFileName(this, tr("Save Processed Image As ..."), outputFile, tr("Images (*.tif *.bmp *.jpg *.png)"));
    if ( !outputFile.isEmpty() )
    {
      outputImageFile->setText(outputFile);
    }
    else {
      return;
    }
  }

  bool ok = image.save(outputImageFile->text());
  if (ok == true) {

  //TODO: Set a window title or something
  }
  else
  {
    //TODO: Add in a GUI dialog to help explain the error or give suggestions.
    err = -1;
  }
  this->setWindowModified(false);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionAbout_triggered()
{
  ApplicationAboutBoxDialog about(EIMApps::LicenseList, this);
  QString an = QCoreApplication::applicationName();
  QString version("");
  version.append(EmMpm_Gui::Version::PackageComplete.c_str());
  about.setApplicationInfo(an, version);
  about.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionClose_triggered() {
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
void EmMpmGui::on_actionExit_triggered()
{
  this->close();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::openOverlayImage(QString processedImage)
{
  if ( true == processedImage.isEmpty() ) // User cancelled the operation
  {
    return;
  }
  m_GraphicsView->loadOverlayImageFile(processedImage);
  setWidgetListEnabled(true);

  updateBaseRecentFileList(processedImage);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::baseImageFileLoaded(const QString &filename)
{
 // std::cout << "Loaded Image file " << filename.toStdString() << std::endl;
  this->setWindowFilePath(filename);
  imageDisplayCombo->setCurrentIndex(EmMpm_Constants::OriginalImage);
  inputImageFilePath->setText(filename);
  plotImageHistogram();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::overlayImageFileLoaded(const QString &filename)
{
//  std::cout << "EmMpmGui::overlayImageFileLoaded" << std::endl;
  imageDisplayCombo->setCurrentIndex(EmMpm_Constants::SegmentedImage);
  outputImageFile->setText(filename);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::plotImageHistogram()
{
  QImage image = m_GraphicsView->getBaseImage();
  const int numValues = 256;

  // Generate the Histogram Bins
  QwtArray<double> intervals(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
  }
  QwtArray<double> values(numValues);

  qint32 height = image.height();
  qint32 width = image.width();
  QRgb rgbPixel;
  int gray;
  qint32 index;
  double max = std::numeric_limits<double>::min();

  for (qint32 y = 0; y < height; y++)
  {
    for (qint32 x = 0; x < width; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      values[gray]++;
      if (values[gray] > max) { max = values[gray]; }
    }
  }

  if (NULL == m_histogram)
  {
    m_histogram = new QwtPlotCurve("Original Image");
    m_histogram->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_histogram->setPen(QPen(Qt::red));
    m_histogram->attach(m_HistogramPlot);
  }
  m_histogram->setData(intervals, values);

  m_HistogramPlot->setAxisScale(QwtPlot::yLeft, 0.0, max);
  m_HistogramPlot->setAxisScale(QwtPlot::xBottom, 0.0, 255.0);
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaUpdated(UserInitArea* uia)
{
//  std::cout << "UserInitArea Updated" << std::endl;
  if (NULL == uia)
  {
    return;
  }
  QPoint p = uia->pos().toPoint();
  QRect b = uia->boundingRect().toAlignedRect();

  QPoint upLeft(b.x() + p.x(), b.y() + p.y());
  QPoint lowRight(b.x() + p.x() + b.width(), b.y() + p.y() + b.height());

  QImage image = m_GraphicsView->getBaseImage();
  qint32 height = image.height();
  qint32 width = image.width();
  QRgb rgbPixel;
  int gray;
  qint32 index;
  double max = std::numeric_limits<double>::min();

  int xStart = b.x() + p.x();
  int xEnd = b.x() + p.x() + b.width();
  int yStart = b.y() + p.y();
  int yEnd = b.y() + p.y() + b.height();

  double mu, sig;
  mu = 0.0;
  sig = 0.0;
  //Calculate Mu
  for (qint32 y = yStart; y < yEnd; y++)
  {
    for (qint32 x = xStart; x < xEnd; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      mu += gray;
    }
  }
  mu /= ((yEnd - yStart)*(xEnd - xStart));
  uia->setMu(mu);

  // Calculate Sigma
  for (qint32 y = yStart; y < yEnd; y++)
  {
    for (qint32 x = xStart; x < xEnd; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      sig += (gray-mu)*(gray-mu);
    }
  }
  sig /= ((yEnd - yStart)*(xEnd - xStart));
  sig = sqrt(sig);
  uia->setSigma(sig);

  m_UserInitAreaTableModel->updateUserInitArea(uia);
#if 0
  // Generate the Histogram Bins
  const int numValues = 256;
  QwtArray<double > intervals(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
  }
  QwtArray<double > values(numValues);
  float twoSigSqrd = sig * sig * 2.0f;
  float constant = 1.0 / (sig * sqrtf(2.0f * M_PI));
  size_t max_index = 0;
  for (size_t x = 0; x < 256; ++x)
  {
    values[x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
    if (values[x] > max) { max = values[x]; max_index = x; }
  }



  // Locate our curve object by getting the row from the TableModel that corresponds
  // to the UIA object that was passed in
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);
  QwtPlotCurve* curve = m_UIAGaussians[row];
  curve->setData(intervals, values);
  QColor c = uia->getColor();
  curve->setPen(QPen(c));

  // Draw a single vertical line centered on the average gray value
  double binSize = m_histogram->y( max_index );
  for (size_t x = 0; x < 256; ++x)
  {
    if (x == max_index) {
      values[x] = (values[x]/max) * binSize;
    }
    else
    {
      values[x] = 0;
    }
  }
#endif
  const int numValues = 256;
  QwtArray<double> values(numValues);
  float twoSigSqrd = sig * sig * 2.0f;
  float constant = 1.0 / (sig * sqrtf(2.0f * M_PI));
  size_t max_index = 0;
  for (size_t x = 0; x < 256; ++x)
  {
    values[x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
    if (values[x] > max) { max = values[x]; max_index = x; }
  }
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);

  QwtPlotMarker* marker = m_UIAMarkers[row];
  marker->attach(m_HistogramPlot);
  marker->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
  marker->setLabelOrientation(Qt::Vertical);
  marker->setLineStyle(QwtPlotMarker::VLine);
  QColor c = uia->getColor();
  marker->setLinePen(QPen(c));
  marker->setXValue(max_index);

 // m_HistogramPlot->setAxisScale(QwtPlot::yLeft, 0.0, 10000);
  m_HistogramPlot->setAxisScale(QwtPlot::xBottom, 0.0, 255.0);
  m_HistogramPlot->replot();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::deleteUserInitArea(UserInitArea* uia)
{
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);
#if 0
  QwtPlotCurve* curve = m_UIAGaussians[row];
  m_UIAGaussians.removeAll(curve);
#else
  QwtPlotMarker* curve = m_UIAMarkers[row];
  m_UIAMarkers.removeAll(curve);
#endif
  curve->detach();
  delete curve; // Clean up the memory
  m_HistogramPlot->replot();
  m_NumClasses->setValue(userInitAreas.size()-1);
  if (userInitAreas.size()-1 == 0)
  {
    m_NumClasses->setEnabled(true);
    m_Gamma->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaAdded(UserInitArea* uia)
{
//  std::cout << "EmMpmGui::userInitAreaAdded(UserInitArea* uia)" << std::endl;
  if (NULL == uia) { return; }
  addUserInitArea->toggle();
  QColor color = uia->getColor();
#if 0
  QwtPlotCurve* curve = new QwtPlotCurve("User Init Area");
  curve->setPen(QPen(color));
#else
  QwtPlotMarker* curve = new QwtPlotMarker();
  curve->setLinePen(color);
#endif
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);


  curve->attach(m_HistogramPlot);

  // Figure out the proper row to insert the curve object to keep it in sync with
  // the table model
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);
  m_NumClasses->setValue(userInitAreas.size());
  if (userInitAreas.size() != 0)
  {
    m_NumClasses->setEnabled(false);
    m_Gamma->setEnabled(false);
  }

#if 0
  m_UIAGaussians.insert(row, curve);
#else
  m_UIAMarkers.insert(row, curve);
#endif

  // Now update the curve with the initial data
  userInitAreaUpdated(uia);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_fitToWindow_clicked()
{
  zoomCB->blockSignals(true);
  zoomCB->setCurrentIndex(zoomCB->count()-1);
  zoomCB->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::populateFileTable(QLineEdit* sourceDirectoryLE, QListView *fileListView)
{
  if (NULL == m_ProxyModel)
  {
    m_ProxyModel = new QSortFilterProxyModel(this);
  }

  QDir sourceDir(sourceDirectoryLE->text());
  sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList strList = sourceDir.entryList();
  QAbstractItemModel* strModel = new QStringListModel(strList, this->m_ProxyModel);
  m_ProxyModel->setSourceModel(strModel);
  m_ProxyModel->setDynamicSortFilter(true);
  m_ProxyModel->setFilterKeyColumn(0);
  fileListView->setModel(m_ProxyModel);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList EmMpmGui::generateInputFileList()
{
  QStringList list;
  int count = this->m_ProxyModel->rowCount();
  // this->fileListView->selectAll();
  QAbstractItemModel* sourceModel = this->m_ProxyModel->sourceModel();
  for (int i = 0; i < count; ++i)
  {
    QModelIndex proxyIndex = this->m_ProxyModel->index(i,0);
    QModelIndex sourceIndex = this->m_ProxyModel->mapToSource(proxyIndex);
    list.append( sourceModel->data(sourceIndex, 0).toString() );
  }
  return list;
}
