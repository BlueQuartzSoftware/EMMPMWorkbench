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

#include "EmMpmWorkbench.h"

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
#include <QtGui/QShortcut>

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
#include <qwt_curve_fitter.h>
#include <qwt_symbol.h>

// Our Project wide includes
#include "QtSupport/ApplicationAboutBoxDialog.h"
#include "QtSupport/QRecentFileList.h"
#include "QtSupport/QFileCompleter.h"
#include "QtSupport/ImageGraphicsDelegate.h"
#include "QtSupport/ProcessQueueController.h"


#include "MXA/Common/MXAMemory.h"

//-- EMMPM Lib Includes
#include "EMMPMLib/EMMPMLib.h"
#include "EMMPMLib/Common/MSVCDefines.h"
#include "EMMPMLib/Core/EMMPM_Constants.h"
#include "EMMPMLib/Core/InitializationFunctions.h"
#include "EMMPMLib/Core/EMMPM.h"
#include "EMMPMLib/tiff/TiffUtilities.h"

//-- EMMPM Gui Includes
#include "EmMpmWorkbenchVersion.h"
#include "UserInitArea.h"
#include "EMMPMTask.h"
#include "AxisSettingsDialog.h"
#include "License/EmMpmWorkbenchLicenseFiles.h"
#include "UserInitAreaWidget.h"
#include "LayersDockWidget.h"
#include "ImageOpenDialog.h"
#include "ManualInitTableModel.h"


#include "PerClassTableModel.h"



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

#define WRITE_COMBO_SETTING(prefs, var)\
  prefs.setValue(#var, this->var->currentIndex());

#define READ_COMBO_SETTING(prefs, var, emptyValue)\
  { QString s = prefs.value(#var).toString();\
  if (s.isEmpty() == false) {\
    bool ok = false; int bb = prefs.value(#var).toInt(&ok);\
  var->setCurrentIndex(bb); } else { var->setCurrentIndex(emptyValue); } }


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
EmMpmWorkbench::EmMpmWorkbench(QWidget *parent) :
QMainWindow(parent),
m_CurrentImageFile(""),
m_CurrentProcessedFile(""),
m_OutputExistsCheck(false),
m_QueueController(NULL),
m_OpenDialogLastDirectory(""),
m_InputOutputFilePairList(),
m_zoomer(NULL),
m_picker(NULL),
m_panner(NULL),
m_grid(NULL),
m_histogram(NULL),
m_CombinedGaussianCurve(NULL),
m_ShowCombinedGaussians(false),
m_MSEPlotCurve(NULL),
m_MeanOfImage(0.0),
m_SigmaOfImage(0.0),
m_ImageStatsReady(false)
{
  m_OpenDialogLastDirectory = QDir::homePath();
  m_StartingMuValues.resize(EMMPM_MAX_CLASSES);
  m_StartingMuValues[0] = 96;
  m_StartingMuValues[1] = 208;
  m_StartingMuValues[2] = 144;
  m_StartingMuValues[3] = 48;
  m_StartingMuValues[4] = 176;
  m_StartingMuValues[5] = 80;
  m_StartingMuValues[6] = 192;
  m_StartingMuValues[7] = 112;
  m_StartingMuValues[8] = 224;
  m_StartingMuValues[9] = 160;
  m_StartingMuValues[10] = 32;
  m_StartingMuValues[11] = 240;
  m_StartingMuValues[12] = 128;
  m_StartingMuValues[13] = 16;
  m_StartingMuValues[14] = 256;


  initializeColorTable();

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
EmMpmWorkbench::~EmMpmWorkbench()
{

}


// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void EmMpmWorkbench::closeEvent(QCloseEvent *event)
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
void EmMpmWorkbench::writeIOSettings(QSettings &prefs)
{
  prefs.beginGroup("Input_Output");
  WRITE_BOOL_SETTING(prefs, processFolder, processFolder->isChecked());
  WRITE_STRING_SETTING(prefs, inputImageFilePath);
  WRITE_STRING_SETTING(prefs, outputImageFile);
  WRITE_STRING_SETTING(prefs, sourceDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputPrefix);
  WRITE_STRING_SETTING(prefs, outputSuffix);
  WRITE_CHECKBOX_SETTING(prefs, muSigmaFeedback);
  WRITE_BOOL_SETTING(prefs, saveHistogramCheckBox, saveHistogramCheckBox->isChecked());
  WRITE_STRING_SETTING(prefs, filterPatternLineEdit);
  WRITE_COMBO_SETTING(prefs, outputImageType);
  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::readIOSettings(QSettings &prefs)
{
  prefs.beginGroup("Input_Output");
  inputImageFilePath->blockSignals(true);
  READ_STRING_SETTING(prefs, inputImageFilePath, "");
  inputImageFilePath->blockSignals(false);

  READ_STRING_SETTING(prefs, outputImageFile, "");

  sourceDirectoryLE->blockSignals(true);
  READ_STRING_SETTING(prefs, sourceDirectoryLE, "");
  sourceDirectoryLE->blockSignals(true);

  READ_STRING_SETTING(prefs, outputDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputPrefix, "Segmented_");
  READ_STRING_SETTING(prefs, outputSuffix, "");
  READ_BOOL_SETTING(prefs, saveHistogramCheckBox, false);
  READ_BOOL_SETTING(prefs, muSigmaFeedback, false);
  READ_STRING_SETTING(prefs, filterPatternLineEdit, "");
  READ_COMBO_SETTING(prefs, outputImageType, 0);

  processFolder->blockSignals(true);
  READ_BOOL_SETTING(prefs, processFolder, false);
  processFolder->blockSignals(false);
  prefs.endGroup();

  on_processFolder_stateChanged(processFolder->isChecked());

}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void EmMpmWorkbench::readSettings(QSettings &prefs)
{
  QString val;
  bool ok;
  qint32 i;
  double d;
  int userInitAreaCount;

  prefs.beginGroup("Parameters");

  READ_SETTING(prefs, m_EmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_MpmIterations, ok, i, 5, Int);
  READ_STRING_SETTING(prefs, m_Beta, "0.5");

  READ_BOOL_SETTING(prefs, useSimulatedAnnealing, false);

  READ_BOOL_SETTING(prefs, useGradientPenalty, false);
  READ_SETTING(prefs, gradientBetaE, ok, d, 1.0, Double);
  if(useGradientPenalty->isChecked()) {gradientBetaE->setEnabled(true);betaELabel->setEnabled(true);}

  READ_BOOL_SETTING(prefs, useCurvaturePenalty, false);
  READ_SETTING(prefs, curvatureBetaC, ok, d, 1.0, Double);
  READ_SETTING(prefs, curvatureRMax, ok, d, 15.0, Double);
  READ_SETTING(prefs, ccostLoopDelay, ok, i, 1, Int);
  READ_VALUE(prefs, userInitAreaCount, ok, i, 0, Int);

  READ_BOOL_SETTING(prefs, muSigmaFeedback, false);
  enableUserDefinedAreas->blockSignals(true);
  READ_BOOL_SETTING(prefs, enableUserDefinedAreas, false);
  enableUserDefinedAreas->blockSignals(false);
  READ_BOOL_SETTING(prefs, showUserDefinedAreas, false);

  READ_BOOL_SETTING(prefs, enableManualInit, false);

  READ_SETTING(prefs, m_NumClasses, ok, i, 2, Int);

  READ_BOOL_SETTING(prefs, useStoppingCriteria, false);
  READ_STRING_SETTING(prefs, m_StoppingThreshold, "");

  int classCouplingCount = prefs.value("ClassCouplingCount", 0).toInt(&ok);

  prefs.endGroup();  // End the Parameters Group

  {
    PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
    if (NULL != model)
    {
      QList<PerClassItemData*> datas = model->getItemDatas();
      for (int i = 0; i < datas.count(); ++i)
      {
        PerClassItemData* item = datas.at(i);
        item->readSettings(prefs);
      }
    }
  }

  if (enableManualInit->isChecked() == true)
  {
    ManualInitTableModel* model = qobject_cast<ManualInitTableModel*>(manualInitTableView->model());
    if (NULL != model)
    {
      QList<ManualInitData*> datas = model->getManualInits();
      for (int i = 0; i < datas.count(); ++i)
      {
        ManualInitData* uia = datas.at(i);
        uia->readSettings(prefs);
      }
    }
  }

  for(int i = 0; i < classCouplingCount; ++i)
  {
    QString groupName("ClassCoupling-");
    groupName = groupName.append(QString::number(i));
    prefs.beginGroup(groupName);
    m_ClassA->setText(prefs.value("Class_A").toString());
    m_ClassB->setText(prefs.value("Class_B").toString());
    m_CouplingBeta->setText(prefs.value("Exchange_Energy").toString());
    on_addClassCoupling_clicked();
    prefs.endGroup();
  }

  // We only load the User Init Areas if there is an image loaded
  // and the checkbox was set
  if (m_GraphicsView != NULL
      && m_GraphicsView->getBaseImage().isNull() == false
      && enableUserDefinedAreas->isChecked() )
  {
    addUserInitArea->setEnabled(true);
    showUserDefinedAreas->setEnabled(true);
    showUserDefinedAreas->setChecked(true);
    UserInitArea::deleteAllUserInitAreas(m_GraphicsView->scene());
    m_GraphicsView->getUserInitAreas()->clear();
    for (int i = 0; i < userInitAreaCount; ++i)
    {
      UserInitArea* uia = UserInitArea::NewUserInitArea(prefs, i);
      //Calculate the Mean and Standard Deviation
      double mu = 0.0;
      double sig = 0.0;
      int err = m_GraphicsView->calculateMuSigma(uia, mu, sig);
      if (err < 0)
      {
        delete uia;
        continue;
      }
      uia->setMu(mu);
      uia->setSigma(sig);
      addUserInitArea->toggle();
      m_GraphicsView->addNewInitArea(uia);
    }
  }

    updateGaussianCurves(true);
}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void EmMpmWorkbench::writeSettings(QSettings &prefs)
{

  prefs.setValue("Version", QString::fromStdString(EmMpm_Workbench::Version::Complete()));
//  WRITE_STRING_SETTING(prefs, outputPrefix);
//  WRITE_STRING_SETTING(prefs, outputSuffix);

//  WRITE_CHECKBOX_SETTING(prefs, processFolder);
//  WRITE_STRING_SETTING(prefs, sourceDirectoryLE);
//  WRITE_STRING_SETTING(prefs, outputDirectoryLE);
//  WRITE_CHECKBOX_SETTING(prefs, muSigmaFeedback);

//  WRITE_CHECKBOX_SETTING(prefs, saveHistogramCheckBox);
//  WRITE_STRING_SETTING(prefs, filterPatternLineEdit);
//  WRITE_COMBO_SETTING(prefs, outputImageType);


  prefs.beginGroup("Parameters");
  WRITE_SETTING(prefs, m_NumClasses);
  WRITE_SETTING(prefs, m_EmIterations);
  WRITE_SETTING(prefs, m_MpmIterations);
  WRITE_STRING_SETTING(prefs, m_Beta);

  WRITE_CHECKBOX_SETTING(prefs, useSimulatedAnnealing);
  WRITE_CHECKBOX_SETTING(prefs, useGradientPenalty);
  WRITE_SETTING(prefs, gradientBetaE);
  WRITE_CHECKBOX_SETTING(prefs, useCurvaturePenalty);
  WRITE_SETTING(prefs, curvatureBetaC);
  WRITE_SETTING(prefs, curvatureRMax);
  WRITE_SETTING(prefs, ccostLoopDelay);

  WRITE_CHECKBOX_SETTING(prefs, enableUserDefinedAreas);
  WRITE_CHECKBOX_SETTING(prefs, showUserDefinedAreas);
  WRITE_CHECKBOX_SETTING(prefs, muSigmaFeedback);
  int userInitAreaCount = this->m_GraphicsView->getUserInitAreas()->size();
  WRITE_VALUE(prefs, userInitAreaCount);

  WRITE_CHECKBOX_SETTING(prefs, enableManualInit);

  WRITE_CHECKBOX_SETTING(prefs, useStoppingCriteria);
  WRITE_STRING_SETTING(prefs, m_StoppingThreshold);

  prefs.setValue("ClassCouplingCount", m_ClassCouplingTableWidget->rowCount());


  prefs.endGroup();

  if (enableManualInit->isChecked() == true)
  {
    ManualInitTableModel* model = qobject_cast<ManualInitTableModel*>(manualInitTableView->model());
    if (NULL != model)
    {
      QList<ManualInitData*> datas = model->getManualInits();
      for (int i = 0; i < datas.count(); ++i)
      {
        ManualInitData* uia = datas.at(i);
        uia->writeSettings(prefs);
      }
    }
  }
  {
    PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
    if (NULL != model)
    {
      QList<PerClassItemData*> datas = model->getItemDatas();
      for (int i = 0; i < datas.count(); ++i)
      {
        PerClassItemData* uia = datas.at(i);
        uia->writeSettings(prefs);
      }
    }
  }

  if (enableUserDefinedAreas->isChecked() == true)
  {
    for (int i = 0; i < this->m_GraphicsView->getUserInitAreas()->size(); ++i)
    {
      UserInitArea* uia = m_GraphicsView->getUserInitAreas()->at(i);
      uia->writeSettings(prefs);
    }
  }

  int count = m_ClassCouplingTableWidget->rowCount();
  for(int row = 0; row < count; ++row)
  {
    QString groupName("ClassCoupling-");
    groupName = groupName.append(QString::number(row));
    prefs.beginGroup(groupName);
    prefs.setValue("Class_A", m_ClassCouplingTableWidget->item(row, 0)->text());
    prefs.setValue("Class_B", m_ClassCouplingTableWidget->item(row, 1)->text());
    prefs.setValue("Exchange_Energy", m_ClassCouplingTableWidget->item(row, 2)->text());
    prefs.endGroup();
  }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::readWindowSettings(QSettings &prefs)
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
void EmMpmWorkbench::writeWindowSettings(QSettings &prefs)
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
void EmMpmWorkbench::on_actionSave_Config_File_triggered()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "EMMPM-Config.txt";
  QString file = QFileDialog::getSaveFileName(this, tr("Save EM/MPM Configuration"),
                                              proposedFile,
                                              tr("*.txt") );
  if ( true == file.isEmpty() ){ return;  }
  QFileInfo fi(file);
  m_OpenDialogLastDirectory = fi.absolutePath();
  QSettings prefs(file, QSettings::IniFormat, this);
  writeIOSettings(prefs);
  writeSettings(prefs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionLoad_Config_File_triggered()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select Configuration File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Configuration File (*.txt)") );
  if ( true == file.isEmpty() ){return;  }
  QFileInfo fi(file);
  m_OpenDialogLastDirectory = fi.absolutePath();
  QSettings prefs(file, QSettings::IniFormat, this);
  readIOSettings(prefs);
  readSettings(prefs);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_action_ResetPreferences_triggered()
{
  #if defined (Q_OS_MAC)
  QSettings prefs(QSettings::NativeFormat, QSettings::UserScope, "bluequartz.net", "EmMpmWorkbench");
#else
  QSettings prefs(QSettings::IniFormat, QSettings::UserScope, "bluequartz.net", "EmMpmWorkbench");
#endif
  prefs.clear();
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
void EmMpmWorkbench::z##var##_triggered() {\
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
void EmMpmWorkbench::on_fitToWindow_clicked()
{
  m_GraphicsView->setZoomIndex(9);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::setupGui()
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

  m_AxisSettingsDialog = new AxisSettingsDialog(this);
  m_AxisSettingsDialog->setVisible(false);

  m_GraphicsView->setEmMpmWorkbench(this);

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

  // Configure the Histogram Plot
  m_HistogramPlot->setCanvasBackground(QColor(Qt::white));
  m_grid = new QwtPlotGrid;
  m_grid->enableXMin(true);
  m_grid->enableYMin(true);
  m_grid->setMajPen(QPen(Qt::gray, 0, Qt::SolidLine));
  m_grid->setMinPen(QPen(Qt::lightGray, 0, Qt::DotLine));
//  m_grid->attach(m_HistogramPlot);

  // setup the Widget List
  m_WidgetList << m_NumClasses << m_EmIterations << m_MpmIterations << m_Beta;
  m_WidgetList << enableUserDefinedAreas << showUserDefinedAreas;
  m_WidgetList << useCurvaturePenalty << useGradientPenalty << useSimulatedAnnealing;
  m_WidgetList << curvatureBetaC << curvatureRMax << ccostLoopDelay;
  m_WidgetList << gradientBetaE;
  m_WidgetList << axisSettingsBtn << clearTempHistograms << saveCurves;
  m_WidgetList << m_StoppingThreshold << m_StoppingThresholdLabel << useStoppingCriteria;
  setWidgetListEnabled(false);

  m_ImageWidgets << zoomIn << zoomOut << fitToWindow;
  setImageWidgetsEnabled(false);

  m_ProcessFolderWidgets <<  sourceDirectoryLE << sourceDirectoryBtn << outputDirectoryLE
  << outputDirectoryBtn << outputPrefix << outputSuffix << filterPatternLabel
  << filterPatternLineEdit << fileListWidget << outputImageTypeLabel << outputImageType
  << muSigmaFeedback << saveHistogramCheckBox << outputSuffixLabel << outputPrefixLabel
  << exampleFileNameLabel << outputFilenamePattern << histogramFilenamePattern;


 // QDoubleValidator* betaValidator = new QDoubleValidator(m_Beta);
  QDoubleValidator* stoppingThresholdValidator = new QDoubleValidator(m_StoppingThreshold);
  stoppingThresholdValidator->setRange(0.0, 1000000, 5);
  m_StoppingThreshold->setValidator(stoppingThresholdValidator);

  m_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_HistogramPlot->canvas());
  m_picker->setRubberBandPen(QColor(QColor(0, 125, 0)));
  m_picker->setRubberBand(QwtPicker::CrossRubberBand);
  m_picker->setTrackerPen(QColor(Qt::blue));


  // Add a Plot Picker for the MSE Plots
  m_MSEpicker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPicker::PointSelection, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, m_MSEPlot->canvas());
  m_MSEpicker->setRubberBandPen(QColor(QColor(0, 125, 0)));
  m_MSEpicker->setRubberBand(QwtPicker::CrossRubberBand);
  m_MSEpicker->setTrackerPen(QColor(Qt::blue));

  /* Setup the Manual Initialization Table View with a Model and hook up some signals/slots */
  ManualInitTableModel* tm = new ManualInitTableModel(manualInitTableView);
  manualInitTableView->setModel(tm);
  QAbstractItemDelegate* aid = tm->getItemDelegate();
  manualInitTableView->setItemDelegate(aid);
  connect(tm, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(manualInitDataChanged(const QModelIndex &, const QModelIndex &)));
  for(int c = 0; c < tm->columnCount(); ++c)
  {
   //   manualInitTableView->horizontalHeader()->setResizeMode(c, QHeaderView::ResizeToContents);
      manualInitTableView->horizontalHeader()->resizeSection(c, 100);
  }


  /* Setup the PerClass Parameters Table */
  PerClassTableModel* pctm = new PerClassTableModel(perClassTableView);
  perClassTableView->setModel(pctm);
  QAbstractItemDelegate* pcid = pctm->getItemDelegate();
  perClassTableView->setItemDelegate(pcid);
//  connect(pcid, SIGNAL(closeEditor(QWidget*)),
//          this, SLOT(perClassItemEditingComplete(QWidget*)));
  connect(pctm, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(perClassItemDataChanged(const QModelIndex &, const QModelIndex &)));
  for(int c = 0; c < pctm->columnCount(); ++c)
  {
      perClassTableView->horizontalHeader()->setResizeMode(c, QHeaderView::ResizeToContents);
  }
  perClassTableView->horizontalHeader()->setResizeMode(PerClassTableModel::Color, QHeaderView::Fixed);
  perClassTableView->horizontalHeader()->resizeSection(PerClassTableModel::Color, 60);


  // option B (pressing DEL activates the slots only when list widget has focus)
  QShortcut* shortcut_delete = new QShortcut(QKeySequence(Qt::Key_Delete), fileListWidget);
  connect(shortcut_delete, SIGNAL(activated()), this, SLOT(deleteFileListItem()));
  QShortcut* shortcut_backspace = new QShortcut(QKeySequence(Qt::Key_Backspace), fileListWidget);
  connect(shortcut_backspace, SIGNAL(activated()), this, SLOT(deleteFileListItem()));

  // Hid the progress bar
  progBar->hide();

  compositeModeCB->blockSignals(true);
  compositeModeCB->insertItem(0, "Exclusion", QVariant(EmMpm_Constants::Exclusion));
  compositeModeCB->insertItem(1, "Difference", QVariant(EmMpm_Constants::Difference));
  compositeModeCB->insertItem(2, "Alpha Blend", QVariant(EmMpm_Constants::Alpha_Blend));
  compositeModeCB->insertItem(3, "Plus", QVariant(EmMpm_Constants::Plus));
  compositeModeCB->insertItem(4, "Multiply", QVariant(EmMpm_Constants::Multiply));
  compositeModeCB->insertItem(5, "Screen", QVariant(EmMpm_Constants::Screen));
  compositeModeCB->insertItem(6, "Darken", QVariant(EmMpm_Constants::Darken));
  compositeModeCB->insertItem(7, "Lighten", QVariant(EmMpm_Constants::Lighten));
  compositeModeCB->insertItem(8, "ColorDodge", QVariant(EmMpm_Constants::ColorDodge));
  compositeModeCB->insertItem(9, "ColorBurn", QVariant(EmMpm_Constants::ColorBurn));
  compositeModeCB->insertItem(10, "HardLight", QVariant(EmMpm_Constants::HardLight));
  compositeModeCB->insertItem(11, "SoftLight", QVariant(EmMpm_Constants::SoftLight));
  compositeModeCB->insertItem(12, "Destination", QVariant(EmMpm_Constants::Destination));
  compositeModeCB->insertItem(13, "Source", QVariant(EmMpm_Constants::Source));
  compositeModeCB->insertItem(14, "DestinationOver", QVariant(EmMpm_Constants::DestinationOver));
  compositeModeCB->insertItem(15, "SourceIn", QVariant(EmMpm_Constants::SourceIn));
  compositeModeCB->insertItem(16, "DestinationIn", QVariant(EmMpm_Constants::DestinationIn));
  compositeModeCB->insertItem(17, "DestinationOut", QVariant(EmMpm_Constants::DestinationOut));
  compositeModeCB->insertItem(18, "SourceAtop", QVariant(EmMpm_Constants::SourceAtop));
  compositeModeCB->insertItem(19, "DestinationAtop", QVariant(EmMpm_Constants::DestinationAtop));
  compositeModeCB->insertItem(20, "Overlay", QVariant(EmMpm_Constants::Overlay));
  compositeModeCB->insertItem(21, "Clear", QVariant(EmMpm_Constants::Clear));

  compositeModeCB->setCurrentIndex(2); // Default to an Alpha Blend;

  compositeModeCB->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::deleteFileListItem()
{
  QList<QListWidgetItem*>selectedItems = fileListWidget->selectedItems();
  qint32 count = selectedItems.count();
  for(qint32 i = 0; i < count; ++i)
  {
    QListWidgetItem* item = selectedItems[i];
    delete item;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::copyGrayValues( EMMPM_Data::Pointer inputs)
{
    int rows = perClassTableView->model()->rowCount();
    for(int i = 0; i < rows; ++i)
    {
        //Take the current color and convert it to a color value and use that
        // value in the gray table
        QModelIndex colorIndex = perClassTableView->model()->index(i, PerClassTableModel::Color);
        QColor color(perClassTableView->model()->data(colorIndex).toString());
        color.setAlpha(255);

//        float R = color.red() * 0.299;
//        float G = color.green() * 0.587;
//        float B = color.blue() * 0.144;

        inputs->colorTable[i] = static_cast<unsigned int>(color.rgba());
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::copyInitCoords( EMMPM_Data::Pointer inputs)
{
  int size = m_GraphicsView->getUserInitAreas()->count();
  UserInitArea* uia = NULL;
  unsigned int* cPtr = inputs->initCoords[0];
  for (int r = 0; r < size; ++r)
  {
    uia = m_GraphicsView->getUserInitAreas()->at(r);
    if (NULL == uia) { continue; }
    cPtr = inputs->initCoords[r];
    uia->getUpperLeft( cPtr[0], cPtr[1]);
    uia->getLowerRight( cPtr[2], cPtr[3] );
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::copyIntializationValues(EMMPM_Data::Pointer inputs)
{
  int size = m_GraphicsView->getUserInitAreas()->count();

  UserInitArea* uia = NULL;
  for (int r = 0; r < size; ++r)
  {
    uia = m_GraphicsView->getUserInitAreas()->at(r);
    if (NULL == uia) { continue; }
    inputs->mean[r] = uia->getMu();
    inputs->variance[r] = uia->getSigma() * uia->getSigma();
 //   std::cout << "Initializing with Mu:" << inputs->m[r] << "  Sigma: " << inputs->v[r] << std::endl;
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int EmMpmWorkbench::copyGammaMinStdDevValues(EMMPM_Data::Pointer inputs)
{
    PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
    QList<PerClassItemData*> items = model->getItemDatas();
    int size = items.count();
    for (int r = 0; r < size; ++r)
    {
        inputs->w_gamma[r] = items.at(r)->getGamma();
        inputs->min_variance[r] = items.at(r)->getMinStdDev() * items.at(r)->getMinStdDev();
    }
    return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMUpdateStats(EMMPM_Data::Pointer data)
{
  // Check to make sure we are at the end of an em loop
  if ( data->inside_mpm_loop == 0 && NULL != data->outputImage)
  {
    char buff[256];
    memset(buff, 0, 256);
    snprintf(buff, 256, "/tmp/emmpm_out_%d.tif", data->currentEMLoop);
    TiffUtilities tifUtil;
    int err = tifUtil.writeGrayScaleImage(buff, data->rows, data->columns, "Intermediate Image", data->outputImage);
    if (err < 0)
    {
      std::cout << "Error writing intermediate tiff image." << std::endl;
    }

    std::cout << "Class\tMu\tSigma" << std::endl;
    for (int l = 0; l < data->classes; l++)
    {
      //    snprintf(msgbuff, 256, "%d\t%.3f\t%.3f", l, data->m[l], data->v[l]);
      //    EMMPM_ShowProgress(msgbuff, data->progress);
      std::cout << l << "\t" << data->mean[l] << "\t" << data->variance[l] << "\t" << std::endl;
    }

    double hist[EMMPM_MAX_CLASSES][256];
    // Generate a gaussian curve for each class based off the mu and sigma for that class
    for (int c = 0; c < data->classes; ++c)
    {
      double mu = data->mean[c];
      double sig = data->variance[c];
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
void EmMpmWorkbench::on_processBtn_clicked()
{
  /* this is a first good sanity check */
  if (enableUserDefinedAreas->isChecked() && m_GraphicsView->getUserInitAreas()->count() == 0)
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

    if (this->fileListWidget->model()->rowCount() == 0)
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
    if (fi.suffix().compare("jpg") == 0 || fi.suffix().compare("jpeg") == 0)
    {
        QMessageBox::critical(this, tr("Output Image Format Error"), tr("Due to compression artifacts using the JPEG format please use tif, bmp or png as the output format."), QMessageBox::Ok);
        return;
    }


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


  if (getQueueController() != NULL)
  {
    getQueueController()->deleteLater();
  }
  ProcessQueueController* queueController = new ProcessQueueController(this);
  setQueueController(queueController);

  InputOutputFilePairList filepairs;

  EMMPMTask* task = newEmMpmTask(queueController);
  if (NULL == task)
  {
      return;
  }
  queueController->addTask(static_cast<QThread*> (task));

  if (this->processFolder->isChecked() == false)
  {
    QString inputFile = inputImageFilePath->text();
    QString outputFile = outputImageFile->text();
    filepairs.append(InputOutputFilePair(inputFile, outputFile));

    QFileInfo fi(outputFile);
    QString statsFile = fi.path();
    statsFile.append(QDir::separator());
    statsFile.append(fi.baseName());
    statsFile.append("_Stats.txt");
    task->setOutputStatsFile(statsFile.toStdString());

  }
  else
  {
    QStringList fileList = generateInputFileList();

    QString inputFile = (sourceDirectoryLE->text() + QDir::separator() + fileList.at(0));
    QFileInfo fileInfo(fileList.at(0));
    QString basename = fileInfo.completeBaseName();
    QString extension = fileInfo.suffix();
    QString outputFile = outputDirectoryLE->text();
    outputFile.append(QDir::separator());
    outputFile.append(outputPrefix->text());
    outputFile.append(basename);
    outputFile.append(outputSuffix->text());
    outputFile.append(".");
    outputFile.append(outputImageType->currentText());

    filepairs.append(InputOutputFilePair(inputFile, outputFile));

    int32_t count = fileList.count();
    for (int32_t i = 1; i < count; ++i)
    {
      inputFile = (sourceDirectoryLE->text() + QDir::separator() + fileList.at(i));

      fileInfo = QFileInfo(fileList.at(i));
      basename = fileInfo.completeBaseName();
      extension = fileInfo.suffix();
      outputFile = outputDirectoryLE->text();
      outputFile.append(QDir::separator());
      outputFile.append(outputPrefix->text());
      outputFile.append(basename);
      outputFile.append(outputSuffix->text());
      outputFile.append(".");
      outputFile.append(outputImageType->currentText());

      filepairs.append(InputOutputFilePair(inputFile, outputFile));
    }
    outputFile = outputDirectoryLE->text();
    outputFile.append(QDir::separator());
    outputFile.append("Stats.txt");
    task->setOutputStatsFile(outputFile.toStdString());
  }


  // Clear out the MSE Vector
  m_MSEValues.clear();
  m_msePlotXMax->setMaximum(0);
  m_msePlotXMax->setValue(0);

  m_msePlotXMin->setMaximum(0);
  m_msePlotXMin->setValue(0);


  task->setInputOutputFilePairList(filepairs);
  setInputOutputFilePairList(filepairs);

  connect(cancelBtn, SIGNAL(clicked()), task, SLOT(cancel()));

  connect(task, SIGNAL(progressMessage(QString )), this, SLOT(processingMessage(QString )), Qt::QueuedConnection);
//  connect(task, SIGNAL(updateImageAvailable(QImage)), m_GraphicsView, SLOT(setOverlayImage(QImage)));
  connect(task, SIGNAL(updateImageAvailable(QImage)), this, SLOT(segmentedImageAvailable(QImage)));
  connect(task, SIGNAL(histogramsAboutToBeUpdated()), this, SLOT(clearGaussianCurves()));
  qRegisterMetaType<QVector<real_t> >("QVector<real_t>");
  connect(task, SIGNAL(updateHistogramAvailable(QVector<real_t>)), this, SLOT(addGaussianCurve(QVector<real_t>)));
  if (processFolder->isChecked())
  {
    connect(task, SIGNAL(imageStarted(QString )), this, SLOT(setCurrentProcessedImage(QString)));
  }

  connect(task, SIGNAL(mseValueUpdated(qreal)), this, SLOT(updateMSEValue(qreal)));
  connect(task, SIGNAL(updateProgress(int)), progBar, SLOT(setValue(int)));
  connect(task, SIGNAL(imageStarted(QString)), progBar, SLOT(setText(QString)));

  // When the event loop of the controller starts it will signal the ProcessQueue to run
  connect(queueController, SIGNAL(started()), queueController, SLOT(processTask()));
  // When the QueueController finishes it will signal the QueueController to 'quit', thus stopping the thread
  connect(queueController, SIGNAL(finished()), this, SLOT(queueControllerFinished()));

  connect(queueController, SIGNAL(started()), this, SLOT(processingStarted()));
  connect(queueController, SIGNAL(finished()), this, SLOT(processingFinished()));

  processBtn->setVisible(false);
  cancelBtn->setVisible(true);

  setWidgetListEnabled(false);
  setImageWidgetsEnabled(true);

  queueController->start();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMTask* EmMpmWorkbench::newEmMpmTask( ProcessQueueController* queueController)
{
  bool ok = false;
  EMMPMTask* task = new EMMPMTask(NULL);
  EMMPM_Data::Pointer data = task->getEMMPM_Data();
  for (int i = 0; i < EMMPM_MAX_CLASSES; i++)
  {
    data->w_gamma[i] = 0.0;
  }

  task->setFeedBackInitialization(muSigmaFeedback->isChecked());
  task->setSaveHistogram(saveHistogramCheckBox->isChecked());
  task->setImageHistogram(m_ImageHistogramData);

  data->in_beta = m_Beta->text().toFloat(&ok);

  if (copyGammaMinStdDevValues(data) < 0)
  {
      delete task;
      return NULL;
  }

  data->mpmIterations = m_MpmIterations->value();
  data->emIterations = m_EmIterations->value();
  data->classes = m_NumClasses->value();
  data->simulatedAnnealing = (useSimulatedAnnealing->isChecked()) ? 1 : 0;
  data->dims = 1; // FORCING A GRAY SCALE IMAGE TO BE USED

  if (enableManualInit->isChecked() == true)
  {
    data->initType = EMMPM_ManualInit;
    // Allocate memory to hold the values - The EMMPM Task will free the memory
    data->mean = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));
    data->variance = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));

    ManualInitTableModel* model = qobject_cast<ManualInitTableModel*>(manualInitTableView->model());
    if (NULL == model) { return NULL; }
    bool ok = false;
    int rows = model->rowCount();

    for(int i = 0; i < rows; ++i)
    {
      QModelIndex muIndex = model->index(i, ManualInitTableModel::Mu);
      double mu = model->data(muIndex).toDouble(&ok);
      QModelIndex sigmaIndex = model->index(i, ManualInitTableModel::Sigma);
      double sigma = model->data(sigmaIndex).toDouble(&ok);

      data->mean[i] = mu;
      data->variance[i] = sigma * sigma; // Variance is sigma squared (sig^2)
    }

  }
  else if (enableUserDefinedAreas->isChecked() == true)
  {
    data->initType = EMMPM_ManualInit;
    // Allocate memory to hold the values - The EMMPM Task will free the memory
    data->mean = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));
    data->variance = (real_t*)malloc(data->classes * data->dims * sizeof(real_t));
    copyInitCoords(data);
    // This copies the mean and StdDev
    copyIntializationValues(data);
  }
  else
  {
    data->initType = EMMPM_Basic;
  }

  copyGrayValues(data);

  data->useCurvaturePenalty = (useCurvaturePenalty->isChecked()) ? 1 : 0;
  data->useGradientPenalty = (useGradientPenalty->isChecked()) ? 1 : 0;
  data->beta_e = (useGradientPenalty->isChecked()) ? gradientBetaE->value() : 0.0;
  data->beta_c = (useCurvaturePenalty->isChecked()) ? curvatureBetaC->value() : 0.0;
  data->r_max = (useCurvaturePenalty->isChecked()) ? curvatureRMax->value() : 0.0;
  data->ccostLoopDelay = (useCurvaturePenalty->isChecked()) ? ccostLoopDelay->value() : m_MpmIterations->value() + 1;

  // Make sure the Coupling Beta Pointer is initialized
  if (NULL == data->couplingBeta)
  {
    unsigned int cSize = data->classes + 1;
    size_t couplingElements = cSize * cSize;
    data->couplingBeta = static_cast<real_t*>(malloc(sizeof(real_t) * couplingElements));
    ::memset(data->couplingBeta, 0, sizeof(real_t) * couplingElements);
  }
  // Transfer from the user interface the Class Coupling entries
  int rc = m_ClassCouplingTableWidget->rowCount();
  std::vector<CoupleType> coupleEntries;
  ok = false;
  for(int i = 0; i < rc; ++i)
  {
    CoupleType couple;
    couple.label_1 = m_ClassCouplingTableWidget->item(i, 0)->text().toInt(&ok);
    if (ok == false) { continue; } // If the conversion didn't work then move to the next one
    couple.label_2 = m_ClassCouplingTableWidget->item(i, 1)->text().toInt(&ok);
    if (ok == false) { continue; } // If the conversion didn't work then move to the next one
    couple.beta = m_ClassCouplingTableWidget->item(i, 2)->text().toDouble(&ok);
    if (ok == false) { continue; } // If the conversion didn't work then move to the next one
    coupleEntries.push_back(couple);
  }
  data->coupleEntries = coupleEntries;
  // The beta matrix will be filled each time through the EM Loop so we do not need to calculate it here.


  /* Transfer the Stopping Criteria Values */
  ok = false;
  data->useStoppingThreshold = useStoppingCriteria->isChecked();
  data->stoppingThreshold = m_StoppingThreshold->text().toFloat(&ok); // The validator should have made sure we are a valid floating point value



  return task;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::processingStarted()
{
//  std::cout << "EmMpmWorkbench::processingStarted()" << std::endl;
  processBtn->setText("Cancel");
  processBtn->setVisible(false);
  this->statusBar()->showMessage("Processing Images...");
  progBar->show();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::processingFinished()
{
//  std::cout << "IPHelper::processingFinished()" << std::endl;
  /* Code that cleans up anything from the processing */
  processBtn->setText("Segment");
  processBtn->setVisible(true);
  cancelBtn->setVisible(false);
  progBar->setText("");
  progBar->setValue(0);
  progBar->hide();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::processingMessage(QString str)
{
  this->statusBar()->showMessage(str);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::queueControllerFinished()
{
  if (this->processFolder->isChecked() == false)
  {
    setCurrentImageFile (inputImageFilePath->text());
//    setCurrentProcessedFile(outputImageFile->text());
//    m_GraphicsView->loadOverlayImageFile(outputImageFile->text());
   // imageDisplaySelection->setCurrentIndex(2);
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
    imageDisplaySelection->setCurrentIndex(2);
  }
  setWindowTitle(m_CurrentImageFile);
  setWidgetListEnabled(true);
  if (m_GraphicsView->getUserInitAreas()->size() != 0)
  {
    m_NumClasses->setEnabled(false);
  }

  getQueueController()->deleteLater();
  setQueueController(NULL);

  /* Curvature Penalty Widgets */
  curvatureBetaC->setEnabled(useCurvaturePenalty->isChecked());
  curvatureRMax->setEnabled(useCurvaturePenalty->isChecked());
  ccostLoopDelay->setEnabled(useCurvaturePenalty->isChecked());

  /* Gradient Penalty widgets  */
  gradientBetaE->setEnabled(useGradientPenalty->isChecked());

  /* Stopping Criteria widgets */
  m_StoppingThreshold->setEnabled(useStoppingCriteria->isChecked());
  m_StoppingThresholdLabel->setEnabled(useStoppingCriteria->isChecked());

  /* Update MSE Plot with better title */
  m_MSEPlot->setTitle("MSE Plot");


  // Make sure the image manipulating widgets are enabled
  setImageWidgetsEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_processFolder_stateChanged(int state)
{

    bool checked = true;
    if (state == Qt::Unchecked)
    {
        checked = false;
    }

    // Enable/Disable widgets
    setProcessFolderWidgetsEnabled(checked);
    inputImageFilePath->setEnabled(!checked);
    inputImageFilePathBtn->setEnabled(!checked);
    outputImageFile->setEnabled(!checked);
    outputImageButton->setEnabled(!checked);



    // If we are checked then load the first image (assuming it exists) in the image viewer
    if (checked == true)
    {
        on_sourceDirectoryLE_textChanged(QString(""));
        // Make sure the filter is run on the file list
        on_filterPatternLineEdit_textChanged();
    }
    else // We load up what ever file is in the input image file path line edit
    {
        on_inputImageFilePath_textChanged(inputImageFilePath->text());
        return;
    }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_fileListWidget_itemDoubleClicked(QListWidgetItem * item)
{
  QString path = sourceDirectoryLE->text();
  path.append(QDir::separator());
  path.append(item->text());

  QFileInfo fi(path);
  if (fi.exists() == true)
  {
    openBaseImageFile(QDir::toNativeSeparators(path));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputPrefix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
  histogramFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + ".csv" );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputSuffix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
  histogramFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + ".csv" );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputImageType_currentIndexChanged(int index)
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_sourceDirectoryBtn_clicked()
{
  QString aDir = QFileDialog::getExistingDirectory(this, tr("Select Source Directory"), getOpenDialogLastDirectory(), QFileDialog::ShowDirsOnly
          | QFileDialog::DontResolveSymlinks);
  setOpenDialogLastDirectory(aDir);

  if (!getOpenDialogLastDirectory().isNull())
  {
    this->sourceDirectoryLE->setText(getOpenDialogLastDirectory() );
    verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE);
    populateFileTable(sourceDirectoryLE, fileListWidget);
    if (fileListWidget->count() > 0) {
      QListWidgetItem* item = fileListWidget->item(0);
      on_fileListWidget_itemDoubleClicked(item);
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputDirectoryBtn_clicked()
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
void EmMpmWorkbench::on_inputImageFilePathBtn_clicked()
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
void EmMpmWorkbench::on_outputImageButton_clicked()
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
void EmMpmWorkbench::on_inputImageFilePath_textChanged(const QString & text)
{
  if (verifyPathExists(inputImageFilePath->text(), inputImageFilePath))
  {
    openBaseImageFile(text);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputImageFile_textChanged(const QString & text)
{
  //  verifyPathExists(outputImageFile->text(), movingImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_sourceDirectoryLE_textChanged(const QString & text)
{
  if (true == verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE) )
  {
    this->populateFileTable(sourceDirectoryLE, fileListWidget);
    if (fileListWidget->count() > 0) {
      QListWidgetItem* item = fileListWidget->item(0);
      on_fileListWidget_itemDoubleClicked(item);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_outputDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(outputDirectoryLE->text(), outputDirectoryLE);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::setWidgetListEnabled(bool b)
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

    /* Stopping Criteria widgets */
    m_StoppingThreshold->setEnabled(useStoppingCriteria->isChecked());
    m_StoppingThresholdLabel->setEnabled(useStoppingCriteria->isChecked());
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::setImageWidgetsEnabled(bool b)
{
  foreach (QWidget* w, m_ImageWidgets)
  {
    w->setEnabled(b);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::setProcessFolderWidgetsEnabled(bool b)
{
  foreach (QWidget* w, m_ProcessFolderWidgets)
  {
    w->setEnabled(b);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EmMpmWorkbench::verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent(fileinfo.dir());
  return parent.exists();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool EmMpmWorkbench::verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
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
qint32 EmMpmWorkbench::checkDirtyDocument()
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
void EmMpmWorkbench::updateBaseRecentFileList(const QString &file)
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
void EmMpmWorkbench::openRecentBaseImageFile()
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
void EmMpmWorkbench::on_actionOpenBaseImage_triggered()
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
void EmMpmWorkbench::on_actionOpenOverlayImage_triggered()
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
void EmMpmWorkbench::on_actionSaveCanvas_triggered()
{
  QImage image = m_GraphicsView->getOverlayImage();
  if (imageDisplaySelection->currentIndex() == 2)
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
void EmMpmWorkbench::on_actionAbout_triggered()
{
  ApplicationAboutBoxDialog about(QEMMPM::LicenseList, this);
  QString an = QCoreApplication::applicationName();
  QString version("");
  version.append(EmMpm_Workbench::Version::PackageComplete().c_str());
  about.setApplicationInfo(an, version);
  about.exec();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionExit_triggered()
{
  this->close();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::setCurrentProcessedImage(QString imageFile)
{
  openBaseImageFile(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::openBaseImageFile(QString imageFile)
{
  if ( true == imageFile.isEmpty() ) // User cancelled the operation
  {
    return;
  }

#if 0
  UserInitArea::deleteAllUserInitAreas(m_GraphicsView->scene());

  // Delete all the User Init Areas from the Scene
  enableUserDefinedAreas->setCheckState(Qt::Unchecked);
  on_enableUserDefinedAreas_stateChanged(Qt::Unchecked);
  //Clear out the UserInitAreas that we are tracking. They have all been released
  // in memory we just simply need to clear the tracking vector. This class does
  // not own the pointer so we don't worry about cleaning up the memory.
  UserInitArea::deleteAllUserInitAreas(m_GraphicsView->scene());
  m_GraphicsView->getUserInitAreas()->clear();
#endif

#if 0
  enableUserDefinedAreas->setEnabled(false);
  showUserDefinedAreas->setEnabled(false);

  showUserDefinedAreas->setChecked(false);
  enableUserDefinedAreas->setChecked(false);
  m_GraphicsView->updateDisplay();
#endif

  QFileInfo fi(imageFile);
  m_OpenDialogLastDirectory = fi.absolutePath();

  inputImageFilePath->blockSignals(true);
  inputImageFilePath->setText(imageFile);
  inputImageFilePath->blockSignals(false);

  setWindowTitle(imageFile);
  this->setWindowFilePath(imageFile);

  imageDisplaySelection->setEnabled(false);
  imageDisplaySelection->setCurrentIndex(0);

  // Tell the RecentFileList to update itself then broadcast those changes.
  QRecentFileList::instance()->addFile(imageFile);
  setWidgetListEnabled(true);
  setImageWidgetsEnabled(true);
  updateBaseRecentFileList(imageFile);

  ImageOpenDialog d(this);

  d.show();
  d.activateWindow();
  d.setModal(false);
  d.setStatus("Loading Image File....");
  m_GraphicsView->loadBaseImageFile(imageFile);
  clearGaussianCurves();
  d.setStatus("Generating Histogram....");

  m_ImageStatsReady = false;

  plotImageHistogram();
  m_HistogramPlot->setTitle(fi.fileName());

  if (perClassTableView->model() == NULL)
  {
      PerClassTableModel* pctm = new PerClassTableModel(perClassTableView);
      perClassTableView->setModel(pctm);
      QAbstractItemDelegate* pcid = pctm->getItemDelegate();
      perClassTableView->setItemDelegate(pcid);
      connect(pctm, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
              this, SLOT(perClassItemDataChanged(const QModelIndex &, const QModelIndex &)));
  }
  addRemovePerClassTableRows();

  // Create a new table model
  if (manualInitTableView->model() == NULL )
  {
      ManualInitTableModel* tm = new ManualInitTableModel(manualInitTableView);
      manualInitTableView->setModel(tm);
      QAbstractItemDelegate* aid = tm->getItemDelegate();
      manualInitTableView->setItemDelegate(aid);
      connect(tm, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
              this, SLOT(manualInitDataChanged(const QModelIndex &, const QModelIndex &)));
  }
  addRemoveManualInitTableRows();

  QSize size = m_GraphicsView->getBaseImage().size();
  estimateMemoryUse(size);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::addRemovePerClassTableRows()
{

    PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
    if (NULL == model) { return; }

    if (model->rowCount() == m_NumClasses->value())
    {
        return;
    }

    while (m_NumClasses->value() < model->rowCount())
    {
        model->removeRow(model->rowCount() - 1);
        // Remove the Gaussian Curve from the Plot
        QwtPlotCurve* curve = m_GaussianCurves.last();
        m_GaussianCurves.pop_back();
        curve->detach();
        delete curve;
    }

    // The number of classes is greater than the number of rows - Add rows
    while(m_NumClasses->value() > perClassTableView->model()->rowCount())
    {
        int count = perClassTableView->model()->rowCount();
        QString colorName = m_GaussianCurveColors[count];
        PerClassItemData* data = new PerClassItemData(count, 0.0, 4.5, colorName, count, model);
        model->insertItemData(data, model->rowCount());
        if (m_GaussianCurves.size() < model->rowCount() )
        {
            /* Update the colors of each line in the Histogram/Gaussian Plot */
            QwtPlotCurve* curve = new QwtPlotCurve("");
            QColor color = colorName;
            color.setAlpha(255);
            curve->setPen(QPen(color, 2));
            curve->setRenderHint(QwtPlotItem::RenderAntialiased);
            curve->attach(m_HistogramPlot);
            m_GaussianCurves.insert(model->rowCount(), curve);
        }
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::addRemoveManualInitTableRows()
{
  ManualInitTableModel* model = qobject_cast<ManualInitTableModel*>(manualInitTableView->model());
  if (NULL == model) { return; }
  // Check to see if the model and the number of classes match.
  if (m_NumClasses->value() == model->rowCount())
  {
    return;
  }

  while (m_NumClasses->value() < model->rowCount())
  {
    model->removeRows(model->rowCount()-1, 1);
  }

  int mVal = 0;


  // The number of classes is greater than the number of rows - Add rows
  while(m_NumClasses->value() > model->rowCount())
  {
    int count = model->rowCount() + 1;
    mVal = m_StartingMuValues[count-1];
    int defMu = mVal;

    ManualInitData* data = new ManualInitData(count-1, (double)defMu, 20.0, model);
    model->insertManualData(data, model->rowCount());
  }
  model->sanityCheckClassValues();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::manualInitDataChanged ( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    updateGaussianCurves(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::perClassItemDataChanged ( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    // THis will update all the curves with the proper data and color except if the user
    // is creating User Area Initializations at which point we really only care about
    // updating the color of the UserInitArea
    updateGaussianCurves(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::generateGaussianData(int rows)
{
    QVector<double> mean(m_NumClasses->value());
    QVector<double> variance(m_NumClasses->value());

    if (true == enableUserDefinedAreas->isChecked())
    {
        // The gaussian data is generated through another mechanism
        return;
    }
    else if (true == enableManualInit->isChecked())
    {
        for(int i = 0; i < rows; ++i)
        {
            bool ok = false;
            ManualInitTableModel* model = qobject_cast<ManualInitTableModel*>(manualInitTableView->model());
            if (NULL == model) { return; }
            QModelIndex muIndex = model->index(i, ManualInitTableModel::Mu);
            double mu = model->data(muIndex).toDouble(&ok);
            QModelIndex sigIndex = model->index(i, ManualInitTableModel::Sigma);
            double sig = model->data(sigIndex).toDouble(&ok);
            mean[i] = mu;
            variance[i] = sig;
        }
    }
    else if (m_ImageStatsReady == true)
    {
        for(int i = 0; i < rows; ++i)
        {
            int classes = rows;
            if (classes % 2 == 0)
            {
              for (int k = 0; k < classes / 2; k++) {
                  mean[classes / 2 + k] = m_MeanOfImage + (k + 1) * m_SigmaOfImage / 2;
                  mean[classes / 2 - 1 - k] = m_MeanOfImage - (k + 1) * m_SigmaOfImage / 2;
              }
            } else  {
              mean[classes / 2] = m_MeanOfImage;
              for (int k = 0; k < classes / 2; k++) {
                mean[classes / 2 + 1 + k] = m_MeanOfImage + (k + 1) * m_SigmaOfImage / 2;
                mean[classes / 2 - 1 - k] = m_MeanOfImage - (k + 1) * m_SigmaOfImage / 2;
              }
            }

            for (int l = 0; l < classes; l++) {
              variance[l] = 20.0;
            }
        }
    }
    else
    {
        QImage image = m_GraphicsView->getBaseImage();
        QSize imageSize = image.size();
      //  int bytesPerLine = image.bytesPerLine();
        int bitDepth = image.depth();
        quint8* scanLine = NULL;
        size_t elements = bitDepth/8;
        size_t offsets[3];
        if (bitDepth == 8) {offsets[0] = 0; offsets[1] = 0; offsets[2] = 0;}
        if (bitDepth == 16) {offsets[0] = 0; offsets[1] = 1; offsets[2] = 0;}
        if (bitDepth == 24) {offsets[0] = 0; offsets[1] = 1; offsets[2] = 2;}
        if (bitDepth == 32) {offsets[0] = 0; offsets[1] = 1; offsets[2] = 2;}

        quint8 r=0, g=0, b=0;
        quint32 gray = 0;
        /* Initialization of parameter estimation */
        m_MeanOfImage = 0;
        m_SigmaOfImage = 0;
        // Find the mean of the image
        for (int row = 0; row < imageSize.height(); row++)
        {
            scanLine = reinterpret_cast<quint8*>(image.scanLine(row));
            for (size_t c = 0; c < imageSize.width(); ++c)
            {
              r = scanLine[c*elements + offsets[0]];
              g = scanLine[c*elements + offsets[1]];
              b = scanLine[c*elements + offsets[2]];
              m_MeanOfImage += qGray(r, g, b);
            }
        }
        m_MeanOfImage /= (imageSize.height() * imageSize.width());
        for (int row = 0; row < imageSize.height(); row++) {
            scanLine = reinterpret_cast<quint8*>(image.scanLine(row));
            for (size_t c = 0; c < imageSize.width(); ++c) {
              r = scanLine[c*elements + offsets[0]];
              g = scanLine[c*elements + offsets[1]];
              b = scanLine[c*elements + offsets[2]];
              gray= qGray(r, g, b);
              m_SigmaOfImage += (gray - m_MeanOfImage) * (gray - m_MeanOfImage);
            }
        }


        m_SigmaOfImage /= (imageSize.height() * imageSize.width());
        m_SigmaOfImage = sqrt((real_t)m_SigmaOfImage);
        m_ImageStatsReady = true;
        int classes = rows;
        if (classes % 2 == 0)
        {
          for (int k = 0; k < classes / 2; k++) {
              mean[classes / 2 + k] = m_MeanOfImage + (k + 1) * m_SigmaOfImage / 2;
              mean[classes / 2 - 1 - k] = m_MeanOfImage - (k + 1) * m_SigmaOfImage / 2;
          }
        } else  {
          mean[classes / 2] = m_MeanOfImage;
          for (int k = 0; k < classes / 2; k++) {
            mean[classes / 2 + 1 + k] = m_MeanOfImage + (k + 1) * m_SigmaOfImage / 2;
            mean[classes / 2 - 1 - k] = m_MeanOfImage - (k + 1) * m_SigmaOfImage / 2;
          }
        }

        for (int l = 0; l < classes; l++) {
          variance[l] = 20.0;
        }
    }

    // Now generate all the gaussian curves because we have our Mean/Std Dev values
    for(int i = 0; i < rows; ++i)
    {
        QwtPlotCurve* curve = m_GaussianCurves[i];
        QwtArray<double> intervals;
        QwtArray<double> values;
        calcGaussianCurve(mean[i], variance[i], intervals, values);
        curve->setData(intervals, values);
    }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::updateGaussianCurves(bool generateNewGaussianData)
{
  if (NULL == m_histogram) { return; }


  PerClassTableModel* colorModel = qobject_cast<PerClassTableModel*>(perClassTableView->model());
  if (NULL == colorModel) { return; }

  int rows = colorModel->rowCount();
  QVector<QRgb> colorTable(rows);

  for(int i = 0; i < rows; ++i)
  {
    QwtPlotCurve* curve = NULL;
    if (m_GaussianCurves.count() > i)
    {
      curve = m_GaussianCurves[i];
    }
    else
    {
      curve = new QwtPlotCurve("");
      curve->setRenderHint(QwtPlotItem::RenderAntialiased);
      curve->attach(m_HistogramPlot);
      m_GaussianCurves.insert(i, curve);
    }
    QModelIndex colorIndex = perClassTableView->model()->index(i, PerClassTableModel::Color);
    QColor c = QColor(colorModel->data(colorIndex).toString());
    colorTable[i] = c.rgb();
    curve->setPen(QPen(c, 2, Qt::SolidLine));

    // If the user is doing Initialization Areas, find the specific row that was
    // just updated and then get the pointer to the correct UserInitArea and
    // update its color with the color from the gaussian curve
    if (true == enableUserDefinedAreas->isChecked())
    {
        int count = m_GraphicsView->getUserInitAreas()->count();
        if (i < count)
        {
            UserInitArea* uia = m_GraphicsView->getUserInitAreas()->at(i);
            if (NULL == uia) { continue; }
            QColor uicColor = c;
            uicColor.setAlpha(128);
            uia->setColor(uicColor);
        }
    }
  }

  if (generateNewGaussianData == true)
  {
    // Generate the Gaussian Data
    generateGaussianData(rows);
    //Update the combine Gaussian Curve
    plotCombinedGaussian();
  }
  // Update the plot
  m_HistogramPlot->replot();


  // Update the colors of the segmented Image
  m_GraphicsView->updateColorTables(colorTable);
  m_GraphicsView->useCustomColorTable(true);
  updateDisplayState();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::imageLoadingComplete()
{
  clearGaussianCurves();
  plotImageHistogram(); // <== This can take a while if the image is large

  QSize size = m_GraphicsView->getBaseImage().size();
  estimateMemoryUse(size);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::estimateMemoryUse(QSize size)
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
void EmMpmWorkbench::on_useCurvaturePenalty_clicked()
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_useGradientPenalty_clicked()
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_m_NumClasses_valueChanged(int i)
{
  QSize size = m_GraphicsView->getBaseImage().size();

  estimateMemoryUse(size);
  addRemovePerClassTableRows();
  addRemoveManualInitTableRows();
  updateGaussianCurves(true); // We are adding another class or removing a class, the
                              // gaussian data needs to be regenerated
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_colorTablePresets_currentIndexChanged(int i)
{
  if (colorTablePresets->currentIndex() == 0)
  {
    initializeColorTable();
  }
  else if (colorTablePresets->currentIndex() == 1)
  {
    initializeGrayScaleTable();
  }

  PerClassTableModel* colorModel = qobject_cast<PerClassTableModel*>(perClassTableView->model());
  if (NULL == colorModel) { return; }
  int rows = colorModel->rowCount();

  for(int i = 0; i < rows; ++i)
  {
    QModelIndex colorIndex = perClassTableView->model()->index(i, PerClassTableModel::Color);
    QColor c = QColor(m_GaussianCurveColors[i]);
    colorModel->setData(colorIndex, c.name()); // That will cause the 'dataChanged()' signal to be emitted
  }
  m_GraphicsView->useCustomColorTable(true);
  updateGaussianCurves(false); // We are just updating colors, do NOT update the gaussian curves
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::initializeColorTable()
{
  int index = 0;
  /* Color Names are taken from the W3C SVG Spec
   * http://www.december.com/html/spec/colorsvghex.html
   */
  m_GaussianCurveColors[index++] = QString("#191970"); // midnightblue
  m_GaussianCurveColors[index++] = QString("#8A2BE2"); // blueviolet
  m_GaussianCurveColors[index++] = QString("#8B0000"); // darkred
  m_GaussianCurveColors[index++] = QString("#FF4500"); // orangered
  m_GaussianCurveColors[index++] = QString("#800080"); // purple
  m_GaussianCurveColors[index++] = QString("#006400"); // darkgreen
  m_GaussianCurveColors[index++] = QString("#00008B"); // darkblue
  m_GaussianCurveColors[index++] = QString("#FF00FF"); // fuchsia
  m_GaussianCurveColors[index++] = QString("#2F4F4F"); // darkslategray
  m_GaussianCurveColors[index++] = QString("#48D1CC"); // mediumturquoise
  m_GaussianCurveColors[index++] = QString("#5F9EA0"); // cadetblue
  m_GaussianCurveColors[index++] = QString("#7FFF00"); // chartreuse
  m_GaussianCurveColors[index++] = QString("#D2691E"); // chocolate
  m_GaussianCurveColors[index++] = QString("#483D8B"); // darkslateblue
  m_GaussianCurveColors[index++] = QString("#4169E1"); // royalblue
  m_GaussianCurveColors[index++] = QString("#FF7F50"); // coral
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::initializeGrayScaleTable()
{
  int index = 0;
  /* Color Names are taken from the W3C SVG Spec
   * http://www.december.com/html/spec/colorsvghex.html
   */
  m_GaussianCurveColors[index++] = QString("#000000");
  m_GaussianCurveColors[index++] = QString("#FFFFFF"); //
  m_GaussianCurveColors[index++] = QString("#222222"); //
  m_GaussianCurveColors[index++] = QString("#EEEEEE"); //
  m_GaussianCurveColors[index++] = QString("#444444"); //
  m_GaussianCurveColors[index++] = QString("#DDDDDD"); //
  m_GaussianCurveColors[index++] = QString("#666666"); //
  m_GaussianCurveColors[index++] = QString("#CCCCCC"); //
  m_GaussianCurveColors[index++] = QString("#888888"); //
  m_GaussianCurveColors[index++] = QString("#BBBBBB"); //
  m_GaussianCurveColors[index++] = QString("#111111"); //
  m_GaussianCurveColors[index++] = QString("#AAAAAA"); //
  m_GaussianCurveColors[index++] = QString("#333333"); //
  m_GaussianCurveColors[index++] = QString("#555555"); //
  m_GaussianCurveColors[index++] = QString("#777777"); //
  m_GaussianCurveColors[index++] = QString("#999999"); //
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::openOverlayImage(QString processedImage)
{
  if ( true == processedImage.isEmpty() ) // User cancelled the operation
  {
    return;
  }
  m_GraphicsView->loadOverlayImageFile(processedImage);

  //if (m_LayersPalette != NULL)
  {
  //  m_LayersPalette->getOriginalImageCheckBox()->setChecked(true);
   // m_LayersPalette->getSegmentedImageCheckBox()->setChecked(true);
    imageDisplaySelection->setEnabled(true);
    imageDisplaySelection->setCurrentIndex(2);
    compositeModeCB->setCurrentIndex(EmMpm_Constants::Alpha_Blend);
    opacitySlider->setEnabled(true);
   // m_LayersPalette->getOpacitySpinBox()->setEnabled(true);
    compositeModeCB->setEnabled(true);
  }

  setWidgetListEnabled(true);
  setImageWidgetsEnabled(true);

  updateBaseRecentFileList(processedImage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::overlayImageFileLoaded(const QString &filename)
{
  // std::cout << "EmMpmWorkbench::overlayImageFileLoaded" << std::endl;
  outputImageFile->blockSignals(true);
  outputImageFile->setText(filename);
  outputImageFile->blockSignals(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::segmentedImageAvailable(QImage image)
{
    imageDisplaySelection->setEnabled(true);
    if (m_GraphicsView)
    {
        m_GraphicsView->setOverlayImage(image);
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::clearGaussianCurves()
{
  //Loop over each entry in the table
  QwtPlotCurve* curve = NULL;

  // Delete all the current histograms
  qint32 nRows = m_GaussianCurves.count();
  for (qint32 r = nRows - 1; r >= 0; --r)
  {
    curve = m_GaussianCurves[r];
    curve->detach();
    m_GaussianCurves.removeAt(r);
    delete curve;
  }

  // This is an internal variable to keep track of the class number
  m_CurrentHistogramClass = 0;
  if (NULL != m_CombinedGaussianCurve) {
    m_CombinedGaussianCurve->detach();
    delete m_CombinedGaussianCurve;
    m_CombinedGaussianCurve = NULL;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::addGaussianCurve(QVector<real_t> data)
{
  //std::cout << "EmMpmWorkbench::setProcessHistograms..... " << std::endl;
  QwtPlotCurve* curve = NULL;
  const int numValues = data.size();
  // Generate the Histogram Bins (X Axis)
  QwtArray<double> intervals(numValues);
  QwtArray<double> dValues(numValues);

  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
    dValues[i] = data[i];
  }

  QPen pen(Qt::red, 1.5, Qt::SolidLine);

  if(enableUserDefinedAreas->isChecked() == true && m_GraphicsView->getUserInitAreas()->size() > m_CurrentHistogramClass)
  {
    QColor c = m_GraphicsView->getUserInitAreas()->at(m_CurrentHistogramClass)->getColor();
    c.setAlpha(255);
    pen.setColor(c);

  }
  else
  {
    PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
    if(NULL == model)
    {
      return;
    }
    QModelIndex colorIndex = model->index(m_CurrentHistogramClass, PerClassTableModel::Color);
    QColor c = QColor(model->data(colorIndex).toString());
    pen.setColor(c);
  }

  // Create a new Plot Curve object
  curve = new QwtPlotCurve("");
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  curve->setPen(pen);
  curve->attach(m_HistogramPlot);
  m_GaussianCurves.append(curve);
  curve->setData(intervals, dValues);

  updateHistogramAxis();
  ++m_CurrentHistogramClass;

  // Only update the combined Gaussian when all the individual Gaussian's are in place
  if(m_GraphicsView->getUserInitAreas()->size() == m_CurrentHistogramClass)
  {
    plotCombinedGaussian();
  }
  else if(m_GaussianCurves.size() == m_CurrentHistogramClass)
  {
    plotCombinedGaussian();
  }
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::userInitAreaSelected(UserInitArea* uia)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::userInitAreaLostFocus()
{
  m_UserInitAreaWidget->setUserInitArea(NULL);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::calcGaussianCurve(double mu, double sig, QwtArray<double> &intervals, QwtArray<double> &values)
{
  double max = std::numeric_limits<double>::min();

  // Generate the Histogram Bins
  const int numValues = 256;
  intervals.resize(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
  }
  values.resize(numValues);
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

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::userInitAreaUpdated(UserInitArea* uia)
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
  if (err < 0)
  {
    return;
  }
  uia->setMu(mu);
  uia->setSigma(sig);

  QwtArray<double> intervals;
  QwtArray<double> values;
  calcGaussianCurve(mu, sig, intervals, values);


  // Locate our curve object by getting the row from the TableModel that corresponds
  // to the UIA object that was passed in
  int row = m_GraphicsView->getUserInitAreas()->indexOf(uia, 0);
  if (row >= m_GaussianCurves.count() )
  {
    return;
  }
  QwtPlotCurve* curve = m_GaussianCurves[row];
  curve->setData(intervals, values);

  m_UserInitAreaWidget->setUserInitArea(uia);

  //Update the combine Gaussian Curve
  plotCombinedGaussian();
  // Update the plot
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::plotImageHistogram()
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
  m_ImageHistogramData = values;

  m_AxisSettingsDialog->setXAxisMax(256.0);
  m_AxisSettingsDialog->setXAxisMin(0.0);
  m_AxisSettingsDialog->setYAxisMax(max);
  m_AxisSettingsDialog->setYAxisMin(0.0);
  updateHistogramAxis();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::plotCombinedGaussian()
{
  if (m_ShowCombinedGaussians == false)
  {
    if (NULL != m_CombinedGaussianCurve) {
      m_CombinedGaussianCurve->detach();
    }
    return;
  }
  if (m_CombinedGaussianCurve == NULL)
  {
    m_CombinedGaussianCurve = new QwtPlotCurve("Combined Gaussians");
    m_CombinedGaussianCurve->setPen(QPen(Qt::black, 3.0, Qt::SolidLine));
    m_CombinedGaussianCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
    m_CombinedGaussianCurve->attach(m_HistogramPlot);
  }

  const int numValues = 256;
  QwtArray<double> intervals(numValues);
  QwtArray<double> values(numValues);
  for (int i = 0; i < numValues; ++i)
  {
    intervals[i] = (double)i;
    values[i] = 0.0;
  }

  QList <QwtPlotCurve*> curves;
  curves = m_GaussianCurves;

  QwtPlotCurve* curve = NULL;
  int count = curves.size();
  if (count < 2) { return; } // only one Gaussian is the same as the user init Gaussian and is meaningless
  for (int c = 0; c < count; ++c)
  {
    curve = curves[c];
    QwtArrayData* data = static_cast<QwtArrayData*>(&(curve->data()));
    for (int i = 0; i < numValues; ++i)
    {
      values[i] += data->y(i);
    }
  }
  m_CombinedGaussianCurve->setData(intervals, values);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_saveMSEDataBtn_clicked()
{
    QString outputFile = getOpenDialogLastDirectory() + QDir::separator() + "MSEData.csv";
    outputFile = QFileDialog::getSaveFileName(this, tr("Save MSE Data As ..."), outputFile, tr("CSV (*.csv)"));
    if (outputFile.isNull())
    {
        return;
    }

    // Update the last directory the user visited
    QFileInfo fi(outputFile);
    setOpenDialogLastDirectory(fi.absolutePath());

    int count = m_MSEValues.size();

    // We now have a table of data, lets write it to a file
    FILE* f = fopen(outputFile.toAscii().data(), "wb");
    if (NULL == f)
    {
        QMessageBox::critical(this, tr("MSE Value Save Error"), tr("The selected output file could not be opened for writing."), QMessageBox::Ok);
        return;
    }

    // Write the header to the file
    fprintf(f, "EMLoop,MSEValue\n");

    // write the data to the file
    for (int i = 0; i < count; ++i)
    {
        fprintf(f, "%d,%f\n", i, m_MSEValues[i]);
    }

    fclose(f);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_useStoppingCriteria_clicked()
{
    m_StoppingThreshold->setEnabled(useStoppingCriteria->isChecked());
    m_StoppingThresholdLabel->setEnabled(useStoppingCriteria->isChecked());

    // These are mutually exclusive
    if (useSimulatedAnnealing->isChecked()) {
            useSimulatedAnnealing->setChecked(!useStoppingCriteria->isChecked());
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_useSimulatedAnnealing_clicked()
{

    // These are mutually exclusive
    if (useStoppingCriteria->isChecked()) {
        useStoppingCriteria->setChecked(!useSimulatedAnnealing->isChecked());
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_m_msePlotXMax_valueChanged(int value)
{
    if (m_msePlotXMin->value() < m_msePlotXMax->value() ) {
        refreshMSEPlot();
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_m_msePlotXMin_valueChanged(int value)
{
    if (m_msePlotXMin->value() < m_msePlotXMax->value() ) {
        refreshMSEPlot();
    }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::updateMSEValue(qreal value)
{
  //  std::cout << "MSE: " << value << std::endl;
    QString str("Current MSE Value: ");
    str = str.append(QString::number(value));
    m_MSEPlot->setTitle(str);
    m_MSEValues.push_back(value);


    if (m_MSEPlotCurve == NULL)
    {
        m_MSEPlotCurve = new QwtPlotCurve("Combined Gaussians");
        m_MSEPlotCurve->setPen(QPen(Qt::black, 1.0, Qt::SolidLine));
        m_MSEPlotCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

        m_MSEPlot->setCanvasBackground(QColor(Qt::white));
        m_MSEPlotCurve->setTitle("MSE (Mean & Variance)");

        QwtPlotGrid *grid = new QwtPlotGrid;
     //   grid->enableXMin(true);
      //  grid->enableYMin(true);
        grid->setMajPen(QPen(Qt::lightGray, 0, Qt::SolidLine));
      //  grid->setMinPen(QPen(Qt::lightGray, 0 , Qt::DotLine));
        grid->attach(m_MSEPlot);

#if 0
        QwtSplineCurveFitter* curveFitter = new QwtSplineCurveFitter();
        curveFitter->setSplineSize(300);
        m_MSEPlotCurve->setCurveAttribute(QwtPlotCurve::Inverted, true);
        m_MSEPlotCurve->setCurveFitter(curveFitter);
#endif

        QwtSymbol sym;
        sym.setStyle(QwtSymbol::XCross);
        sym.setPen(QPen(Qt::red,2));
        sym.setSize(4);
        m_MSEPlotCurve->setSymbol(sym);

        m_MSEPlotCurve->attach(m_MSEPlot);
    }

    const int numValues = m_MSEValues.size();
    QwtArray<double> intervals(numValues);
    QwtArray<double> values(numValues);
    for (int i = 0; i < numValues; ++i)
    {
      intervals[i] = (double)i;
      values[i] = m_MSEValues[i];
    }

    m_MSEPlotCurve->setData(intervals, values);

#if 0
    refreshMSEPlot();
#endif

    m_msePlotXMax->setMaximum(m_MSEValues.size());
    m_msePlotXMin->setMaximum(m_MSEValues.size()-1);
    m_msePlotXMax->setValue(m_MSEValues.size());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::refreshMSEPlot()
{
    int xmin = m_msePlotXMin->value();
    int xmax = m_msePlotXMax->value();

    if (xmax > m_MSEValues.size())
    {
        return;
    }

    qreal max = 0.0;

    for (int i = xmin; i < xmax; ++i)
    {
        if (m_MSEValues[i] > max)
        {
            max = m_MSEValues[i];
        }
    }

    m_MSEPlot->setAxisScale(QwtPlot::xBottom, xmin, xmax);
    m_MSEPlot->setAxisScale(QwtPlot::yLeft, 0, max * 1.10);

    m_MSEPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::deleteUserInitArea(UserInitArea* uia)
{
  int row = m_GraphicsView->getUserInitAreas()->indexOf(uia, 0);
  if (row >= m_GaussianCurves.count() )
  {
    return;
  }
  QwtPlotCurve* curve = m_GaussianCurves[row];
  m_GaussianCurves.removeAll(curve);

  curve->detach();
  delete curve; // Clean up the memory
  m_HistogramPlot->replot();

  uia = m_GraphicsView->getUserInitAreas()->at(row);
  m_GraphicsView->getUserInitAreas()->remove(row);


  // Now renumber the classes:
  qint32 size = m_GraphicsView->getUserInitAreas()->size();
  for(qint32 i = 0; i < size; ++i)
  {
    m_GraphicsView->getUserInitAreas()->at(i)->setEmMpmClass(i);
  }
  m_NumClasses->setValue(m_GraphicsView->getUserInitAreas()->size());
  if (m_GraphicsView->getUserInitAreas()->size() == 0)
  {
    m_NumClasses->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::userInitAreaAdded(UserInitArea* uia)
{
  m_UserInitAreaWidget->setUserInitArea(uia);

  if (NULL == uia) { return; }

  addUserInitArea->setChecked(false);

  // Figure out the proper row to insert the curve object to keep it in sync with
  // the table model
  int row = m_GraphicsView->getUserInitAreas()->indexOf(uia, 0);
  // This will have the cascade effect of removing or adding rows to both the PerClass
  // TableView and the Manual Initialization Table View
  m_NumClasses->setValue(m_GraphicsView->getUserInitAreas()->size());
  if (m_GraphicsView->getUserInitAreas()->size() != 0)
  {
    m_NumClasses->setEnabled(false);
  }

  // This model should have been properly updated at this point along with the list of
  // Gaussian Curves
  PerClassTableModel* model = qobject_cast<PerClassTableModel*>(perClassTableView->model());
  if (NULL == model) { return; }
  QList<PerClassItemData*> items = model->getItemDatas();
  // Get the color name from the PerClassItem object for the specific row
  QString colorName = items.at(row)->getColor();
  QColor color = colorName;
  color.setAlpha(128);
  uia->setColor(color);


  // Now update the curve with the initial data
  userInitAreaUpdated(uia);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::populateFileTable(QLineEdit* sourceDirectoryLE, QListWidget *fileListView)
{
  QDir sourceDir(sourceDirectoryLE->text());
  sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList strList = sourceDir.entryList();

  // Load the fileListWidget
  fileListWidget->clear();
  for(int i = 0; i < strList.count(); ++i)
  {
    QListWidgetItem* item  = new QListWidgetItem(strList.at(i));
    fileListWidget->addItem(item);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_filterPatternLineEdit_textChanged()
{

  QDir sourceDir(sourceDirectoryLE->text());
  sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList strList = sourceDir.entryList();
  strList = strList.filter(filterPatternLineEdit->text(), Qt::CaseInsensitive);
  // Load the fileListWidget
  fileListWidget->clear();
  for(int i = 0; i < strList.count(); ++i)
  {
    QListWidgetItem* item  = new QListWidgetItem(strList.at(i));
    fileListWidget->addItem(item);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList EmMpmWorkbench::generateInputFileList()
{
  QStringList list;
  int count = fileListWidget->count();
  for(int i = 0; i < count; ++i)
  {
    list.append(fileListWidget->item(i)->text());
  }
  return list;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::updateHistogramAxis()
{
  double min, max;
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
void EmMpmWorkbench::on_axisSettingsBtn_clicked()
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
void EmMpmWorkbench::on_saveCurves_clicked()
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
  int count = m_GaussianCurves.size();

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
      curve = m_GaussianCurves[c];
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
void EmMpmWorkbench::on_clearTempHistograms_clicked()
{
  std::cout << "EmMpmWorkbench::on_clearTempHistograms_clicked()" << std::endl;
  clearGaussianCurves();
  plotImageHistogram();
  if (enableUserDefinedAreas->isChecked() )
  {
    int size = m_GraphicsView->getUserInitAreas()->count();
    UserInitArea* uia = NULL;
    for (int r = 0; r < size; ++r)
    {
      uia = m_GraphicsView->getUserInitAreas()->at(r);
      userInitAreaAdded(uia);
    }
  }
  addUserInitArea->setChecked(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_showUserDefinedAreas_stateChanged(int state)
{
  int size = m_GraphicsView->getUserInitAreas()->count();
  UserInitArea* uia = NULL;

  for (int r = 0; r < size; ++r)
  {
    uia = m_GraphicsView->getUserInitAreas()->value(r, NULL);
    if(uia)
    {
      if(showUserDefinedAreas->isChecked())
      {
        uia->setVisible(true);
        if(m_GraphicsView->scene() != uia->scene())
        {
          m_GraphicsView->scene()->addItem(uia);
        }
        userInitAreaAdded(uia);
      }
      else
      {
        if(uia->scene() == m_GraphicsView->scene())
        {
          m_GraphicsView->scene()->removeItem(uia);
          uia->setVisible(false);
        }
      }
    }
  }
  m_GraphicsView->updateDisplay();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_enableUserDefinedAreas_stateChanged(int state)
{
  m_NumClasses->setEnabled( !enableUserDefinedAreas->isChecked() );
  showUserDefinedAreas->setEnabled( enableUserDefinedAreas->isChecked() );
  if (enableUserDefinedAreas->isChecked() == false)
  {
    showUserDefinedAreas->setChecked(false);
  }

  m_UserInitAreaWidget->setUserInitArea(NULL);
  if(enableUserDefinedAreas->isChecked() == false )
  {
    updateGaussianCurves(true); // We need to generate the Gaussian curves for the User Init Area Mu/Sigma values
  }
  m_HistogramPlot->replot();
  if(enableUserDefinedAreas->isChecked() == true) {
    enableManualInit->setChecked(!enableUserDefinedAreas->isChecked());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_enableManualInit_stateChanged(int state)
{
  if (enableManualInit->isChecked() == true)
  {
    enableUserDefinedAreas->setChecked(!enableManualInit->isChecked());
  }
  // The user is enabling or disabling the manual init table so we need to regenerate the gaussian data
  updateGaussianCurves(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaWidget* EmMpmWorkbench::getUserInitAreaWidget()
{
  return m_UserInitAreaWidget;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionUser_Initialization_triggered()
{
  UserInitAreaDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionHistogram_triggered()
{
  HistogramDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionParameters_triggered()
{
  ParametersDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_actionMSE_Plot_triggered()
{
    m_MSEDockWidget->show();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_addClassCoupling_clicked()
{
  if(m_ClassA->text().isEmpty() == false
      && m_ClassB->text().isEmpty() == false
      && m_CouplingBeta->text().isEmpty() == false)
  {
    int a = m_ClassA->text().toInt();
    if(a >= m_NumClasses->value())
    {
      return;
    }
    int b = m_ClassB->text().toInt();
    if(b >= m_NumClasses->value())
    {
      return;
    }

    if(a >= b)
    {
      QMessageBox::critical(this, tr("Class Coupling Error"), tr("Class B must be a larger value than Class A. Input (1,2) instead of (2,1)"), QMessageBox::Ok);
      return;
    }
    int rc = m_ClassCouplingTableWidget->rowCount();
    m_ClassCouplingTableWidget->insertRow(rc);

    QTableWidgetItem* classA_WI = new QTableWidgetItem(m_ClassA->text());
    classA_WI->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem* classB_WI = new QTableWidgetItem(m_ClassB->text());
    classB_WI->setTextAlignment(Qt::AlignCenter);

    QTableWidgetItem* beta_WI = new QTableWidgetItem(m_CouplingBeta->text());
    beta_WI->setTextAlignment(Qt::AlignCenter);

    m_ClassCouplingTableWidget->setItem(rc, 0, classA_WI);
    m_ClassCouplingTableWidget->setItem(rc, 1, classB_WI);
    m_ClassCouplingTableWidget->setItem(rc, 2, beta_WI);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_removeClassCoupling_clicked()
{
  QModelIndexList indices = m_ClassCouplingTableWidget->selectionModel()->selection().indexes();
  int count = indices.count();
  for(int i = count - 1; i >= 0; --i)
  {
    QModelIndex index = indices.at(i);
    m_ClassCouplingTableWidget->removeRow(index.row());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_imageDisplaySelection_currentIndexChanged(int index)
{
    updateDisplayState();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_opacitySlider_valueChanged(int value)
{
    float f = (float)value/100.0;
    m_GraphicsView->setOverlayTransparency(f);
    m_GraphicsView->updateDisplay();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::on_compositeModeCB_currentIndexChanged()
{
    //  std::cout << "on_compositeModeCB_indexChanged" << std::endl;
      bool ok = false;
      EmMpm_Constants::CompositeType cType = static_cast<EmMpm_Constants::CompositeType>(compositeModeCB->itemData(compositeModeCB->currentIndex()).toInt(&ok));
      m_GraphicsView->setImageDisplayType(EmMpm_Constants::CompositedImage);
      m_GraphicsView->setCompositeMode(cType);
      if (cType == EmMpm_Constants::Alpha_Blend)
      {
        opacitySlider->setEnabled(true);
      }
      else
      {
        opacitySlider->setEnabled(false);
      }
      m_GraphicsView->updateDisplay();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmWorkbench::updateDisplayState()
{
    float f = (float)opacitySlider->value()/100.0;
    m_GraphicsView->setOverlayTransparency(f);

    bool ok = false;
    // Display only the original image only
    if (imageDisplaySelection->currentIndex() == 0)
    {
      m_GraphicsView->setImageDisplayType(EmMpm_Constants::OriginalImage);
      opacitySlider->setEnabled(false);
      compositeModeCB->setEnabled(false);
    }
    else if ( imageDisplaySelection->currentIndex() == 1)
    {
      m_GraphicsView->setImageDisplayType(EmMpm_Constants::SegmentedImage);
      opacitySlider->setEnabled(false);
      compositeModeCB->setEnabled(false);
    }
    else if (imageDisplaySelection->currentIndex() == 2)
    {
      m_GraphicsView->setImageDisplayType(EmMpm_Constants::CompositedImage);
      EmMpm_Constants::CompositeType cType = static_cast<EmMpm_Constants::CompositeType>(compositeModeCB->itemData(compositeModeCB->currentIndex()).toInt(&ok));
      m_GraphicsView->setCompositeMode(cType);
      compositeModeCB->setEnabled(true);
      if (cType == EmMpm_Constants::Alpha_Blend)
      {
        opacitySlider->setEnabled(true);
      }
      else
      {
        opacitySlider->setEnabled(false);
      }
    }
    else
    {
      m_GraphicsView->setImageDisplayType(EmMpm_Constants::UnknownDisplayType);
      opacitySlider->setEnabled(false);
      compositeModeCB->setEnabled(false);
    }
    m_GraphicsView->updateDisplay();
}
