/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "customuistyle.h"
#include <QToolBar>
#include <QApplication>

QMap<QStyle::PixelMetric, int> CustomUiStyle::pixel_metrics;

CustomUiStyle::CustomUiStyle() : QProxyStyle()
{

}

CustomUiStyle::CustomUiStyle(const QString &key): QProxyStyle(key)
{

}

void CustomUiStyle::setPixelMetricValue(QStyle::PixelMetric metric, int value)
{
	pixel_metrics[metric] = value;
}

CustomUiStyle::~CustomUiStyle()
{

}

int CustomUiStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
	if(pixel_metrics.contains(metric))
		return pixel_metrics[metric];

	// Use the default pixel metric attribute value if there's no custom value defined
	return QProxyStyle::pixelMetric(metric, option, widget);
}

void CustomUiStyle::drawItemPixmap(QPainter *painter, const QRect &rect, 
																	 int alignment, const QPixmap &pixmap) const
{
	qreal curr_opacity = painter->opacity();
	
	// Check if the painter opacity is low (indicating disabled widget)
	if (curr_opacity < 0.9) 
	{
		painter->save();
		QPixmap grayed_pixmap = createGrayMaskedPixmap(pixmap);
		QProxyStyle::drawItemPixmap(painter, rect, alignment, grayed_pixmap);
		painter->restore();
		return;
	}
	
	QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void CustomUiStyle::drawControl(ControlElement element, const QStyleOption *option,
																QPainter *painter, const QWidget *widget) const
{
	// Only apply special handling to specific toolbar button labels
	if(element == CE_ToolButtonLabel && option && !(option->state & State_Enabled)) 
	{
		const QStyleOptionToolButton *tb_option = 
					qstyleoption_cast<const QStyleOptionToolButton *>(option);
			
		// Check if this is a QToolButton from a QToolBar (has QAction)
		if(tb_option && widget && widget->parent()) 
		{
			QToolBar *toolbar = qobject_cast<QToolBar*>(widget->parent());

			if(toolbar) 
			{
				// This is a QToolButton in a QToolBar, use default behavior
				// The icon will be handled by generatedIconPixmap
				QProxyStyle::drawControl(element, option, painter, widget);
				return;
			}
		}
	}

	// For all other elements, use default behavior without opacity changes
	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
																	QPainter *painter, const QWidget *widget) const
{
	// Frame elements that need to be customized
  if((element == PE_Frame || element == PE_FrameLineEdit || 
		  element == PE_FrameGroupBox || element == PE_FrameTabWidget || 
      element == PE_FrameWindow) && 
     option && painter && widget)
  {
    bool customize = false, has_round_corners = false;
    
    // Lista de classes base para verificar
    static const QStringList target_classes = {
      "QLineEdit",
      "QPlainTextEdit", 
      "QTreeWidget",
      "QTreeView",
      "NumberedTextEditor"
    };
    
    for(auto &class_name : target_classes)
    {
			// We customize widgets that inherit from the target classes
      if(widget->inherits(class_name.toStdString().c_str()))
      {
        customize = true;
				has_round_corners = (class_name == "QLineEdit" || 
														 class_name == "QPlainTextEdit"); 
        break;
      }
    }
    
		// If the widget itself is not a target class, check its parent hierarchy
    if(!customize)
    {
      const QWidget *parent = widget->parentWidget();

      while(parent && !customize)
      {
        for(auto &class_name : target_classes)
        {
          if(parent->inherits(class_name.toStdString().c_str()))
          {
            customize = true;
						has_round_corners = (class_name == "QLineEdit" || 
																 class_name == "QPlainTextEdit"); 
            break;
          }
        }
        parent = parent->parentWidget();
      }
    }
    
		// If it is one of the target classes, customize the border
    if(customize)
    {
      painter->save();
      
      // Use the border color based on QPalette color but a bit ligther
      QColor border_color = qApp->palette().color(QPalette::Dark).lighter(150);
      QPen border_pen(border_color);
      border_pen.setWidth(1);
      painter->setPen(border_pen);
      
			if(has_round_corners)
			{
				painter->setRenderHints(QPainter::Antialiasing, true);     
				painter->drawRoundedRect(option->rect, 4, 4);    
			}
			else
				painter->drawRect(option->rect.adjusted(0, 0, -1, -1));      
      
			painter->restore();
      return;
    }
  }

	// Use default behavior without opacity changes for primitives
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap CustomUiStyle::generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap &pixmap,
																					 const QStyleOption *option) const
{
	// Generate grayscale version for disabled icons
	if(icon_mode == QIcon::Disabled) 
		return createGrayMaskedPixmap(pixmap);
			
	return QProxyStyle::generatedIconPixmap(icon_mode, pixmap, option);
}

QPixmap CustomUiStyle::createGrayMaskedPixmap(const QPixmap &original) const
{
	if(original.isNull()) 
		return original;
		
	// Convert to QImage for desaturation and color blending
	QImage image = original.toImage().convertToFormat(QImage::Format_ARGB32);
	QRgb *line = nullptr, pixel;
	QColor mask_color = qApp->palette().color(QPalette::Disabled, QPalette::Window);
	int gray = 0, final_r = 0, 
			final_g = 0, final_b = 0, alpha = 0,
			mask_r = mask_color.red(),
			mask_g = mask_color.green(),
			mask_b = mask_color.blue();

	// Apply desaturation (grayscale conversion) and blend with color
	for(int y = 0; y < image.height(); ++y) 
	{
		// Get pointer to the start of the scan line
		line = reinterpret_cast<QRgb *>(image.scanLine(y));

		for(int x = 0; x < image.width(); ++x) 
		{
			pixel = line[x];
			alpha = qAlpha(pixel);
					
			// Appky only to non-transparent pixels
			if(alpha > 0) 
			{
				// Convert to grayscale using standard luminance formula
				gray = qGray(pixel);  // qGray uses formula: (11*r + 16*g + 5*b)/32
							
				// Blend grayscale with the target mask color 
				final_r = (gray * (1.0 - BlendFactor)) + (mask_r * BlendFactor);
				final_g = (gray * (1.0 - BlendFactor)) + (mask_g * BlendFactor);
				final_b = (gray * (1.0 - BlendFactor)) + (mask_b * BlendFactor);

				// Ensure values are in valid range [0-255]
				final_r = qBound(0, final_r, 255);
				final_g = qBound(0, final_g, 255);
				final_b = qBound(0, final_b, 255);
							
				line[x] = qRgba(final_r, final_g, final_b, alpha);
			}
		}
	}
	
	return QPixmap::fromImage(image);
}
