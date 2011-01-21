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

#include "SGPowerLawTableModel.h"
#include <iostream>

#include <QApplication>
#include <QtGui/QStyleOptionComboBox>
#include <QtGui/QAbstractItemDelegate>

#include "SGPowerLawItemDelegate.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SGPowerLawTableModel::SGPowerLawTableModel(QObject* parent) :
  SGAbstractTableModel(parent), m_RowCount(0)
{
  m_ColumnCount = ColumnCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SGPowerLawTableModel::~SGPowerLawTableModel()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags SGPowerLawTableModel::flags(const QModelIndex &index) const
{
  //  std::cout << "SGPowerLawTableModel::flags" << std::endl;
  if (!index.isValid())
  {
    return Qt::NoItemFlags;
  }
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid())
  {
    theFlags |= Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    int col = index.column();
    if (col == BinNumber)
    {
      theFlags = Qt::ItemIsEnabled;
    }
    else if (col == Alpha)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    else if (col == K)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    else if (col == Beta)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    else if (col == LineColor)
    {
      theFlags = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
  }
  return theFlags;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant SGPowerLawTableModel::data(const QModelIndex &index, qint32 role) const
{

  if (!index.isValid())
  {
    return QVariant();
  }

  if (role == Qt::SizeHintRole)
  {
    QStyleOptionComboBox comboBox;

    switch(index.column())
    {
      case BinNumber:
      {
        comboBox.currentText = QString("101");
        const QString header = headerData(BinNumber, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case Alpha:
      {
        comboBox.currentText = QString("00011");
        const QString header = headerData(BinNumber, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case K:
      {
        comboBox.currentText = QString("10001");
        const QString header = headerData(BinNumber, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case Beta:
      {
        comboBox.currentText = QString("10001");
        const QString header = headerData(BinNumber, Qt::Horizontal, Qt::DisplayRole).toString();
        if (header.length() > comboBox.currentText.length()) comboBox.currentText = header;
        break;
      }
      case LineColor:
      {
        comboBox.currentText = QString("Dark Blue     ");
        const QString header = headerData(BinNumber, Qt::Horizontal, Qt::DisplayRole).toString();
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
  }
  else if (role == Qt::TextAlignmentRole)
  {
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  else if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    int col = index.column();
    if (col == BinNumber)
    {
      return QVariant(m_BinNumbers[index.row()]);
    }
    else if (col == Alpha)
    {
      return QVariant(m_Alpha[index.row()]);
    }
    else if (col == K)
    {
      return QVariant(m_K[index.row()]);
    }
    else if (col == Beta)
    {
      return QVariant(m_Beta[index.row()]);
    }
    else if (col == LineColor)
    {
      return QVariant(m_Colors[index.row()]);
    }
  }

  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant SGPowerLawTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch(section)
    {
      case BinNumber:
        return QVariant(QString("Bin"));
        break;
      case Alpha:
        return QVariant(QString("Alpha"));
        break;
      case K:
        return QVariant(QString("K"));
        break;
      case Beta:
        return QVariant(QString("Beta"));
        break;
      case LineColor:
        return QVariant(QString("Color"));
        break;
      default:
        break;
    }

  }
  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SGPowerLawTableModel::rowCount(const QModelIndex &index) const
{
  return index.isValid() ? 0 : m_RowCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SGPowerLawTableModel::columnCount(const QModelIndex &index) const
{
  return index.isValid() ? 0 : m_ColumnCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SGPowerLawTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  // std::cout << "SGPowerLawTableModel::setData " << value.toString().toStdString() << std::endl;
  if (!index.isValid() || role != Qt::EditRole || index.row() < 0 || index.row() >= m_BinNumbers.count() || index.column() < 0 || index.column()
      >= m_ColumnCount)
  {
    return false;
  }
  bool ok;
  qint32 row = index.row();
  qint32 col = index.column();
  switch(col)
  {
    case BinNumber:
      m_BinNumbers[row] = value.toDouble(&ok);
      break;
    case Alpha:
      m_Alpha[row] = value.toDouble(&ok);
      break;
    case K:
      m_K[row] = value.toDouble(&ok);
      break;
    case Beta:
      m_Beta[row] = value.toDouble(&ok);
      break;
    case LineColor:
      m_Colors[row] = value.toString();
      break;
    default:
      Q_ASSERT(false);

  }

  emit
  dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SGPowerLawTableModel::insertRows(int row, int count, const QModelIndex& index)
{
  qint32 binNum = 0;
  double alpha = 15.0;
  double k = 2.0;
  double beta = 1.0;
  QString c("blue");

  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    m_BinNumbers.append(binNum);
    m_Alpha.append(alpha);
    m_K.append(k);
    m_Beta.append(beta);
    m_Colors.append(c);
    m_RowCount = m_BinNumbers.count();
  }
  endInsertRows();
  emit
  dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SGPowerLawTableModel::removeRows(int row, int count, const QModelIndex& index)
{
  if (count < 1)
  {
    return true;
  } // No Rows to remove
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
  {
    m_BinNumbers.remove(row);
    m_Alpha.remove(row);
    m_K.remove(row);
    m_Beta.remove(row);
    m_Colors.remove(row);
    m_RowCount = m_BinNumbers.count();
  }
  endRemoveRows();
  emit
  dataChanged(index, index);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<double > SGPowerLawTableModel::getData(int col)
{

  switch(col)
  {
    case Alpha:
      return m_Alpha;
    case K:
      return m_K;
    case Beta:
      return m_Beta;
    default:
      Q_ASSERT(false);
  }
  return QVector<double > ();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double SGPowerLawTableModel::getDataValue(int col, int row)
{
  switch(col)
  {
    case Alpha:
      return m_Alpha[row];
    case K:
      return m_K[row];
    case Beta:
      return m_Beta[row];
    default:
      Q_ASSERT(false);
  }
  return 0.0f;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SGPowerLawTableModel::setBinNumbers(QVector<double> binNumbers)
{

  qint32 count = binNumbers.count();

  // Remove all the current rows in the table model
  removeRows(0, rowCount());

  //TODO: SET SOME SESIBLE Defaults
  double alpha = 14.0;
  double k = 1.0;
  double beta = 1.0;

  double alphaStep = 2.0 / (double)(count);
  double kStep = 0.25 / (double)(count);
  double betaStep = 20.0 / (double)(count);

  QStringList colorNames = QColor::colorNames();
  qint32 colorOffset = 21;

  // Now Populate the table data with the data that was passed in
  for (qint32 i = 0; i < count; ++i)
  {
    if (!insertRow(rowCount())) return;

    QModelIndex binNumberIndex = index(rowCount() - 1, SGAbstractTableModel::BinNumber);
    setData(binNumberIndex, QVariant(binNumbers[i]), Qt::EditRole);

    QModelIndex alphaIndex = index(rowCount() - 1, Alpha);
    setData(alphaIndex, QVariant(alpha), Qt::EditRole);

    QModelIndex kIndex = index(rowCount() - 1, K);
    setData(kIndex, QVariant(k), Qt::EditRole);

    QModelIndex betaIndex = index(rowCount() - 1, Beta);
    setData(betaIndex, QVariant(beta), Qt::EditRole);

    alpha += alphaStep;
    k += kStep;
    beta += betaStep;

    QModelIndex colorIndex = index(rowCount() - 1, LineColor);
    setData(colorIndex, QVariant(colorNames[colorOffset++]), Qt::EditRole);
    if (colorOffset == colorNames.count())
    {
      colorOffset = colorNames.count() - 1;
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAbstractItemDelegate* SGPowerLawTableModel::getItemDelegate()
{
  return new SGPowerLawItemDelegate;
}

