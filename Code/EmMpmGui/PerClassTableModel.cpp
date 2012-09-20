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
#include "PerClassTableModel.h"

#include <QApplication>
#include <QtGui/QAbstractItemDelegate>

#include "PerClassItemData.h"
#include "PerClassItemDelegate.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PerClassTableModel::PerClassTableModel(QObject* parent) :
QAbstractTableModel(parent),
m_RowCount(0)
{
  m_ColumnCount = ColumnCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PerClassTableModel::~PerClassTableModel()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags PerClassTableModel::flags(const QModelIndex &index) const
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
    if (  col == Gamma || col == MinStdDev|| col == Gray || col == Color )
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

  }
  return theFlags;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant PerClassTableModel::data(const QModelIndex &index, qint32 role) const
{

  if (! index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::SizeHintRole)
  {
    QStyleOptionComboBox comboBox;

    switch(index.column())
    {

      case Color:
      {
        comboBox.currentText = QString("Dark Blue     ");
        const QString header = headerData(Color, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length())
        {
          comboBox.currentText = header;
        }
        break;
      }

    }
    QFontMetrics fontMetrics(data(index, Qt::FontRole) .value<QFont > ());
    comboBox.fontMetrics = fontMetrics;
    QSize size(fontMetrics.width(comboBox.currentText), fontMetrics.height());
    return qApp->style()->sizeFromContents(QStyle::CT_ComboBox, &comboBox, size);

  }
  else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    PerClassItemData* itemData = m_ItemDatas.at(index.row());
    if(NULL == itemData)
    {
      return QVariant();
    }

    QString s;
    //  unsigned int x, y;

    int col = index.column();
    switch(col)
    {
      case PerClassTableModel::Label:
        return QVariant(itemData->getLabel());
      case PerClassTableModel::Gamma:
        return QVariant(itemData->getGamma());
      case PerClassTableModel::MinStdDev:
        return QVariant(itemData->getMinStdDev());
      case PerClassTableModel::Gray:
        return QVariant(itemData->getFinalLabel());
      case PerClassTableModel::Color:
        return QVariant(itemData->getColor());
      default:
        break;
    }

  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant  PerClassTableModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch(section)
    {
      case Label: return QVariant(QString("Class"));
      case Gamma: return QVariant(QString("-Chem. Pntl"));
      case MinStdDev: return QVariant(QString("Min Std Dev"));
      case Color: return QVariant(QString("Color"));
      case Gray: return QVariant(QString("Final Class"));
      default:
        break;
    }
  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PerClassTableModel::rowCount(const QModelIndex &index) const
{
  return m_ItemDatas.count();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PerClassTableModel::columnCount(const QModelIndex &index) const
{
  return index.isValid() ? 0 : m_ColumnCount;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PerClassTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    // std::cout << "SGLogNormalTableModel::setData " << value.toString().toStdString() << std::endl;
    if (!index.isValid()
            || role != Qt::EditRole
            || index.row() < 0
            || index.row() >= m_ItemDatas.count()
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
    case PerClassTableModel::Label:
        m_ItemDatas.at(row)->setLabel(value.toInt(&ok));
        break;
    case PerClassTableModel::Gamma:
        m_ItemDatas.at(row)->setGamma(value.toDouble(&ok));
        break;
    case PerClassTableModel::MinStdDev:
        m_ItemDatas.at(row)->setMinStdDev(value.toDouble(&ok));
        break;
    case PerClassTableModel::Color:
        m_ItemDatas.at(row)->setColor(value.toString());
        break;
    case PerClassTableModel::Gray:
        m_ItemDatas.at(row)->setFinalLabel(value.toInt(&ok));
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
bool PerClassTableModel::insertRows(int row, int count, const QModelIndex& index)
{
    // This is basically disabled at this point
 //   qint32 binNum = 0;
    double gamma = 0.0;
    double minStdDev = 4.5;
    int grayLevel = 0;


    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
    {
        // Create a new PerClassItemData object
        PerClassItemData* d = new PerClassItemData(m_ItemDatas.count(), gamma, minStdDev, QColor::colorNames().at(10), grayLevel, this);
        m_ItemDatas.push_back(d);
        m_RowCount = m_ItemDatas.count();
    }
    endInsertRows();
    emit dataChanged(index, index);
    return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PerClassTableModel::insertItemData(PerClassItemData* data, int row, const QModelIndex& index)
{
  beginInsertRows(QModelIndex(), row, row );
  for (int i = 0; i < 1; ++i)
  {
    // Create a new PerClassItemData object
    m_ItemDatas.push_back(data);
    m_RowCount = m_ItemDatas.count();
  }
  endInsertRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassTableModel::sanityCheckClassValues()
{
  for (int i = 0; i < m_ItemDatas.count(); ++i)
  {
    m_ItemDatas[i]->setLabel(i);
  }
  emit dataChanged(QModelIndex(), QModelIndex());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PerClassTableModel::removeRows(int row, int count, const QModelIndex& index)
{
  if (count < 1)
  {
    return true;
  } // No Rows to remove
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    // Remove the UIA Object Pointer
    m_ItemDatas.removeAt(row + i);
    m_RowCount = m_ItemDatas.count();
  }
  endRemoveRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassTableModel::addItemData(PerClassItemData* uia)
{
  m_ItemDatas.push_back(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassTableModel::deleteItemData(PerClassItemData* uia)
{
  m_ItemDatas.removeAll(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PerClassTableModel::updateItemData(PerClassItemData* uia)
{
  int row = m_ItemDatas.indexOf(uia, 0);
  QModelIndex index0 = createIndex(row, 0);
  QModelIndex index1 = createIndex(row, m_ColumnCount);
  emit dataChanged(index0, index1);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAbstractItemDelegate* PerClassTableModel::getItemDelegate()
{
  return new PerClassItemDelegate;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QList<PerClassItemData*> PerClassTableModel::getItemDatas()
{
  return m_ItemDatas;
}

