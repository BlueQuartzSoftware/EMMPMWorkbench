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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
UserInitAreaTableModel::UserInitAreaTableModel(QObject* parent) :
QAbstractTableModel(parent),
m_column_count(4)
{


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
  QModelIndex index1 = createIndex(row, m_column_count);
  emit dataChanged(index0, index1);
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
  return m_column_count;
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

  if (role == Qt::TextAlignmentRole) {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole)
  {
    UserInitArea* uia = m_UserInitAreas.at(index.row());
    if (NULL == uia)
    {
      return QVariant();
    }
    QPoint p = uia->pos().toPoint();
    QRect b = uia->boundingRect().toAlignedRect();


    int col = index.column();
    if (col == 2) // TOP LEFT CORNER
    {
      QString s = QString::number(b.x()+p.x());
      s.append(", ");
      s.append(QString::number(b.y()+p.y()));
      return QVariant(s);
    }
    else if (col == 3) // BOTTOM RIGHT CORNER
    {
      QString s = QString::number(b.x()+p.x() + b.width());
      s.append(", ");
      s.append(QString::number(b.y()+p.y() + b.height()));
      return QVariant(s);
    }
    else if (col == 1)
    {
      return QVariant(QString::number(uia->getEmMpmGrayLevel()));
    }
    else if (col == 0)
    {
      return QVariant(QString::number(uia->getEmMpmClass()));
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
    if (section == 0) return QVariant(QString("Class"));
    else if (section == 1) return QVariant(QString("Gray Value"));
    else if (section == 2) return QVariant(QString("Upper Left"));
    else if (section == 3) return QVariant(QString("Lower Right"));

  }
  return QVariant();
}
