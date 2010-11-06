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


#include "UserInitAreaTableModel.h"
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_interval_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>

#include "histogram_item.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
EmMpmGui::EmMpmGui(QWidget *parent) :
QMainWindow(parent),
m_zoomer(NULL),
m_picker(NULL),
m_panner(NULL),
 m_grid(NULL),
 m_histogram(NULL)
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
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void EmMpmGui::closeEvent(QCloseEvent *event)
{
//  qint32 err = _checkDirtyDocument();
//  if (err < 0)
//  {
//    event->ignore();
//  }
//  else
  {
   // writeSettings();
    event->accept();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::on_actionClose_triggered() {
  //std::cout << "on_actionClose_triggered" << std::endl;
  qint32 err = 0;
//  err = _checkDirtyDocument();
  if (err >= 0)
  {
    // Close the window. Files have been saved if needed
    this->close();
  }
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
#endif

  compositeModeCB->setCurrentIndex(0);
  compositeModeCB->blockSignals(false);

  QHeaderView* headerView = new QHeaderView(Qt::Horizontal, m_UserInitTable);
  headerView->setResizeMode(QHeaderView::Interactive);
  m_UserInitTable->setHorizontalHeader(headerView);
  m_UserInitAreaTableModel = new UserInitAreaTableModel;
  m_GraphicsView->setUserInitAreaTableModel(m_UserInitAreaTableModel);
  m_UserInitTable->setModel(m_UserInitAreaTableModel);
  headerView->show();



  connect (m_GraphicsView, SIGNAL(fireImageFileLoaded(const QString &)),
           this, SLOT(imageFileLoaded(const QString &)), Qt::QueuedConnection);

  connect (addUserInitArea, SIGNAL(toggled(bool)),
           m_GraphicsView, SLOT(addUserInitArea(bool)));

  connect (m_GraphicsView, SIGNAL(fireUserInitAreaAdded(UserInitArea*)),
           this, SLOT(userInitAreaAdded(UserInitArea*)), Qt::QueuedConnection);

  connect (zoomIn, SIGNAL(clicked()),
           m_GraphicsView, SLOT(zoomIn()), Qt::QueuedConnection);
  connect(zoomOut, SIGNAL(clicked()),
          m_GraphicsView, SLOT(zoomOut()), Qt::QueuedConnection);

  connect(fitToWindow, SIGNAL(clicked()),
          m_GraphicsView, SLOT(fitToWindow()), Qt::QueuedConnection);


  connect (compositeModeCB, SIGNAL(currentIndexChanged(int)),
           m_GraphicsView, SLOT(setCompositeMode(int)), Qt::QueuedConnection);

  connect (zoomCB, SIGNAL(currentIndexChanged(int)),
           m_GraphicsView, SLOT(setZoomIndex(int)), Qt::QueuedConnection);


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
void EmMpmGui::imageFileLoaded(const QString &filename)
{
  std::cout << "Loaded Image file " << filename.toStdString() << std::endl;
  this->setWindowFilePath(filename);
  plotImageHistogram();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::plotImageHistogram()
{
  QImage image = m_GraphicsView->getCurrentImage();
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

  QImage image = m_GraphicsView->getCurrentImage();
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
  sig = 10.0;

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
  mu /= (yEnd - yStart)*(xEnd - xStart);

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

  double binSize = m_histogram->y( max_index );
  for (size_t x = 0; x < 256; ++x)
  {
    values[x] = (values[x]/max) * binSize;
  }

  // Locate our curve object by getting the row from the TableModel that corresponds
  // to the UIA object that was passed in
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);
  QwtPlotCurve* curve = m_UIAGaussians[row];
  curve->setData(intervals, values);
  QColor c = uia->getColor();
  curve->setPen(QPen(c));

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
  QwtPlotCurve* curve = m_UIAGaussians[row];
  m_UIAGaussians.removeAll(curve);
  curve->detach();
  delete curve; // Clean up the memory
  m_HistogramPlot->replot();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void EmMpmGui::userInitAreaAdded(UserInitArea* uia)
{
//  std::cout << "EmMpmGui::userInitAreaAdded(UserInitArea* uia)" << std::endl;
  if (NULL == uia) { return; }
  addUserInitArea->toggle();
  QwtPlotCurve* curve = new QwtPlotCurve("User Init Area");
  curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  QColor color = uia->getColor();
  curve->setPen(QPen(color));
  curve->attach(m_HistogramPlot);

  // Figure out the proper row to insert the curve object to keep it in sync with
  // the table model
  QList<UserInitArea*> userInitAreas = m_UserInitAreaTableModel->getUserInitAreas();
  int row = userInitAreas.indexOf(uia, 0);

  m_UIAGaussians.insert(row, curve);

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
