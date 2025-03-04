/*
 * Copyright (C) 2005-2020 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mvdStatusBarWidget.h"
#include "ui_mvdStatusBarWidget.h"


/*****************************************************************************/
/* INCLUDE SECTION                                                           */

//
// Qt includes (sorted by alphabetic order)
//// Must be included before system/custom includes.

//
// System includes (sorted by alphabetic order)
// #include <cassert>

//
// ITK includes (sorted by alphabetic order)

//
// OTB includes (sorted by alphabetic order)

//
// Monteverdi includes (sorted by alphabetic order)

namespace mvd
{
/*
  TRANSLATOR mvd::StatusBarWidget

  Necessary for lupdate to be aware of C++ namespaces.

  Context comment for translator.
*/


/*****************************************************************************/
/* CONSTANTS                                                                 */


/*****************************************************************************/
/* STATIC IMPLEMENTATION SECTION                                             */


/*****************************************************************************/
/* CLASS IMPLEMENTATION SECTION                                              */
/*****************************************************************************/
StatusBarWidget::StatusBarWidget(QWidget* p, Qt::WindowFlags flags) : QWidget(p, flags), m_UI(new mvd::Ui::StatusBarWidget())
{
  m_UI->setupUi(this);
  // mantis-1385 hide scaleLineEdit
  m_UI->label_4->hide();
  m_UI->scaleLineEdit->hide();
}

/*****************************************************************************/
StatusBarWidget::~StatusBarWidget()
{
  delete m_UI;
  m_UI = NULL;
}

/*****************************************************************************/
QString StatusBarWidget::ZoomLevel(double scale)
{
  if (scale > 1.0)
    return QString("%1:1").arg(scale);

  else if (scale < 1.0)
    return QString("1:%1").arg(1.0 / scale);

  return "1:1";
}

/*****************************************************************************/
void StatusBarWidget::SetGLSLEnabled(bool enabled)
{
  m_UI->renderModelLabel->setText(enabled ? tr("GLSL") : tr("OpenGL"));
}

/*****************************************************************************/
/* SLOTS                                                                     */
/*****************************************************************************/
void StatusBarWidget::SetPixelIndex(const IndexType& pixel, bool isInsideRegion)
{
  assert(m_UI != NULL);
  assert(m_UI->pixelIndexLineEdit != NULL);

  m_UI->pixelIndexLineEdit->setText(isInsideRegion ? QString("%1, %2").arg(pixel[0]).arg(pixel[1]) : QString());
}

/*****************************************************************************/
void StatusBarWidget::SetScale(double sx, double)
{
  // qDebug() << this << "::SetScale(" << sx << "," << sy << ")";

  /*
  if( sx!=sy )
    zoomLevel.append( "/" + StatusBarWidget::ZoomLevel( sy ) );
  */

  m_UI->scaleLineEdit->setText(StatusBarWidget::ZoomLevel(sx));
}

/*****************************************************************************/
/* PRIVATE SLOTS                                                             */
/*****************************************************************************/
/*
void
StatusBarWidget
::SetPixelIndexText( const QString& text )
{
  m_UI->pixelIndexLineEdit->setText( text );
}
*/

/*****************************************************************************/
void StatusBarWidget::SetText(const QString& text)
{
  assert(m_UI != NULL);
  assert(m_UI->pixelIndexLineEdit != NULL);

  m_UI->pixelRadiometryLabel->setText(text);
}

/*****************************************************************************/
void StatusBarWidget::on_pixelIndexLineEdit_returnPressed()
{
  //
  // Cancel if pixel-index coordinates text is empty.
  if (m_UI->pixelIndexLineEdit->text().isEmpty())
    return;

  //
  // Split coordinates.
  QStringList coordinates(m_UI->pixelIndexLineEdit->text().split(','));

  //
  // Check split coordinates format.
  assert(coordinates.size() == 1 || coordinates.size() == 2);

  if (coordinates.size() != 1 && coordinates.size() != 2)
    return;

  //
  // Construct resulting pixel-index.
  IndexType index;

  index.Fill(0);

  //
  // Convert first pixel-index coordinate.
  bool isOk = true;
  index[0]  = coordinates.front().toUInt(&isOk);

  // assert( isOk );

  if (!isOk)
    return;

  //
  // Convert second pixel-index coordinate.
  if (coordinates.size() > 1)
  {
    index[1] = coordinates.back().toUInt(&isOk);

    // assert( isOk );

    if (!isOk)
      return;
  }

  //
  // If both pixel-index coordinates have correctly been converted,
  // Q_EMIT pixel-index changed signal.
  Q_EMIT PixelIndexChanged(index);
}

/*****************************************************************************/
void StatusBarWidget::on_scaleLineEdit_returnPressed()
{
  ChangeScale();
}

void StatusBarWidget::on_scaleLineEdit_editingFinished()
{
  if (m_UI->scaleLineEdit->isModified())
  {
    ChangeScale();
  }
}

void StatusBarWidget::ChangeScale()
{
  //
  // Cancel if scale text is empty.
  if (m_UI->scaleLineEdit->text().isEmpty())
    return;

  //
  // Split scale text.
  QStringList scale(m_UI->scaleLineEdit->text().split(':'));

  //
  // Check scale text format.
  // assert( scale.size()==1 || scale.size()==2 );

  if (scale.size() != 1 && scale.size() != 2)
    return;

  /*
  if( scale.size()!=1 && scale.size()!=2 )
    {
    throw std::invalid_argument(
      ToStdString(
        tr( "Invalid argument: '%1' should be scale of the form <numerator>[:<denominator>] with numerator and denominator being real numbers." )
      )
    );
    }
  */

  //
  // Convert scale numerator.
  bool   isOk      = true;
  double numerator = scale.front().toDouble(&isOk);

  // assert( isOk );
  // assert( numerator!=0.0 );

  if (!isOk || numerator == 0.0)
    return;

  /*
  if( !isOk )
    {
    throw std::invalid_argument(
      ToStdString(
        tr( "Invalid argument: '%1' should be scale of the form <numerator>[:<denominator>] with numerator and denominator being real numbers." )
      )
    );
    }
  */

  //
  // Convert scale denominator.
  double denominator = 1.0;

  if (scale.size() > 1)
  {
    denominator = scale.back().toDouble(&isOk);

    // assert( isOk );

    if (!isOk)
      return;

    /*
    if( !isOk )
      {
      throw std::invalid_argument(
        ToStdString(
          tr( "Invalid argument: '%1' should be scale of the form <numerator>[:<denominator>] with numerator and denominator being real numbers." )
        )
      );
      }
    */
  }

  //
  // Emit scale changed.
  Q_EMIT ScaleChanged(numerator / denominator);
}

} // end namespace 'mvd'
