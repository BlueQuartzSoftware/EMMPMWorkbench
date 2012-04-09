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

#include <iostream>
#include "ManualInitTableModel.h"

#include <QApplication>
#include <QtGui/QAbstractItemDelegate>

#include "ManualInitData.h"
#include "ManualInitDataItemDelegate.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ManualInitTableModel::ManualInitTableModel(QObject* parent) :
QAbstractTableModel(parent),
m_RowCount(0)
{
  m_ColumnCount = ColumnCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ManualInitTableModel::~ManualInitTableModel()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags ManualInitTableModel::flags(const QModelIndex &index) const
{
  //  std::cout << "SGLogNormalTableModel::flags" << std::endl;
  if (!index.isValid())
  {
    return Qt::NoItemFlags;
  }
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid())
  {
    theFlags |= Qt::ItemIsEnabled;

    int col = index.column();
    if ( col == GrayValue  || col == Mu || col == StdDev)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

  }
  return theFlags;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant ManualInitTableModel::data(const QModelIndex &index, qint32 role) const
{

  if (! index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::SizeHintRole)
  {


  }
  else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    ManualInitData* uia = m_ManualInitDatas.at(index.row());
    if (NULL == uia)
    {
      return QVariant();
    }

    QString s;
  //  unsigned int x, y;

    int col = index.column();
    switch(col)
    {
      case ManualInitTableModel::Class:
        return QVariant(uia->getEmMpmClass());
      case ManualInitTableModel::GrayValue:
        return QVariant(uia->getEmMpmGrayLevel());
      case ManualInitTableModel::Mu:
        return QVariant(uia->getMu());
      case ManualInitTableModel::StdDev:
        return QVariant(uia->getSigma());
      default:
        break;
    }

  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant  ManualInitTableModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch(section)
    {
      case Class: return QVariant(QString("Class"));
      case GrayValue: return QVariant(QString("Gray Value"));
      case Mu: return QVariant(QString("Mu"));
      case StdDev: return QVariant(QString("Sigma"));
      default:
        break;
    }
  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ManualInitTableModel::rowCount(const QModelIndex &index) const
{
  return m_ManualInitDatas.count();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ManualInitTableModel::columnCount(const QModelIndex &index) const
{
  return index.isValid() ? 0 : m_ColumnCount;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ManualInitTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  // std::cout << "SGLogNormalTableModel::setData " << value.toString().toStdString() << std::endl;
  if (!index.isValid()
      || role != Qt::EditRole
      || index.row() < 0
      || index.row() >= m_ManualInitDatas.count()
      || index.column() < 0
      || index.column() >= m_ColumnCount)
  {
    return false;
  }
  bool ok;
  qint32 row = index.row();
  qint32 col = index.column();
  if (row >= rowCount(index))
  {
    return false;
  }
  switch(col)
  {
    case ManualInitTableModel::Class:
      m_ManualInitDatas.at(row)->setEmMpmClass(value.toInt(&ok));
      break;
    case ManualInitTableModel::GrayValue:
      m_ManualInitDatas.at(row)->setEmMpmGrayLevel(value.toInt(&ok));
      break;
    case ManualInitTableModel::Mu:
      m_ManualInitDatas.at(row)->setMu(value.toDouble(&ok));
      break;
    case ManualInitTableModel::StdDev:
      m_ManualInitDatas.at(row)->setSigma(value.toDouble(&ok));
      break;
    default:
      Q_ASSERT(false);
  }
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ManualInitTableModel::insertRows(int row, int count, const QModelIndex& index)
{
  // This is basically disabled at this point
  qint32 binNum = 0;
  double mu = 128.0;
  double sigma = 20.0;


  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    // Create a new ManualInitData object
    ManualInitData* d = new ManualInitData(m_ManualInitDatas.count(), mu, sigma, 128, this);
    m_ManualInitDatas.push_back(d);
    m_RowCount = m_ManualInitDatas.count();
  }
  endInsertRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ManualInitTableModel::insertManualData(ManualInitData* data, int row, const QModelIndex& index)
{
  qint32 binNum = 0;
  double mu = 128.0;
  double sigma = 20.0;

  beginInsertRows(QModelIndex(), row, row );
  for (int i = 0; i < 1; ++i)
  {
    // Create a new ManualInitData object
    m_ManualInitDatas.push_back(data);
    m_RowCount = m_ManualInitDatas.count();
  }
  endInsertRows();
  emit dataChanged(index, index);
  return true;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ManualInitTableModel::removeRows(int row, int count, const QModelIndex& index)
{
  if (count < 1)
  {
    return true;
  } // No Rows to remove
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    // Remove the UIA Object Pointer
    m_ManualInitDatas.removeAt(row + i);
    m_RowCount = m_ManualInitDatas.count();
  }
  endRemoveRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitTableModel::addManualInitData(ManualInitData* uia)
{
  m_ManualInitDatas.push_back(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitTableModel::deleteManualInitData(ManualInitData* uia)
{
  m_ManualInitDatas.removeAll(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ManualInitTableModel::updateManualInitData(ManualInitData* uia)
{
  int row = m_ManualInitDatas.indexOf(uia, 0);
  QModelIndex index0 = createIndex(row, 0);
  QModelIndex index1 = createIndex(row, m_ColumnCount);
  emit dataChanged(index0, index1);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAbstractItemDelegate* ManualInitTableModel::getItemDelegate()
{
  return new ManualInitDataItemDelegate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<ManualInitData*> ManualInitTableModel::getManualInits()
{
  return m_ManualInitDatas;
}

