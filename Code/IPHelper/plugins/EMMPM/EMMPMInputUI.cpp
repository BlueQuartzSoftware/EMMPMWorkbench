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

#include "EMMPMInputUI.h"
//-- C++ includes
#include <iostream>

//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/QFileInfoList>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>
#include <QtGui/QStringListModel>

// Our Project wide includes
#include "QtSupport/QFileCompleter.h"
#include "QtSupport/ImageGraphicsDelegate.h"
#include "QtSupport/ProcessQueueController.h"
#include "QtSupport/ProcessQueueDialog.h"

// Our Own Includes
#include "EMMPMTask.h"

#include "EIMApps/Common/EIMAppsVersion.h"

#define READ_STRING_SETTING(prefs, var, emptyValue)\
  var->setText( prefs->value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }

#define READ_SETTING(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs->value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var->setValue(temp);

#define WRITE_STRING_SETTING(prefs, var)\
  prefs->setValue(#var , this->var->text());

#define WRITE_SETTING(prefs, var)\
  prefs->setValue(#var, this->var->value());

#define READ_BOOL_SETTING(prefs, var, emptyValue)\
  { QString s = prefs->value(#var).toString();\
  if (s.isEmpty() == false) {\
    bool bb = prefs->value(#var).toBool();\
  var->setChecked(bb); } else { var->setChecked(emptyValue); } }

#define WRITE_BOOL_SETTING(prefs, var, b)\
    prefs->setValue(#var, (b) );

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMInputUI::EMMPMInputUI(QWidget *parent) :
  QImageProcessingInputFrame(parent)
{
  setupUi(this);

  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EMMPMInputUI::~EMMPMInputUI()
{

}

// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void EMMPMInputUI::readSettings(QSettings* prefs)
{
  QString val;
  bool ok;
  qint32 i;
  prefs->beginGroup("EMMPMPlugin");
  READ_STRING_SETTING(prefs, m_Beta, "5.5");
  READ_STRING_SETTING(prefs, m_Gamma, "0.1");
  READ_SETTING(prefs, m_MpmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_EmIterations, ok, i, 5, Int);
  READ_SETTING(prefs, m_NumClasses, ok, i, 2, Int);
  READ_BOOL_SETTING(prefs, useSimulatedAnnealing, true);
  READ_BOOL_SETTING(prefs, processFolder, false);
  READ_STRING_SETTING(prefs, fixedImageFile, "");
  READ_STRING_SETTING(prefs, outputImageFile, "");
  READ_STRING_SETTING(prefs, sourceDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputDirectoryLE, "");
  READ_STRING_SETTING(prefs, outputPrefix, "");
  READ_STRING_SETTING(prefs, outputSuffix, "");
  prefs->endGroup();
  on_processFolder_stateChanged(processFolder->isChecked());

  if (this->sourceDirectoryLE->text().isEmpty() == false)
  {
    this->populateFileTable(this->sourceDirectoryLE, this->fileListView);
  }

}

// -----------------------------------------------------------------------------
//  Write our prefs to file
// -----------------------------------------------------------------------------
void EMMPMInputUI::writeSettings(QSettings* prefs)
{
 // std::cout << "EMMPMInputUI::writeSettings" << std::endl;

  prefs->beginGroup("EMMPMPlugin");
  WRITE_STRING_SETTING(prefs, m_Beta);
  WRITE_STRING_SETTING(prefs, m_Gamma);
  WRITE_SETTING(prefs, m_MpmIterations);
  WRITE_SETTING(prefs, m_EmIterations);
  WRITE_SETTING(prefs, m_NumClasses);
  WRITE_BOOL_SETTING(prefs, useSimulatedAnnealing, useSimulatedAnnealing->isChecked());
  WRITE_BOOL_SETTING(prefs, processFolder, processFolder->isChecked());
  WRITE_STRING_SETTING(prefs, fixedImageFile);
  WRITE_STRING_SETTING(prefs, outputImageFile);
  WRITE_STRING_SETTING(prefs, sourceDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputDirectoryLE);
  WRITE_STRING_SETTING(prefs, outputPrefix);
  WRITE_STRING_SETTING(prefs, outputSuffix);
  prefs->endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::setupGui()
{
  QFileCompleter* com = new QFileCompleter(this, false);
  fixedImageFile->setCompleter(com);
  QObject::connect(com, SIGNAL(activated(const QString &)), this, SLOT(on_fixedImageFile_textChanged(const QString &)));

  QFileCompleter* com4 = new QFileCompleter(this, false);
  outputImageFile->setCompleter(com4);
  QObject::connect(com4, SIGNAL(activated(const QString &)), this, SLOT(on_outputImageFile_textChanged(const QString &)));

  QFileCompleter* com2 = new QFileCompleter(this, true);
  sourceDirectoryLE->setCompleter(com2);
  QObject::connect(com2, SIGNAL(activated(const QString &)), this, SLOT(on_sourceDirectoryLE_textChanged(const QString &)));

  QFileCompleter* com3 = new QFileCompleter(this, true);
  outputDirectoryLE->setCompleter(com3);
  QObject::connect(com3, SIGNAL(activated(const QString &)), this, SLOT(on_outputDirectoryLE_textChanged(const QString &)));

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int EMMPMInputUI::processInputs(QObject* parentGUI)
{

  /* If the 'processFolder' checkbox is checked then we need to check for some
   * additional inputs
   */
  if (this->processFolder->isChecked())
  {
    if (this->sourceDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Input Parameter Error"), tr("Source Directory must be set."), QMessageBox::Ok);
      return -1;
    }

    if (this->outputDirectoryLE->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Parameter Error"), tr("Output Directory must be set."), QMessageBox::Ok);
      return -1;
    }

    if (this->fileListView->model()->rowCount() == 0)
    {
      QMessageBox::critical(this, tr("Parameter Error"), tr("No image files are available in the file list view."), QMessageBox::Ok);
      return -1;
    }

    QDir outputDir(this->outputDirectoryLE->text());
    if (outputDir.exists() == false)
    {
      bool ok = outputDir.mkpath(".");
      if (ok == false)
      {
        QMessageBox::critical(this, tr("Output Directory Creation"), tr("The output directory could not be created."), QMessageBox::Ok);
        return -1;
      }
    }

  }
  else
  {
    QFileInfo fi(fixedImageFile->text());
    if (fi.exists() == false)
    {
      QMessageBox::critical(this, tr("Fixed Image File Error"), tr("Fixed Image does not exist. Please check the path."), QMessageBox::Ok);
      return -1;
    }

    if (outputImageFile->text().isEmpty() == true)
    {
      QMessageBox::critical(this, tr("Output Image File Error"), tr("Please select a file name for the registered image to be saved as."), QMessageBox::Ok);
      return -1;
    }
    QFile file(outputImageFile->text());
    if (file.exists() == true)
    {
      int ret = QMessageBox::warning(this, tr("QEM/MPM"), tr("The Output File Already Exists\nDo you want to over write the existing file?"), QMessageBox::No
          | QMessageBox::Default, QMessageBox::Yes, QMessageBox::Cancel);
      if (ret == QMessageBox::Cancel)
      {
        return -1;
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
          return -1;
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
    task->setBeta(m_Beta->text().toFloat(&ok));
    task->setGamma(m_Gamma->text().toFloat(&ok));
    task->setEmIterations(m_EmIterations->value());
    task->setMpmIterations(m_MpmIterations->value());
    task->setNumberOfClasses(m_NumClasses->value());
    if (useSimulatedAnnealing->isChecked())
    {
      task->useSimulatedAnnealing();
    }
    task->setInputFilePath(fixedImageFile->text());
    task->setOutputFilePath(outputImageFile->text());
    filepairs.append(InputOutputFilePair(task->getInputFilePath(), task->getOutputFilePath()));
    queueController->addTask(static_cast<QThread* > (task));
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
      task->setBeta(m_Beta->text().toFloat(&ok));
      task->setGamma(m_Gamma->text().toFloat(&ok));
      task->setEmIterations(m_EmIterations->value());
      task->setMpmIterations(m_MpmIterations->value());
      task->setNumberOfClasses(m_NumClasses->value());
      if (useSimulatedAnnealing->isChecked())
      {
        task->useSimulatedAnnealing();
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
      filepairs.append(InputOutputFilePair(task->getInputFilePath(), task->getOutputFilePath()));
      queueController->addTask(static_cast<QThread* > (task));
      this->addProcess(task);
    }

  }
  setInputOutputFilePairList(filepairs);

  // When the event loop of the controller starts it will signal the ProcessQueue to run
  connect(queueController, SIGNAL(started()), queueController, SLOT(processTask()));
  // When the QueueController finishes it will signal the QueueController to 'quit', thus stopping the thread
  connect(queueController, SIGNAL(finished()), this, SLOT(queueControllerFinished()));

  connect(queueController, SIGNAL(started()), parentGUI, SLOT(processingStarted()));
  connect(queueController, SIGNAL(finished()), parentGUI, SLOT(processingFinished()));

//  getQueueDialog()->setParent(this);
  getQueueDialog()->setVisible(true);

  queueController->start();

  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::addProcess(EMMPMTask* task)
{
  getQueueDialog()->addProcess(task);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::queueControllerFinished()
{
  getQueueDialog()->setVisible(false);
  if (this->processFolder->isChecked() == false)
  {
    setCurrentImageFile (fixedImageFile->text());
    setCurrentProcessedFile(outputImageFile->text());
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
void EMMPMInputUI::on_processFolder_stateChanged(int state)
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

  fixedImageFile->setEnabled(!enabled);
  fixedImageButton->setEnabled(!enabled);

  outputImageFile->setEnabled(!enabled);
  outputImageButton->setEnabled(!enabled);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_filterPatternLineEdit_textChanged()
{
  // std::cout << "filterPattern: " << std::endl;
  getProxyModel()->setFilterFixedString(filterPatternLineEdit->text());
  getProxyModel()->setFilterCaseSensitivity(Qt::CaseInsensitive);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputPrefix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputSuffix_textChanged()
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputImageType_currentIndexChanged(int index)
{
  outputFilenamePattern->setText(outputPrefix->text() + "[ORIGINAL FILE NAME]" + outputSuffix->text() + "." + outputImageType->currentText() );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_sourceDirectoryBtn_clicked()
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
void EMMPMInputUI::on_outputDirectoryBtn_clicked()
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
void EMMPMInputUI::on_fixedImageButton_clicked()
{
  //std::cout << "on_actionOpen_triggered" << std::endl;
  QString imageFile =
      QFileDialog::getOpenFileName(this, tr("Select Fixed Image"), getOpenDialogLastDirectory(), tr("Images (*.tif *.tiff *.bmp *.jpg *.jpeg *.png)"));

  if (true == imageFile.isEmpty())
  {
    return;
  }
  fixedImageFile->setText(imageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputImageButton_clicked()
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
void EMMPMInputUI::on_fixedImageFile_textChanged(const QString & text)
{
  verifyPathExists(fixedImageFile->text(), fixedImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputImageFile_textChanged(const QString & text)
{
  //  verifyPathExists(outputImageFile->text(), movingImageFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_sourceDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(sourceDirectoryLE->text(), sourceDirectoryLE);
  this->populateFileTable(sourceDirectoryLE, fileListView);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EMMPMInputUI::on_outputDirectoryLE_textChanged(const QString & text)
{
  verifyPathExists(outputDirectoryLE->text(), outputDirectoryLE);
}
