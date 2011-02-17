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
#include "UserInitAreaTableModel.h"

#include <QApplication>
#include <QtGui/QAbstractItemDelegate>

#include "UserInitAreaItemDelegate.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaTableModel::UserInitAreaTableModel(QObject* parent) :
QAbstractTableModel(parent),
m_RowCount(0)
{
  m_ColumnCount = ColumnCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaTableModel::~UserInitAreaTableModel()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags UserInitAreaTableModel::flags(const QModelIndex &index) const
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
    if (col == Class || col == GrayValue || col == Gamma)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

  }
  return theFlags;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant UserInitAreaTableModel::data(const QModelIndex &index, qint32 role) const
{

  if (! index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::SizeHintRole)
  {
#if 0
    QStyleOptionComboBox comboBox;

    switch(index.column())
    {
      case Class:
      {
        comboBox.currentText = QString("101");
        const QString header = headerData(Class, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case Average:
      {
        comboBox.currentText = QString("10001");
        const QString header = headerData(Class, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case StdDev:
      {
        comboBox.currentText = QString("10001");
        const QString header = headerData(Class, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case LineColor:
      {
        comboBox.currentText = QString("Dark Blue      ");
        const QString header = headerData(Class, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length())
        {
          comboBox.currentText = header;
        }
        break;
      }
      default:
      Q_ASSERT(false);
    }
    QFontMetrics fontMetrics(data(index, Qt::FontRole) .value<QFont > ());
    comboBox.fontMetrics = fontMetrics;
    QSize size(fontMetrics.width(comboBox.currentText), fontMetrics.height());
    return qApp->style()->sizeFromContents(QStyle::CT_ComboBox, &comboBox, size);
#endif

  }
  else if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    UserInitArea* uia = m_UserInitAreas.at(index.row());
    if (NULL == uia)
    {
      return QVariant();
    }
    QPoint p = uia->pos().toPoint();
    QRect b = uia->boundingRect().toAlignedRect();
    QString s;

    int col = index.column();
    switch(col)
    {
      case UserInitAreaTableModel::Class:
        return QVariant(uia->getEmMpmClass());
      case UserInitAreaTableModel::GrayValue:
        return QVariant(uia->getEmMpmGrayLevel());
      case UserInitAreaTableModel::UpperLeft:
        s = QString::number(b.x() + p.x());
        s.append(", ");
        s.append(QString::number(b.y() + p.y()));
        uia->setUpperLeft(b.x() + p.x(), b.y() + p.y());
        return QVariant(s);
      case UserInitAreaTableModel::LowerRight:
        s = QString::number(b.x() + p.x() + b.width());
        s.append(", ");
        s.append(QString::number(b.y() + p.y() + b.height()));
        uia->setLowerRight(b.x() + p.x() + b.width(), b.y() + p.y() + b.height());
        return QVariant(s);
      case UserInitAreaTableModel::Mu:
        return QVariant(uia->getMu());
      case UserInitAreaTableModel::Sigma:
        return QVariant(uia->getSigma());
      case UserInitAreaTableModel::Gamma:
        return QVariant(uia->getGamma());
      default:
        break;
    }

  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant  UserInitAreaTableModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch(section)
    {
      case Class: return QVariant(QString("Class"));
      case GrayValue: return QVariant(QString("Gray Value"));
      case UpperLeft: return QVariant(QString("Upper Left"));
      case LowerRight: return QVariant(QString("Lower Right"));
      case Mu: return QVariant(QString("Mu"));
      case Sigma: return QVariant(QString("Sigma"));
      case Gamma: return QVariant(QString("Gamma"));
      default:
        break;
    }
  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int UserInitAreaTableModel::rowCount(const QModelIndex &index) const
{
  return m_UserInitAreas.count();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int UserInitAreaTableModel::columnCount(const QModelIndex &index) const
{
  return index.isValid() ? 0 : m_ColumnCount;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool UserInitAreaTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  // std::cout << "SGLogNormalTableModel::setData " << value.toString().toStdString() << std::endl;
  if (!index.isValid()
      || role != Qt::EditRole
      || index.row() < 0
      || index.row() >= m_UserInitAreas.count()
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
    case UserInitAreaTableModel::Class:
      m_UserInitAreas.at(row)->setEmMpmClass(value.toInt(&ok));
      break;
    case UserInitAreaTableModel::GrayValue:
      m_UserInitAreas.at(row)->setEmMpmGrayLevel(value.toInt(&ok));
      break;
    case UserInitAreaTableModel::UpperLeft:
      break;
    case UserInitAreaTableModel::LowerRight:
      break;
    case UserInitAreaTableModel::Mu:
      break;
    case UserInitAreaTableModel::Sigma:
      break;
    case UserInitAreaTableModel::Gamma:
      m_UserInitAreas.at(row)->setGamma(value.toDouble(&ok));
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
bool UserInitAreaTableModel::insertRows(int row, int count, const QModelIndex& index)
{
  if (true) return false;

  // This is basically disabled at this point
  qint32 binNum = 0;
  double avg = 1.0;
  double stdDev = 0.25;
  QString c("blue");

  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    // Create a new UserInitArea object
    m_RowCount = m_UserInitAreas.count();
  }
  endInsertRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool UserInitAreaTableModel::removeRows(int row, int count, const QModelIndex& index)
{

  // This is basically disabled for now.
  if (true) return false;


  if (count < 1)
  {
    return true;
  } // No Rows to remove
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    // Remove the UIA Object Pointer
    m_RowCount = m_UserInitAreas.count();
  }
  endRemoveRows();
  emit dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaTableModel::addUserInitArea(UserInitArea* uia)
{
  m_UserInitAreas.push_back(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaTableModel::deleteUserInitArea(UserInitArea* uia)
{
  m_UserInitAreas.removeAll(uia);
  emit layoutChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void UserInitAreaTableModel::updateUserInitArea(UserInitArea* uia)
{
  int row = m_UserInitAreas.indexOf(uia, 0);
  QModelIndex index0 = createIndex(row, 0);
  QModelIndex index1 = createIndex(row, m_ColumnCount);
  emit dataChanged(index0, index1);
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAbstractItemDelegate* UserInitAreaTableModel::getItemDelegate()
{
  return new UserInitAreaItemDelegate;
}

