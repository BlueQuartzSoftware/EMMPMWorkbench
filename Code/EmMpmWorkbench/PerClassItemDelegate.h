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
#ifndef _PER_CLASS_ITEM_DELEGATE_H_
#define _PER_CLASS_ITEM_DELEGATE_H_

#include <iostream>

#include <QtCore/QModelIndex>
#include <QtGui/QToolButton>
#include <QtGui/QColorDialog>
#include <QtGui/QComboBox>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionViewItemV4>
#include <QtGui/QLineEdit>
#include <QtGui/QIntValidator>
#include <QtGui/QDoubleValidator>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QPushButton>

#include "QtSupport/ColorComboPicker.h"

#include "PerClassTableModel.h"

namespace Detail {
    const static int Alpha = 155;
}

/**
 * @class PerClassItemDelegate PerClassItemDelegate.h AIM/StatsGenerator/PerClassItemDelegate.h
 * @brief This class creates the appropriate Editor Widget for the Tables
 * @author Michael A. Jackson for BlueQuartz Software
 * @date April 05, 2012
 * @version 1.0
 */
class PerClassItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

  public:
    explicit PerClassItemDelegate(QObject *parent = 0) :
      QStyledItemDelegate(parent)
    {
        m_ColorBtn = new QPushButton("");
        m_ColorBtn->setText("");
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
     qint32 col = index.column();
     if (col == PerClassTableModel::Color )
     {
        QString colorName = index.model()->data(index).toString();
        if (QColor::isValidColor(colorName) == true)
        {
            QString cssColor = "border: 1px solid #101010; background-color: ";
            cssColor.append(colorName);
            m_ColorBtn->setStyleSheet(cssColor);
            m_ColorBtn->setGeometry(option.rect);

            if (option.state == QStyle::State_Selected)
                         painter->fillRect(option.rect, option.palette.highlight());
            QPixmap map = QPixmap::grabWidget(m_ColorBtn);
            painter->drawPixmap(option.rect.x(),option.rect.y(),map);
        }
     }
     else
     {
        QStyledItemDelegate::paint(painter, option, index);
      }
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QLineEdit* editor = NULL;
        QDoubleValidator* dValidator = NULL;
        QIntValidator* iValidator = NULL;

        qint32 col = index.column();

        if(col == PerClassTableModel::Color)
        {
            QPushButton* colorBtn = new QPushButton(parent);
            QString colorName = index.model()->data(index).toString();
            if (QColor::isValidColor(colorName) == true)
            {
                QColor color(colorName);

                QColor result = QColorDialog::getColor(color);
                if(result.isValid())
                {
                    QString cssColor = "border: 1px solid #101010; background-color: ";
                    cssColor.append(result.name());
                    colorBtn->setStyleSheet(cssColor);

                    QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
                    model->setData(index, result.name());
                }
            }
            else
            {
                QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
                model->setData(index, "#000000");
            }
            return colorBtn;
        }

        switch(col)
        {
        case PerClassTableModel::Gamma:
            editor = new QLineEdit(parent);
            editor->setFrame(false);
            dValidator = new QDoubleValidator(editor);
            dValidator->setDecimals(6);
            editor->setValidator(dValidator);
            return editor;
        case PerClassTableModel::MinStdDev:
            editor = new QLineEdit(parent);
            editor->setFrame(false);
            dValidator = new QDoubleValidator(editor);
            dValidator->setDecimals(6);
            editor->setValidator(dValidator);
            return editor;
        case PerClassTableModel::MergeLabel:
            editor = new QLineEdit(parent);
            editor->setFrame(false);
            iValidator = new QIntValidator(editor);
            editor->setValidator(iValidator);
            return editor;
        default:
            break;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
      qint32 col = index.column();

      if (col == PerClassTableModel::Gamma ||
          col == PerClassTableModel::MergeLabel ||
          col == PerClassTableModel::MinStdDev)
      {
        QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
        Q_ASSERT(lineEdit);
        lineEdit->setText(index.model()->data(index).toString());
      }
      else QStyledItemDelegate::setEditorData(editor, index);
    }


    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
      qint32 col = index.column();

      if (col == PerClassTableModel::Gamma)
      {
        QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
        Q_ASSERT(lineEdit);
        bool ok = false;
        double v = lineEdit->text().toDouble(&ok);
        model->setData(index, v);
      }
      else if (col == PerClassTableModel::MinStdDev)
      {
        QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
        Q_ASSERT(lineEdit);
        bool ok = false;
        double v = lineEdit->text().toDouble(&ok);
        model->setData(index, v);
      }
      else if (col == PerClassTableModel::MergeLabel)
      {
          QLineEdit* lineEdit = qobject_cast<QLineEdit* > (editor);
          Q_ASSERT(lineEdit);
          bool ok = false;
          int v = lineEdit->text().toInt(&ok);
          model->setData(index, v);
      }
      else if (col == PerClassTableModel::Color)
      {
         // Do nothing because the data has already been committed.
      }
      else QStyledItemDelegate::setModelData(editor, model, index);
    }

  private:
    QPushButton* m_ColorBtn;


};

#endif /* _PER_CLASS_ITEM_DELEGATE_H_ */
