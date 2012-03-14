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
#include <QtGui/QDoubleValidator>

//-- Qwt Includes
#include <qwt.h>
#include <qwt_data.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_interval_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_scale_div.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>

// Our Project wide includes
#include "QtSupport/ApplicationAboutBoxDialog.h"
#include "QtSupport/QRecentFileList.h"
#include "QtSupport/QFileCompleter.h"
#include "QtSupport/ImageGraphicsDelegate.h"
#include "QtSupport/ProcessQueueController.h"
#include "QtSupport/ProcessQueueDialog.h"

#include "MXA/Common/MXAMemory.h"

//
#include "EMMPMLib/EMMPMLib.h"
#include "EMMPMLib/Common/MSVCDefines.h"
#include "EMMPMLib/public/InitializationFunctions.h"
#include "EMMPMLib/public/ProgressFunctions.h"
#include "EMMPMLib/tiff/EMTiffIO.h"
#include "EMMPMLib/public/EMMPM_Structures.h"
#include "EMMPMLib/public/EMMPM.h"

//
#include "EmMpmGuiVersion.h"
#include "UserInitArea.h"
#include "EMMPMTask.h"
#include "AxisSettingsDialog.h"
#include "License/EmMpmGuiLicenseFiles.h"
#include "UserInitAreaWidget.h"
#include "LayersDockWidget.h"
#include "ImageOpenDialog.h"



#define READ_STRING_SETTING(prefs, var, emptyValue)\
  var->setText( prefs.value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }


#define READ_SETTING(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs.value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var->setValue(temp);

#define READ_VALUE(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs.value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var = temp;

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

#define WRITE_CHECKBOX_SETTING(prefs, var)\
    prefs.setValue(#var, var->isChecked() );

#define WRITE_VALUE(prefs, var)\
    prefs.setValue(#var, var);

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
m_CombinedGaussians(NULL),
m_ShowCombinedGaussians(false),
m_ProxyModel(NULL),
m_OutputExistsCheck(false),
m_QueueController(NULL),
m_LayersPalette(NULL),
#if defined(Q_WS_WIN)
m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{

  setupUi(this);
  setupGui();

#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif
  readIOSettings(prefs);
  readSettings(prefs);
  readWindowSettings(prefs);

  QRecentFileList* recentFileList = QRecentFileList::instance();
  connect(recentFileList, SIGNAL (fileListChanged(const QString &)), this, SLOT(updateBaseRecentFileList(const QString &)));
  // Get out initial Recent File List
  this->updateBaseRecentFileList(QString::null);
  qRegisterMetaType<QVector<double> >("QVector<double>");
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
#if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationDomain(), QCoreApplication::applicationName());
#endif
    writeIOSettings(prefs);
    writeSettings(prefs);
    writeWindowSettings(prefs);
    event->accept();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::writeIOSettings(QSettings &prefs)
{
  prefs.beginGroup("Input_Output");
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
void EmMpmGui::readIOSettings(QSettings &prefs)
{
  prefs.beginGroup("Input_Output");
//  inputImageFilePath->blockSignals(true);
//  READ_STRING_SETTING(prefs, inputImageFilePath, "");
//  inputImageFilePath->blockSignals(false);
  READ_STRING_SETTING(prefs, outputImageFile, "");

  READ_BOOL_SETTING(prefs, processFolder, false);
  READ_STRING_SETTING(prefs, sourceDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputPrefix, "Segmented_");
  READ_STRING_SETTING(prefs, outputSuffix, "");
  prefs.endGroup();


  on_processFolder_stateChanged(processFolder->checkState());
  if (this->sourceDirectoryLE->text().isEmpty() == false)
  {
    this->populateFileTable(this->sourceDirectoryLE, this->fileListView);
  }
}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void EmMpmGui::readSettings(QSettings &prefs)
{
  QString val;
  bool ok;
  qint32 i;
  double d;
  int userInitAreaCount;
  prefs.beginGroup("Parameters");
  READ_SETTING(prefs, m_NumClasses, ok, i, 2, Int);
  READ_SETTING(prefs, m_EmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_MpmIterations, ok, i, 5, Int);
  READ_STRING_SETTING(prefs, m_Beta, "0.5");
  READ_STRING_SETTING(prefs, m_MinVariance, "20");

  READ_BOOL_SETTING(prefs, useSimulatedAnnealing, false);

  READ_BOOL_SETTING(prefs, useGradientPenalty, false);
  READ_SETTING(prefs, gradientBetaE, ok, d, 1.0, Double);

  READ_BOOL_SETTING(prefs, useCurvaturePenalty, false);
  READ_SETTING(prefs, curvatureBetaC, ok, d, 1.0, Double);
  READ_SETTING(prefs, curvatureRMax, ok, d, 15.0, Double);
  READ_SETTING(prefs, ccostLoopDelay, ok, i, 1, Int);
  READ_VALUE(prefs, userInitAreaCount, ok, i, 0, Int);
  enableUserDefinedAreas->blockSignals(true);
  READ_BOOL_SETTING(prefs, enableUserDefinedAreas, false);
  enableUserDefinedAreas->blockSignals(false);

  prefs.endGroup();

  // We only load the User Init Areas if there is an image loaded
  // and the checkbox was set
  if (m_GraphicsView != NULL
      && m_GraphicsView->getBaseImage().isNull() == false
      && enableUserDefinedAreas->isChecked() )
  {
    addUserInitArea->setEnabled(true);
    UserInitArea::deleteAllUserInitAreas(m_GraphicsView->scene());
    m_UserInitAreaVector->clear();
    for (int i = 0; i < userInitAreaCount; ++i)
    {
      UserInitArea* uia = UserInitArea::NewUserInitArea(prefs, i);
      //Calculate the Mean and Standard Deviation
      double mu = 0.0;
      double sig = 0.0;
      int err = m_GraphicsView->calculateMuSigma(uia, mu, sig);
      uia->setMu(mu);
      uia->setSigma(sig);
      addUserInitArea->toggle();
      m_GraphicsView->addNewInitArea(uia);
    }
  }
}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void EmMpmGui::writeSettings(QSettings &prefs)
{

  prefs.beginGroup("Parameters");
  WRITE_SETTING(prefs, m_NumClasses);
  WRITE_SETTING(prefs, m_EmIterations);
  WRITE_SETTING(prefs, m_MpmIterations);
  WRITE_STRING_SETTING(prefs, m_Beta);
  WRITE_STRING_SETTING(prefs, m_MinVariance);

  WRITE_CHECKBOX_SETTING(prefs, useSimulatedAnnealing);
  WRITE_CHECKBOX_SETTING(prefs, useGradientPenalty);
  WRITE_SETTING(prefs, gradientBetaE);
  WRITE_CHECKBOX_SETTING(prefs, useCurvaturePenalty);
  WRITE_SETTING(prefs, curvatureBetaC);
  WRITE_SETTING(prefs, curvatureRMax);
  WRITE_SETTING(prefs, ccostLoopDelay);

  WRITE_CHECKBOX_SETTING(prefs, enableUserDefinedAreas);
  int userInitAreaCount = this->m_UserInitAreaVector->size();
  WRITE_VALUE(prefs, userInitAreaCount);
  prefs.endGroup();


  for (int i = 0; i < this->m_UserInitAreaVector->size(); ++i)
  {
    UserInitArea* uia = m_UserInitAreaVector->at(i);
    uia->writeSettings(prefs);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::readWindowSettings(QSettings &prefs)
{
  bool ok = false;
  prefs.beginGroup("WindodwSettings");
  if (prefs.contains(QString("Geometry")) )
  {
    QByteArray geo_data = prefs.value(QString("Geometry")).toByteArray();
    ok = restoreGeometry(geo_data);
  }

  if (prefs.contains(QString("Layout")))
  {
    QByteArray layout_data = prefs.value(QString("Layout")).toByteArray();
    restoreState(layout_data);
  }
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::writeWindowSettings(QSettings &prefs)
{
  prefs.beginGroup("WindodwSettings");
  QByteArray geo_data = saveGeometry();
  QByteArray layout_data = saveState();
  prefs.setValue(QString("Geometry"), geo_data);
  prefs.setValue(QString("Layout"), layout_data);
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionSave_Config_File_triggered()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "EMMPM-Config.txt";
  QString file = QFileDialog::getSaveFileName(this, tr("Save EM/MPM Configuration"),
                                              proposedFile,
                                              tr("*.txt") );
  if ( true == file.isEmpty() ){ return;  }
  QFileInfo fi(file);
  m_OpenDialogLastDirectory = fi.absolutePath();
  QSettings prefs(file, QSettings::IniFormat, this);
  writeSettings(prefs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionLoad_Config_File_triggered()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select Configuration File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Configuration File (*.txt)") );
  if ( true == file.isEmpty() ){return;  }
  QFileInfo fi(file);
  m_OpenDialogLastDirectory = fi.absolutePath();
  QSettings prefs(file, QSettings::IniFormat, this);
  readSettings(prefs);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
#define ZOOM_MENU(var, menu, slot)\
  {\
  QAction* action = new QAction(menu);\
  action->setText( #var );\
  QString actionName("action_z" #var "Action");\
  action->setObjectName(actionName);\
  zoomMenu->addAction(action);\
  connect(action, SIGNAL(triggered()), this, SLOT(slot())); \
}

#define ZOOM_MENU_SLOT_DEF(var, index)\
void EmMpmGui::z##var##_triggered() {\
  zoomButton->setText(#var " % ");\
  m_GraphicsView->setZoomIndex(index);\
}

ZOOM_MENU_SLOT_DEF(10, 0);
ZOOM_MENU_SLOT_DEF(25, 1);
ZOOM_MENU_SLOT_DEF(50, 2);
ZOOM_MENU_SLOT_DEF(100, 3);
ZOOM_MENU_SLOT_DEF(125, 4);
ZOOM_MENU_SLOT_DEF(150, 5);
ZOOM_MENU_SLOT_DEF(200, 6);
ZOOM_MENU_SLOT_DEF(400, 7);
ZOOM_MENU_SLOT_DEF(600, 8);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_fitToWindow_clicked()
{
  m_GraphicsView->setZoomIndex(9);
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

  QMenu* zoomMenu = new QMenu(this);
  ZOOM_MENU(10, zoomMenu, z10_triggered);
  ZOOM_MENU(25, zoomMenu, z25_triggered);
  ZOOM_MENU(50, zoomMenu, z50_triggered);
  ZOOM_MENU(100, zoomMenu, z100_triggered);
  ZOOM_MENU(125, zoomMenu, z125_triggered);
  ZOOM_MENU(150, zoomMenu, z150_triggered);
  ZOOM_MENU(200, zoomMenu, z200_triggered);
  ZOOM_MENU(400, zoomMenu, z400_triggered);
  ZOOM_MENU(600, zoomMenu, z600_triggered);

  zoomButton->setMenu(zoomMenu);

  m_UserInitAreaVector = new QVector<UserInitArea*>;

  m_AxisSettingsDialog = new AxisSettingsDialog(this);
  m_AxisSettingsDialog->setVisible(false);

  m_GraphicsView->setEmMpmGui(this);
  m_GraphicsView->setUserInitAreaTableModel(m_UserInitAreaVector);

  if (m_LayersPalette == NULL)
  {
    m_LayersPalette = new LayersDockWidget(this);
    m_LayersPalette->setGraphicsView(m_GraphicsView);
  //  m_LayersPalette->setFloating(false);
    m_LayersPalette->setVisible(false);
    m_LayersPalette->setFeatures(QDockWidget::AllDockWidgetFeatures);
    m_LayersPalette->setAllowedAreas(Qt::AllDockWidgetAreas);
    m_LayersPalette->getUseColorTable()->setEnabled(enableUserDefinedAreas->isChecked());
  }

#if 0
  compositeModeCB->blockSignals(true);

  compositeModeCB->insertItem(0, "Exclusion", QVariant(EmMpm_Constants::Exclusion));
  compositeModeCB->insertItem(1, "Difference", QVariant(EmMpm_Constants::Difference));
  compositeModeCB->insertItem(2, "Alpha Blend", QVariant(EmMpm_Constants::Alpha_Blend));
#endif


#if 0
  compositeModeCB->insertItem(2, "Plus");
  compositeModeCB->insertItem(3, "Multiply");
  compositeModeCB->insertItem(4, "Screen");
  compositeModeCB->insertItem(5, "Darken");
  compositeModeCB->insertItem(6, "Lighten");
  compositeModeCB->insertItem(7, "Color Dodge");
  compositeModeCB->insertItem(8, "Color Burn");
  compositeModeCB->insertItem(9, "Hard Light");
  compositeModeCB->insertItem(10, "Soft Light");


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

#if 0
  compositeModeCB->setCurrentIndex(2);
  compositeModeCB->blockSignals(false);

  compositeModeCB->setEnabled(false);
#endif

//  connect (m_GraphicsView, SIGNAL(fireBaseImageFileLoaded(const QString &)),
//           this, SLOT(baseImageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (m_GraphicsView, SIGNAL(fireOverlayImageFileLoaded(const QString &)),
           this, SLOT(overlayImageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (m_GraphicsView, SIGNAL(fireUserInitAreaLostFocus()),
           this, SLOT(userInitAreaLostFocus()), Qt::QueuedConnection);

  connect (addUserInitArea, SIGNAL(toggled(bool)),
           m_GraphicsView, SLOT(addUserInitArea(bool)));

  connect (m_GraphicsView, SIGNAL(fireUserInitAreaAdded(UserInitArea*)),
           this, SLOT(userInitAreaAdded(UserInitArea*)), Qt::QueuedConnection);

  connect (zoomIn, SIGNAL(clicked()),
           m_GraphicsView, SLOT(zoomIn()), Qt::QueuedConnection);
  connect(zoomOut, SIGNAL(clicked()),
          m_GraphicsView, SLOT(zoomOut()), Qt::QueuedConnection);
//  connect (zoomCB, SIGNAL(currentIndexChanged(int)),
//           m_GraphicsView, SLOT(setZoomIndex(int)), Qt::QueuedConnection);
//  connect(fitToWindow, SIGNAL(clicked()),
//          m_GraphicsView, SLOT(fitToWindow()), Qt::QueuedConnection);


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


//  m_QueueDialog->setVisible(false);
  cancelBtn->setVisible(false);

  // Hid the user init table by default
 // m_UserInitAreaWidget->hide();

  // Configure the Histogram Plot
  m_HistogramPlot->setCanvasBackground(QColor(Qt::white));
  //m_HistogramPlot->setTitle("Image Histogram");
  //  m_HistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Scale Value");
  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->enableYMin(true);
  m_grid->setMajPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinPen(QPen(Qt::lightGray, 0, Qt::DotLine));
//  m_grid->attach(m_HistogramPlot);

  // setup the Widget List
  m_WidgetList << m_NumClasses << m_EmIterations << m_MpmIterations << m_Beta << m_MinVariance;
  m_WidgetList << enableUserDefinedAreas << useSimulatedAnnealing;
  m_WidgetList << useCurvaturePenalty << useGradientPenalty;
  m_WidgetList << curvatureBetaC << curvatureRMax << ccostLoopDelay;
  m_WidgetList << gradientPenaltyLabel << gradientBetaE;
  m_WidgetList << axisSettingsBtn << clearTempHistograms << saveCurves;
  setWidgetListEnabled(false);

  m_ImageWidgets << zoomIn << zoomOut << fitToWindow << layersPalette;
  setImageWidgetsEnabled(false);

  m_ProcessFolderWidgets <<  sourceDirectoryLE << sourceDirectoryBtn << outputDirectoryLE
  << outputDirectoryBtn << outputPrefix << outputSuffix << filterPatternLabel
  << filterPatternLineEdit << fileListView << outputImageTypeLabel << outputImageType << loadFirstImageBtn;

  QDoubleValidator* betaValidator = new QDoubleValidator(m_Beta);
  QDoubleValidator* minVarValidator = new QDoubleValidator(m_MinVariance);


#if 0
  m_zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft, m_HistogramPlot->canvas());
  m_zoomer->setRubberBand(QwtPicker::RectRubberBand);
  m_zoomer->setRubberBandPen(QColor(Qt::green));
  m_zoomer->setTrackerMode(QwtPicker::ActiveOnly);
  m_zoomer->setTrackerPen(QColor(Qt::blue));

  m_panner = new QwtPlotPanner(m_HistogramPlot->canvas());
  m_panner->setMouseButton(Qt::MidButton);
#endif
  m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_HistogramPlot->canvas());
  m_picker->setRubberBandPen(QColor(Qt::green));
  m_picker->setRubberBand(QwtPicker::CrossRubberBand);
  m_picker->setTrackerPen(QColor(Qt::blue));

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_layersPalette_clicked()
{
  //m_LayersPalette->setFloating(true);
  m_LayersPalette->setVisible(true);
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
  int size = m_UserInitAreaVector->count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    inputs->grayTable[r] = uia->getEmMpmGrayLevel();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyInitCoords( EMMPM_Data* inputs)
{
  int size = m_UserInitAreaVector->count();
  UserInitArea* uia = NULL;
  unsigned int* cPtr = inputs->initCoords[0];
  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    cPtr = inputs->initCoords[r];
    uia->getUpperLeft( cPtr[0], cPtr[1]);
    uia->getLowerRight( cPtr[2], cPtr[3] );
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyIntializationValues(EMMPM_Data* inputs)
{
  int size = m_UserInitAreaVector->count();

  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    inputs->m[r] = uia->getMu();
    inputs->v[r] = uia->getSigma() * uia->getSigma();
 //   std::cout << "Initializing with Mu:" << inputs->m[r] << "  Sigma: " << inputs->v[r] << std::endl;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyGammaValues(EMMPM_Data* inputs)
{
  int size = m_UserInitAreaVector->count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    inputs->w_gamma[r] = uia->getGamma();
 //   std::cout << "Initializing with Gamma:" << inputs->w_gamma[r] << std::endl;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::copyMinVarianceValues(EMMPM_Data* inputs)
{
  int size = m_UserInitAreaVector->count();
  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    inputs->min_variance[r] = uia->getMinVariance();
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
      double constant = 1.0 / (sig * sqrt(2.0 * M_PI));
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

  /* this is a first good sanity check */
  if (enableUserDefinedAreas->isChecked() && m_UserInitAreaVector->count() == 0)
   {
     QMessageBox::critical(this, tr("User Initialization Areas Error"), tr("Enable User Init Areas is ON but no areas are defined."), QMessageBox::Ok);
     return;
   }

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
      QMessageBox::critical(this, tr("Fixed Image File Error"), tr("Input Image does not exist. Please check the path."), QMessageBox::Ok);
      return;
    }

    if (outputImageFile->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Image File Error"), tr("Please select a file name for the segmented image to be saved as."), QMessageBox::Ok);
      return;
    }
    QFile file(outputImageFile->text());
    if (file.exists() == true)
    {
      int ret = QMessageBox::warning(this, tr("EM/MPM GUI"),
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

    fi = QFileInfo(outputImageFile->text());
    QDir outputDir(fi.absolutePath());
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


  m_QueueDialog->clearTable();
  if (getQueueController() != NULL)
  {
    getQueueController()->deleteLater();
  }
  ProcessQueueController* queueController = new ProcessQueueController(this);
  setQueueController(queueController);
 // bool ok;

  InputOutputFilePairList filepairs;

 // m_LayersPalette->getSegmentedImageCheckBox()->blockSignals(true);
  m_LayersPalette->getSegmentedImageCheckBox()->setEnabled(true);
  m_LayersPalette->getSegmentedImageCheckBox()->setChecked(true);
 // m_LayersPalette->getSegmentedImageCheckBox()->blockSignals(false);
  m_LayersPalette->getCompositeTypeComboBox()->setEnabled(true);
  m_LayersPalette->getUseColorTable()->setEnabled(enableUserDefinedAreas->isChecked());
  //m_GraphicsView->setImageDisplayType(EmMpm_Constants::CompositedImage);

  if (this->processFolder->isChecked() == false)
  {
    QString inputFile = inputImageFilePath->text();
    QString outputFile = outputImageFile->text();
    EMMPMTask* task = newEmMpmTask(inputFile, outputFile, queueController);

    queueController->addTask(static_cast<QThread*> (task));
    connect(cancelBtn, SIGNAL(clicked()), task, SLOT(cancel()));

    connect(task, SIGNAL(progressTextChanged(QString )), this, SLOT(processingMessage(QString )), Qt::QueuedConnection);
    connect(task, SIGNAL(updateImageAvailable(QImage)), m_GraphicsView, SLOT(setOverlayImage(QImage)));
    connect(task, SIGNAL(histogramsAboutToBeUpdated()), this, SLOT(clearProcessHistograms()));
    qRegisterMetaType<QVector<real_t> >("QVector<real_t>");
    connect(task, SIGNAL(updateHistogramAvailable(QVector<real_t>)), this, SLOT(addProcessHistogram(QVector<real_t>)));
    this->addProcess(task);
  }
  else
  {
    QStringList fileList = generateInputFileList();
    int32_t count = fileList.count();
    for (int32_t i = 0; i < count; ++i)
    {
      QString inputFile = (sourceDirectoryLE->text() + QDir::separator() + fileList.at(i));

      QFileInfo fileInfo(fileList.at(i));
      QString basename = fileInfo.completeBaseName();
      QString extension = fileInfo.suffix();
      QString outputFile = outputDirectoryLE->text();
      outputFile.append(QDir::separator());
      outputFile.append(outputPrefix->text());
      outputFile.append(basename);
      outputFile.append(outputSuffix->text());
      outputFile.append(".");
      outputFile.append(outputImageType->currentText());

      EMMPMTask* task = newEmMpmTask(inputFile, outputFile, queueController);
      queueController->addTask(static_cast<QThread*> (task));
      connect(cancelBtn, SIGNAL(clicked()), task, SLOT(cancel()));

      filepairs.append(InputOutputFilePair(task->getInputFilePath(), task->getOutputFilePath()));
      queueController->addTask(static_cast<QThread* > (task));
      if (i == 0)
      {
        connect(task, SIGNAL(updateImageAvailable(QImage)), m_GraphicsView, SLOT(setOverlayImage(QImage)));
        connect(task, SIGNAL(histogramsAboutToBeUpdated()), this, SLOT(clearProcessHistograms()));
        connect(task, SIGNAL(updateHistogramAvailable(QVector<real_t>)), this, SLOT(addProcessHistogram(QVector<real_t>)));

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
//  m_QueueDialog->setVisible(true);
  processBtn->setVisible(false);
  cancelBtn->setVisible(true);

  setWidgetListEnabled(false);
  setImageWidgetsEnabled(true);

  queueController->start();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask* EmMpmGui::newEmMpmTask( QString inputFile, QString outputFile, ProcessQueueController* queueController)
{
  bool ok = false;
  EMMPMTask* task = new EMMPMTask(NULL);
  EMMPM_Data* data = task->getEMMPM_Data();

  data->input_file_name = copyStringToNewBuffer(inputFile);
  data->output_file_name = copyStringToNewBuffer(outputFile);
  task->setInputFilePath(inputFile);
  task->setOutputFilePath(outputFile);

  data->in_beta = m_Beta->text().toFloat(&ok);

  for (int i = 0; i < EMMPM_MAX_CLASSES; i++)
  {
    data->w_gamma[i] = 0.0;
  }
  data->mpmIterations = m_MpmIterations->value();
  data->emIterations = m_EmIterations->value();
  data->classes = m_NumClasses->value();
  data->simulatedAnnealing = (useSimulatedAnnealing->isChecked()) ? 1 : 0;
  data->dims = 1; // FORCING A GRAY SCALE IMAGE TO BE USED
  if (enableUserDefinedAreas->isChecked() == false)
  {
    data->initType = EMMPM_Basic;
    int n = data->classes - 1;
    for (int value = 0; value < data->classes; ++value)
    {
      data->grayTable[value] = value * 255 / n;
      data->min_variance[value] = m_MinVariance->text().toFloat(&ok);
    }
  }
  else
  {
    data->initType = EMMPM_ManualInit;
    // Allocate memory to hold the values - The EMMPM Task will free the memory
    data->m = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));
    data->v = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));
    copyGrayValues(data);
    copyInitCoords(data);
    copyIntializationValues(data);
    copyGammaValues(data);
    copyMinVarianceValues(data);
  }
  data->useCurvaturePenalty = (useCurvaturePenalty->isChecked()) ? 1 : 0;
  data->useGradientPenalty = (useGradientPenalty->isChecked()) ? 1 : 0;
  data->beta_e = (useGradientPenalty->isChecked()) ? gradientBetaE->value() : 0.0;
  data->beta_c = (useCurvaturePenalty->isChecked()) ? curvatureBetaC->value() : 0.0;
  data->r_max = (useCurvaturePenalty->isChecked()) ? curvatureRMax->value() : 0.0;
  data->ccostLoopDelay = (useCurvaturePenalty->isChecked()) ? ccostLoopDelay->value() : m_MpmIterations->value() + 1;

  return task;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_cancelBtn_clicked()
{
  std::cout << "on_cancelBtn_clicked" << std::endl;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::addProcess(EMMPMTask* task)
{
  m_QueueDialog->addProcess(task);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::processingStarted()
{
//  std::cout << "EmMpmGui::processingStarted()" << std::endl;
  processBtn->setText("Cancel");
  processBtn->setVisible(false);
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
  processBtn->setVisible(true);
  cancelBtn->setVisible(false);
//  this->statusBar()->showMessage("Processing Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::processingMessage(QString str)
{
  this->statusBar()->showMessage(str);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::queueControllerFinished()
{

#if 0
  if (m_LayersPalette != NULL)
  {
    m_LayersPalette->getSegmentedImageCheckBox()->setEnabled(true);
    m_LayersPalette->getOriginalImageCheckBox()->setChecked(true);
    m_LayersPalette->getSegmentedImageCheckBox()->setChecked(false);
    m_LayersPalette->getCompositeTypeComboBox()->setCurrentIndex(EmMpm_Constants::Alpha_Blend);
    m_LayersPalette->getOpacitySlider()->setEnabled(true);
    m_LayersPalette->getOpacitySpinBox()->setEnabled(true);
    m_LayersPalette->getCompositeTypeComboBox()->setEnabled(true);
  }
#endif

  if (this->processFolder->isChecked() == false)
  {
    setCurrentImageFile (inputImageFilePath->text());
    setCurrentProcessedFile(outputImageFile->text());
    m_GraphicsView->loadOverlayImageFile(outputImageFile->text());
    m_LayersPalette->getSegmentedImageCheckBox()->setChecked(true);
  }
  else
  {
    QStringList fileList = generateInputFileList();

    setCurrentImageFile (sourceDirectoryLE->text() + QDir::separator() + fileList.at(0) );
    m_GraphicsView->blockSignals(true);
    m_GraphicsView->loadBaseImageFile(m_CurrentImageFile);
    m_GraphicsView->blockSignals(false);

    //std::cout << "Setting current Image file: " << getCurrentImageFile().toStdString() << std::endl;
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
    m_GraphicsView->loadOverlayImageFile(m_CurrentProcessedFile);
    m_LayersPalette->getSegmentedImageCheckBox()->setChecked(true);
  //  std::cout << "Setting processed Image file: " << filepath.toStdString() << std::endl;
  }
  setWindowTitle(m_CurrentImageFile);
  setWidgetListEnabled(true);
  if (m_UserInitAreaVector->size() != 0)
  {
    m_NumClasses->setEnabled(false);
    m_MinVariance->setEnabled(false);
  }

  getQueueController()->deleteLater();
  setQueueController(NULL);

  /* Curvature Penalty Widgets */
  curvatureBetaC->setEnabled(useCurvaturePenalty->isChecked());
  curvatureRMax->setEnabled(useCurvaturePenalty->isChecked());
  ccostLoopDelay->setEnabled(useCurvaturePenalty->isChecked());

  /* Gradient Penalty widgets  */
  gradientBetaE->setEnabled(useGradientPenalty->isChecked());

  // Make sure the image manipulating widgets are enabled
  setImageWidgetsEnabled(true);
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

  QFileInfo fileinfo(inputImageFilePath->text());
  if (true == fileinfo.exists())
  {
    setWidgetListEnabled(true);
    setImageWidgetsEnabled(true);
  }
  else
  {
    setWidgetListEnabled(false);
    setImageWidgetsEnabled(false);
  }
  setProcessFolderWidgetsEnabled(enabled);
  inputImageFilePath->setEnabled(!enabled);
  inputImageFilePathBtn->setEnabled(!enabled);
  outputImageFile->setEnabled(!enabled);
  outputImageButton->setEnabled(!enabled);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_loadFirstImageBtn_clicked()
{
  // If the input folder exists
  QFileInfo fileinfo(outputDirectoryLE->text());
  if (true == fileinfo.exists())
  {
    // Get the first image from the list of images
    QStringList fileList = generateInputFileList();
    QString inputFile = (sourceDirectoryLE->text() + QDir::separator() + fileList.at(0));
    openBaseImageFile(inputFile);
  }
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
  loadFirstImageBtn->setEnabled(false);
  if (!getOpenDialogLastDirectory().isNull())
  {
    this->sourceDirectoryLE->setText(getOpenDialogLastDirectory() );
    populateFileTable(sourceDirectoryLE, fileListView);
    loadFirstImageBtn->setEnabled(true);
  }

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
  if (verifyPathExists(inputImageFilePath->text(), inputImageFilePath))
  {
    openBaseImageFile(text);
  }
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
  loadFirstImageBtn->setEnabled(false);
  if (true == verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE) )
  {
    this->populateFileTable(sourceDirectoryLE, fileListView);
    loadFirstImageBtn->setEnabled(true);
  }
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
void EmMpmGui::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList)
  {
    w->setEnabled(b);
  }

  if (b == true) {
    /* Curvature Penalty Widgets */
    curvatureBetaC->setEnabled(useCurvaturePenalty->isChecked());
    curvatureRMax->setEnabled(useCurvaturePenalty->isChecked());
    ccostLoopDelay->setEnabled(useCurvaturePenalty->isChecked());

    /* Gradient Penalty widgets  */
    gradientBetaE->setEnabled(useGradientPenalty->isChecked());
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::setImageWidgetsEnabled(bool b)
{
  foreach (QWidget* w, m_ImageWidgets)
  {
    w->setEnabled(b);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::setProcessFolderWidgetsEnabled(bool b)
{
  foreach (QWidget* w, m_ProcessFolderWidgets)
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
  QFileInfo fi(imageFile);
  m_OpenDialogLastDirectory = fi.absolutePath();
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
  QFileInfo fi(imageFile);
  m_OpenDialogLastDirectory = fi.absolutePath();
  openOverlayImage(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionSaveCanvas_triggered()
{
  QImage image = m_GraphicsView->getOverlayImage();
  if (m_LayersPalette->getOriginalImageCheckBox()->isChecked()
       && m_LayersPalette->getSegmentedImageCheckBox()->isChecked())
  {
    image = m_GraphicsView->getCompositedImage();
  }
  int err = 0;

  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator() + "Segmented.tif";
  outputFile = QFileDialog::getSaveFileName(this, tr("Save Processed Image As ..."), outputFile, tr("Images (*.tif *.bmp *.jpg *.png)"));
  if (!outputFile.isEmpty())
  {
    bool ok = image.save(outputFile);
    if (ok == true)
    {
      //TODO: Set a window title or something
    }
    else
    {
      //TODO: Add in a GUI dialog to help explain the error or give suggestions.
      err = -1;
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionAbout_triggered()
{
  ApplicationAboutBoxDialog about(QEMMPM::LicenseList, this);
  QString an = QCoreApplication::applicationName();
  QString version("");
  version.append(EmMpm_Gui::Version::PackageComplete.c_str());
  about.setApplicationInfo(an, version);
  about.exec();
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
void EmMpmGui::openBaseImageFile(QString imageFile)
{
  if ( true == imageFile.isEmpty() ) // User cancelled the operation
  {
    return;
  }

  inputImageFilePath->blockSignals(true);
  inputImageFilePath->setText(imageFile);
  inputImageFilePath->blockSignals(false);

  setWindowTitle(imageFile);
  this->setWindowFilePath(imageFile);

  m_LayersPalette->getOriginalImageCheckBox()->setChecked(true);
  m_LayersPalette->getSegmentedImageCheckBox()->setChecked(false);

  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
  setImageWidgetsEnabled(true);
  updateBaseRecentFileList(imageFile);

  UserInitArea::deleteAllUserInitAreas(m_GraphicsView->scene());

  // Delete all the User Init Areas from the Scene
  enableUserDefinedAreas->setCheckState(Qt::Unchecked);
  on_enableUserDefinedAreas_stateChanged(Qt::Unchecked);
  //Clear out the UserInitAreas that we are tracking. They have all been released
  // in memory we just simply need to clear the tracking vector. This class does
  // not own the pointer so we don't worry about cleaning up the memory.
  m_UserInitAreaVector->clear();

  ImageOpenDialog d(this);

  d.show();
  d.raise();
  d.activateWindow();
  d.setModal(false);

  d.setStatus("Loading Image File....");
  m_GraphicsView->loadBaseImageFile(imageFile);
  clearProcessHistograms();
  d.setStatus("Generating Histogram....");
  plotImageHistogram();

  QSize size = m_GraphicsView->getBaseImage().size();
  estimateMemoryUse(size);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::imageLoadingComplete()
{
  clearProcessHistograms();
  plotImageHistogram(); // <== This can take a while if the image is large

  QSize size = m_GraphicsView->getBaseImage().size();
  estimateMemoryUse(size);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::estimateMemoryUse(QSize size)
{
  int rows = size.height();
  int cols = size.width();
  int dims = 1;
  int classes = m_NumClasses->value();
  qint64 total = 0;
  total += sizeof(char) * dims * rows * cols; // Input Image
  total += sizeof(char) * dims * rows * cols; // Output image
  total += sizeof(char) * dims * rows * cols; // y
  total += sizeof(char) * rows * cols; // xt
  total += sizeof(real_t) * dims * classes; // m
  total += sizeof(real_t) * dims * classes; // v
  total += sizeof(real_t) * rows * cols * classes; // probs
  total += sizeof(real_t) * dims * classes; // histograms
  total += sizeof(real_t) * rows * cols * classes; // yk (internal to mpm)


  if (useCurvaturePenalty->isChecked() == true)
  {
    total += sizeof(real_t) * rows * cols * classes; // curvature cost
  }
  if (useGradientPenalty->isChecked() == true)
  {
    total += sizeof(real_t) * rows * cols; // ns
    total += sizeof(real_t) * rows * cols; // ew
    total += sizeof(real_t) * rows * cols; // sw
    total += sizeof(real_t) * rows * cols; // nw
  }
  // Major GUI memory allocations to display image
  total += sizeof(int) * rows * cols * 6; // Displayed Image
//  total += sizeof(int) * rows * cols; // Displayed Segmented Image
//  total += sizeof(int) * rows * cols; // Displayed Composited Image
//  total += sizeof(int) * rows * cols; // Displayed QPixMap

  qulonglong totalPhysical = MXAMemory::totalPhysical();
  if (totalPhysical > 1E9)
  {
    totalPhysical = totalPhysical / 1E9;
    QString est = QString::number(totalPhysical);
    est.append(" GB");
    memoryInstalled->setText(est);
  }
  else if (totalPhysical > 1E6 && totalPhysical < 1E9)
  {
    totalPhysical = totalPhysical / 1E6;
    QString est = QString::number(totalPhysical);
    est.append(" MB");
    memoryInstalled->setText(est);
  }
  else
  {
    QString est = QString::number(totalPhysical);
    est.append(" Bytes");
    memoryInstalled->setText(est);
  }

  if (total > 1E9)
  {
    total = total / 1E9;
    QString est = QString::number(total);
    est.append(" GB");
    estimatedMemory->setText(est);
  }
  else if (total > 1E6 && total < 1E9)
  {
    total = total / 1E6;
    QString est = QString::number(total);
    est.append(" MB");
    estimatedMemory->setText(est);
  }
  else
  {
    QString est = QString::number(total);
    est.append(" Bytes");
    estimatedMemory->setText(est);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_useCurvaturePenalty_clicked()
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_useGradientPenalty_clicked()
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_m_NumClasses_valueChanged(int i)
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
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

  if (m_LayersPalette != NULL)
  {
    m_LayersPalette->getOriginalImageCheckBox()->setChecked(true);
    m_LayersPalette->getSegmentedImageCheckBox()->setChecked(true);
    m_LayersPalette->getCompositeTypeComboBox()->setCurrentIndex(EmMpm_Constants::Alpha_Blend);
    m_LayersPalette->getOpacitySlider()->setEnabled(true);
    m_LayersPalette->getOpacitySpinBox()->setEnabled(true);
    m_LayersPalette->getCompositeTypeComboBox()->setEnabled(true);
  }

  setWidgetListEnabled(true);
  setImageWidgetsEnabled(true);

  updateBaseRecentFileList(processedImage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::overlayImageFileLoaded(const QString &filename)
{
  // std::cout << "EmMpmGui::overlayImageFileLoaded" << std::endl;
  outputImageFile->blockSignals(true);
  outputImageFile->setText(filename);
  outputImageFile->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::clearProcessHistograms()
{
  //Loop over each entry in the table
  QwtPlotCurve* curve = NULL;

  // Delete all the current histograms
  qint32 nRows = m_Gaussians.count();
  for (qint32 r = nRows - 1; r >= 0; --r)
  {
    curve = m_Gaussians[r];
    curve->detach();
    m_Gaussians.removeAt(r);
    delete curve;
  }
  // This is an internal variable to keep track of the class number
  m_CurrentHistogramClass = 0;
  if (NULL != m_CombinedGaussians) {
    m_CombinedGaussians->detach();
    delete m_CombinedGaussians;
    m_CombinedGaussians = NULL;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::addProcessHistogram(QVector<real_t> data)
{
  //std::cout << "EmMpmGui::setProcessHistograms..... " << std::endl;
  QwtPlotCurve* curve = NULL;
  const int numValues = data.size();
  double max = 0;
  // Generate the Histogram Bins (X Axis)
  QwtArray<double> intervals(numValues);
  QwtArray<double> dValues(numValues);

  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
    if (data[i] > max) { max = data[i]; }
    dValues[i] = data[i];
  }

  QPen pen(Qt::red, 1.5, Qt::SolidLine);

  if (m_UserInitAreaVector->size() > 0)
  {
    QColor c = m_UserInitAreaVector->at(m_CurrentHistogramClass)->getColor();
    pen.setColor(c);
  }

  // Create a new Plot Curve object
  curve = new QwtPlotCurve("");
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  curve->setPen(pen);
  curve->attach(m_HistogramPlot);
  m_Gaussians.append(curve);

  curve->setData(intervals, dValues);

  updateHistogramAxis();
  ++m_CurrentHistogramClass;

  // Only update the combined Gaussian when all the individual Gaussian's are in place
  if (m_UserInitAreaVector->size() == m_CurrentHistogramClass)
  {
      plotCombinedGaussian();
  }
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaSelected(UserInitArea* uia)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaLostFocus()
{
  m_UserInitAreaWidget->setUserInitArea(NULL);
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

 // m_UserInitAreaWidget->setUserInitArea(uia);
  double mu = 0.0;
  double sig = 0.0;
  int err = m_GraphicsView->calculateMuSigma(uia, mu, sig);
  uia->setMu(mu);
  uia->setSigma(sig);

  double max = std::numeric_limits<double>::min();

  // Generate the Histogram Bins
  const int numValues = 256;
  QwtArray<double> intervals(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
  }
  QwtArray<double> values(numValues);
  float sqrt2pi = sqrt(2.0 * M_PI);
  float twoSigSqrd = sig * sig * 2.0;
  float constant = 1.0 / (sig * sqrt2pi);
  int max_index = 0;
  for (int x = 0; x < 256; ++x)
  {
    values[x] = constant * exp(-1.0f * ((x - mu) * (x - mu)) / (twoSigSqrd));
    if (values[x] > max)
    {
      max = values[x];
      max_index = x;
    }
  }

  // Get the Y Value from the Image Histogram based on the array index of the max value
  double binSize = m_histogram->y( max_index );
  // Now rescale all the Y Values for this Gaussian so that the peak of the gaussian
  // coincides with the local peak of the Image Histogram
  for (int x = 0; x < 256; ++x)
  {
    values[x] = (values[x]/max) * binSize;
  }

  // Locate our curve object by getting the row from the TableModel that corresponds
  // to the UIA object that was passed in
  int row = m_UserInitAreaVector->indexOf(uia, 0);
  QwtPlotCurve* curve = m_Gaussians[row];
  curve->setData(intervals, values);
  QColor c = uia->getColor();
  c.setAlpha(255);
  curve->setPen(QPen(c, uia->getLineWidth(), Qt::SolidLine));

  m_UserInitAreaWidget->setUserInitArea(uia);

//Update the combine Gaussian Curve
  plotCombinedGaussian();
  // Update the plot
  m_HistogramPlot->replot();
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
  float totalPixels = height * width;
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

      gray = (((rgbPixel >> 16) & 0xff)*11
          + ((rgbPixel >> 8) & 0xff)*16
          + (rgbPixel & 0xff)*5)/32;
      values[gray]++;
    }
  }

  // Normalize the bin counts by the total number of pixels
  max = 0.0;
  for (int i = 0; i < 256; ++i)
  {
    values[i] = values[i] / totalPixels;
    if (values[i] > max) { max = values[i]; }
  }
  if (NULL == m_histogram)
  {
    m_histogram = new QwtPlotCurve("Original Image");
    m_histogram->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_histogram->setPen(QPen(Qt::blue, 2.0, Qt::SolidLine));
    m_histogram->attach(m_HistogramPlot);
  }
  m_histogram->setData(intervals, values);

  m_AxisSettingsDialog->setXAxisMax(256.0);
  m_AxisSettingsDialog->setXAxisMin(0.0);
  m_AxisSettingsDialog->setYAxisMax(max);
  m_AxisSettingsDialog->setYAxisMin(0.0);
  updateHistogramAxis();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::plotCombinedGaussian()
{
  if (m_ShowCombinedGaussians == false)
  {
    if (NULL != m_CombinedGaussians) {
      m_CombinedGaussians->detach();
    }
    return;
  }
  if (m_CombinedGaussians == NULL)
  {
    m_CombinedGaussians = new QwtPlotCurve("Combined Gaussians");
    m_CombinedGaussians->setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
    m_CombinedGaussians->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_CombinedGaussians->attach(m_HistogramPlot);
  }

  const int numValues = 256;
  QwtArray<double> intervals(numValues);
  QwtArray<double> values(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
    values[i] = 0.0;
  }


  QwtPlotCurve* curve = NULL;
  int count = m_Gaussians.size();
  if (count < 2) { return; } // only one Gaussian is the same as the user init Gaussian and is meaningless
  for (int c = 0; c < count; ++c)
  {
    curve = m_Gaussians[c];
    QwtArrayData* data = static_cast<QwtArrayData*>(&(curve->data()));
    for (int i = 0; i < numValues; ++i)
    {
      values[i] += data->y(i);
    }
  }
  m_CombinedGaussians->setData(intervals, values);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::deleteUserInitArea(UserInitArea* uia)
{
  int row = m_UserInitAreaVector->indexOf(uia, 0);
  if (row >= m_Gaussians.count() )
  {
    return;
  }
  QwtPlotCurve* curve = m_Gaussians[row];
  m_Gaussians.removeAll(curve);

  curve->detach();
  delete curve; // Clean up the memory
  m_HistogramPlot->replot();

  uia = m_UserInitAreaVector->at(row);
  m_UserInitAreaVector->remove(row);

  // Now renumber the classes:
  qint32 size = m_UserInitAreaVector->size();
  for(qint32 i = 0; i < size; ++i)
  {
    m_UserInitAreaVector->at(i)->setEmMpmClass(i);
  }
  m_NumClasses->setValue(m_UserInitAreaVector->size());
  if (m_UserInitAreaVector->size() == 0)
  {
    m_NumClasses->setEnabled(true);
    m_MinVariance->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaAdded(UserInitArea* uia)
{
 // std::cout << "EmMpmGui::userInitAreaAdded()" << std::endl;
  m_UserInitAreaWidget->setUserInitArea(uia);

  if (NULL == uia) { return; }

  addUserInitArea->setChecked(false);
  QColor color = uia->getColor();
  color.setAlpha(255);

  QwtPlotCurve* curve = new QwtPlotCurve("User Init Area");
  curve->setPen(QPen(color, uia->getLineWidth()));
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  curve->attach(m_HistogramPlot);

  // Figure out the proper row to insert the curve object to keep it in sync with
  // the table model
  int row = m_UserInitAreaVector->indexOf(uia, 0);
  m_NumClasses->setValue(m_UserInitAreaVector->size());
  if (m_UserInitAreaVector->size() != 0)
  {
    m_NumClasses->setEnabled(false);
    m_MinVariance->setEnabled(false);
  }

  m_Gaussians.insert(row, curve);

  // Now update the curve with the initial data
  userInitAreaUpdated(uia);
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::updateHistogramAxis()
{
  double min, max;
  bool ok = false;
  min = m_AxisSettingsDialog->getYAxisMin();
  max = m_AxisSettingsDialog->getYAxisMax();

  m_HistogramPlot->setAxisScale(QwtPlot::yLeft, min, max);

  min = m_AxisSettingsDialog->getXAxisMin();
  max = m_AxisSettingsDialog->getXAxisMax();
  m_HistogramPlot->setAxisScale(QwtPlot::xBottom, min, max);
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_axisSettingsBtn_clicked()
{

  QwtScaleDiv* sd = m_HistogramPlot->axisScaleDiv(QwtPlot::yLeft);
  m_AxisSettingsDialog->setYAxisMin(sd->lowerBound());
  m_AxisSettingsDialog->setYAxisMax(sd->upperBound());

  sd = m_HistogramPlot->axisScaleDiv(QwtPlot::xBottom);
  m_AxisSettingsDialog->setXAxisMin(sd->lowerBound());
  m_AxisSettingsDialog->setXAxisMax(sd->upperBound());

  int ok = m_AxisSettingsDialog->exec();
  if (ok == 1) {
    m_ShowCombinedGaussians = m_AxisSettingsDialog->getShowCombinedGaussians();
    plotCombinedGaussian();
    updateHistogramAxis();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_saveCurves_clicked()
{
  QString outputFile = getOpenDialogLastDirectory() + QDir::separator() + "Curves.csv";
  outputFile = QFileDialog::getSaveFileName(this, tr("Save Plot Curve File As ..."), outputFile, tr("CSV (*.csv)"));
  if (outputFile.isNull())
  {
    return;
  }

  // Update the last directory the user visited
  QFileInfo fi(outputFile);
  setOpenDialogLastDirectory(fi.absolutePath());

  QwtPlotCurve* curve = NULL;
  int count = m_Gaussians.size();

  int columns = count + 2;

  // Set up a row major array
  double* data = new double[columns * 256];
  // Generate the Histogram Data array
  const int numValues = 256;

  // Write the bin values to the array
  for (int i = 0; i < numValues; ++i)
  {
    data[i * columns] = (double)i;
  }

  // Generate the Histogram frequency values
  QwtArray<double> values(numValues);
  QImage image = m_GraphicsView->getBaseImage();
  qint32 height = image.height();
  qint32 width = image.width();
  float totalPixels = height * width;
  QRgb rgbPixel;
  int gray;
  qint32 index;
 // double max = std::numeric_limits<double>::min();

  for (qint32 y = 0; y < height; y++)
  {
    for (qint32 x = 0; x < width; x++)
    {
      index = (y * width) + x;
      rgbPixel = image.pixel(x, y);
      gray = qGray(rgbPixel);
      values[gray]++;
     // if (values[gray] > max) { max = values[gray]; }
    }
  }

  // Normalize the bin counts by the total number of pixels
  // and write the values into the data array
 // max = 0.0;
  for (int i = 0; i < 256; ++i)
  {
    values[i] = values[i] / totalPixels;
    data[i*columns + 1] = values[i];
  }

  // Now step through all the Gaussian Curves and extrat the data
 // if (count < 2) { return; } // only one Gaussian is the same as the user init Gaussian and is meaningless
  for (int i = 0; i < numValues; ++i)
  {
    for (int c = 0; c < count; ++c)
    {
      curve = m_Gaussians[c];
      QwtArrayData* dd = static_cast<QwtArrayData*> (&(curve->data()));
    //  values[i] += data->y(i);
      data[i*columns + 2 + c] = dd->y(i);
    }
  }

  // We now have a table of data, lets write it to a file
  FILE* f = fopen(outputFile.toAscii().data(), "wb");
  if (NULL == f)
  {
    std::cout << "Error trying to write the Histogram and Gaussian data to a file" << std::endl;
    this->statusBar()->setStatusTip(QString("Error trying to write the Histogram and Gaussian data to a file"));
    return;
  }
  double d;
  char comma[2] = {',', 0};

  // Write the header to the file
  fprintf(f, "Bins,Histogram");
  for (int c = 2; c < columns; ++c)
  {
    fprintf(f, ",Class %d", (c-2));
  }
  fprintf(f, "\n");

  // write the data to the file
  for (int i = 0; i < 256; ++i)
  {
    for (int c = 0; c < columns; ++c)
    {
      if (c < columns - 1) { comma[0] = ',';}
      else { comma[0] = '\n';}
      d = data[i*columns + c];
      fprintf(f, "%f%s", d, comma);
    }
  }

  fclose(f);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_clearTempHistograms_clicked()
{
  clearProcessHistograms();
  plotImageHistogram();
  if (enableUserDefinedAreas->isChecked() )
  {
    int size = m_UserInitAreaVector->count();
    UserInitArea* uia = NULL;
    for (int r = 0; r < size; ++r)
    {
      uia = m_UserInitAreaVector->at(r);
      userInitAreaAdded(uia);
    }
  }
  addUserInitArea->setChecked(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_enableUserDefinedAreas_stateChanged(int state)
{
  m_NumClasses->setEnabled( !enableUserDefinedAreas->isChecked() );
  m_MinVariance->setEnabled( !enableUserDefinedAreas->isChecked() );
  m_LayersPalette->getUseColorTable()->setEnabled(enableUserDefinedAreas->isChecked());
  m_LayersPalette->getUseColorTable()->setChecked(enableUserDefinedAreas->isChecked());

  int size = m_UserInitAreaVector->count();
  UserInitArea* uia = NULL;
  int s = m_Gaussians.size();
  int u = m_UserInitAreaVector->size();

  for (int r = 0; r < size; ++r)
  {
    uia = m_UserInitAreaVector->at(r);
    if(enableUserDefinedAreas->isChecked()) {
      uia->setVisible(true);
      m_GraphicsView->scene()->addItem(uia);
      userInitAreaAdded(uia);
    } else {
      uia->setVisible(false);
      if (uia && uia->scene()) {
        m_GraphicsView->scene()->removeItem(uia);
      }
    }
  }
  s = m_Gaussians.size();
  m_UserInitAreaWidget->setUserInitArea(NULL);
  if(enableUserDefinedAreas->isChecked() == false)
  {
    clearProcessHistograms();
  }
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaWidget* EmMpmGui::getUserInitAreaWidget()
{
  return m_UserInitAreaWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionUser_Initialization_triggered()
{
  UserInitAreaDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionHistogram_triggered()
{
  HistogramDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionParameters_triggered()
{
  ParametersDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionLayers_Palette_triggered()
{
  m_LayersPalette->show();
}
