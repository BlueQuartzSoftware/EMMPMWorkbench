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

#ifndef USERINITAREATABLEMODEL_H_
#define USERINITAREATABLEMODEL_H_

#include <QtCore/QVector>
#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtCore/QAbstractTableModel>

#include "UserInitArea.h"

class QAbstractItemDelegate;


class UserInitAreaTableModel : public QAbstractTableModel
{
    Q_OBJECT;
  public:
    enum ColumnIndexes
    {
      Class = 0,
      GrayValue,
      UpperLeft,
      LowerRight,
      Mu,
      Sigma,
      Gamma,
      ColumnCount
    };

    UserInitAreaTableModel(QObject* parent = 0);
    virtual ~UserInitAreaTableModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant  data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role=Qt::DisplayRole) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    int  columnCount ( const QModelIndex & parent = QModelIndex() ) const;

    bool setData(const QModelIndex &index, const QVariant &value,
        int role=Qt::EditRole);

    /**
     *
     * @param col
     * @param orientation
     * @param data
     * @param role
     * @return
     */
    bool setHeaderData(int col, Qt::Orientation orientation, const QVariant& data, int role=Qt::EditRole)
    { return false;}

    bool insertRows(int row, int count, const QModelIndex &parent=QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex());

    QAbstractItemDelegate* getItemDelegate();


    const QList<UserInitArea*>& getUserInitAreas() { return m_UserInitAreas; }

  public slots:

    void addUserInitArea(UserInitArea* uia);
    void deleteUserInitArea(UserInitArea* uia);
    void updateUserInitArea(UserInitArea* uia);


  protected:


  private:
    int m_ColumnCount;
    int m_RowCount;
    QList<UserInitArea*> m_UserInitAreas;


    UserInitAreaTableModel(const UserInitAreaTableModel&); // Copy Constructor Not Implemented
    void operator=(const UserInitAreaTableModel&); // Operator '=' Not Implemented

};

#endif /* USERINITAREATABLEMODEL_H_ */
