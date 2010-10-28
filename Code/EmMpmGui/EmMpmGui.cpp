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
#include "UserInitAreaTableModel.h"

#include <iostream>
#include <sstream>
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EmMpmGui::EmMpmGui(QWidget *parent) :
QMainWindow(parent)
{
  setupUi(this);
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EmMpmGui::~EmMpmGui()
{

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
#endif

  compositeModeCB->setCurrentIndex(0);
  compositeModeCB->blockSignals(false);


  m_UserInitAreaTableModel = new UserInitAreaTableModel;
  m_GraphicsView->setUserInitAreaTableModel(m_UserInitAreaTableModel);
  userInitTable->setModel(m_UserInitAreaTableModel);


  connect (m_GraphicsView, SIGNAL(fireImageFileLoaded(const QString &)),
           this, SLOT(imageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (addUserInitArea, SIGNAL(toggled(bool)),
           m_GraphicsView, SLOT(addUserInitArea(bool)), Qt::QueuedConnection);

  connect (m_GraphicsView, SIGNAL(fireUserInitAreaAdded()),
           addUserInitArea, SLOT(toggle()), Qt::QueuedConnection);

//  connect (m_GraphicsView, SIGNAL(fireUserInitAreaAdded(bool, int, const QRectF&)),
//           this, SLOT(userInitAreaAdded(bool, int, const QRectF&)), Qt::QueuedConnection);

//  connect (zoomCB, SIGNAL(currentIndexChanged(int)),
//           m_GraphicsView, SLOT(setZoomIndex(int)), Qt::QueuedConnection);

  connect (zoomIn, SIGNAL(clicked()),
           m_GraphicsView, SLOT(zoomIn()), Qt::QueuedConnection);
  connect(zoomOut, SIGNAL(clicked()),
          m_GraphicsView, SLOT(zoomOut()), Qt::QueuedConnection);
  connect(fitToWindow, SIGNAL(clicked()),
          m_GraphicsView, SLOT(fitToWindow()), Qt::QueuedConnection);

  connect (compositeModeCB, SIGNAL(currentIndexChanged(int)),
           m_GraphicsView, SLOT(setCompositeMode(int)), Qt::QueuedConnection);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::imageFileLoaded(const QString &filename)
{
  std::cout << "Loaded Image file " << filename.toStdString() << std::endl;
  this->setWindowFilePath(filename);
}








