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
#include <QToolButton>
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
	// Custom flat style for QToolButton with rounded corners - NO shadows
	if((element == PE_PanelButtonTool || element == PE_FrameButtonTool || 
		  element == PE_PanelButtonCommand || element == PE_PanelButtonBevel) && widget)
	{
		const QToolButton *tool_button = qobject_cast<const QToolButton*>(widget);
		
		if(tool_button && option && painter)
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			
			// Disable any shadow effects completely
			painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
			
			// Get base colors from palette
			QColor dark_color = option->palette.color(QPalette::Dark);
			QColor button_color = option->palette.color(QPalette::Button);
			QColor border_color = dark_color;
			QColor bg_color;
			
			// Adjust background color based on button state
			if(!(option->state & State_Enabled))
			{
				// Disabled: use QPalette::Button color
				bg_color = button_color;
			}
			else if(option->state & (State_Sunken | State_On))
			{
				// Pressed or checked: 120 points lighter than QPalette::Button
				bg_color = button_color.lighter(120);
			}
			else if(option->state & State_MouseOver)
			{
				// Hover: 150 points lighter than QPalette::Dark
				bg_color = dark_color.lighter(150);
			}
			else
			{
				// Normal state: 120 points lighter than QPalette::Dark
				bg_color = dark_color.lighter(120);
			}
			
			// Draw completely flat rounded background (no shadows)
			painter->setBrush(bg_color);
			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(option->rect, 4, 4);
			
			// Draw flat rounded border (no 3D effects)
			QPen border_pen(border_color);
			border_pen.setWidth(1);
			border_pen.setStyle(Qt::SolidLine);
			painter->setPen(border_pen);
			painter->setBrush(Qt::NoBrush);
			painter->drawRoundedRect(option->rect.adjusted(0, 0, -1, -1), 4, 4);
			
			painter->restore();
			return;
		}
	}
	
	// Frame elements that need to be customized
  if((element == PE_Frame || element == PE_FrameLineEdit || 
		  element == PE_FrameGroupBox || element == PE_FrameTabWidget || 
      element == PE_FrameWindow) && 
     option && painter && widget)
  {
    bool customize = false, has_round_corners = false;
    
	   static const QStringList target_classes = {
      "QLineEdit",
      "QPlainTextEdit", 
      "QTreeWidget",
      "QTreeView",
			"QTableView",
			"QTableWidget",
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
      QColor border_color = qApp->palette().color(QPalette::Dark).lighter(130);
      QPen border_pen(border_color);
      border_pen.setWidth(1);
      painter->setPen(border_pen);
      
			if(has_round_corners)
			{
				painter->setRenderHints(QPainter::Antialiasing, true);     
				painter->drawRoundedRect(option->rect, 3, 3);    
			}
			else
				painter->drawRect(option->rect.adjusted(1, 1, -1, -1));      
      
			painter->restore();
      return;
    }
  }

	// Use default behavior without opacity changes for primitives
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
																			 QPainter *painter, const QWidget *widget) const
{
	// Handle QToolButton complex control to ensure completely flat appearance
	if(control == CC_ToolButton && widget)
	{
		const QToolButton *tool_button = qobject_cast<const QToolButton*>(widget);
		const QStyleOptionToolButton *tb_option = 
					qstyleoption_cast<const QStyleOptionToolButton *>(option);
		
		if(tool_button && tb_option && painter)
		{
			// Draw button background without any 3D effects or shadows
			if(tb_option->subControls & SC_ToolButton)
			{
				QStyleOptionToolButton button_opt = *tb_option;
				button_opt.subControls = SC_ToolButton;
				
				// Remove ALL 3D state flags to ensure flat appearance
				button_opt.state &= ~(State_Raised | State_Sunken);
				button_opt.features &= ~(QStyleOptionToolButton::HasMenu);
				
				// Use our custom flat primitive drawing
				drawPrimitive(PE_PanelButtonTool, &button_opt, painter, widget);
			}
			
			// Draw button content (icon/text) without any shadow effects
			if(tb_option->subControls & SC_ToolButton)
			{
				QStyleOptionToolButton content_opt = *tb_option;
				content_opt.subControls = SC_ToolButton;
				
				// Remove shadow-related state flags
				content_opt.state &= ~(State_Raised | State_Sunken);
				
				drawControl(CE_ToolButtonLabel, &content_opt, painter, widget);
			}
			
			// Draw the menu indicator if present (also flat)
			if(tb_option->subControls & SC_ToolButtonMenu)
			{
				QStyleOptionToolButton menu_opt = *tb_option;
				menu_opt.subControls = SC_ToolButtonMenu;
				
				// Remove 3D effects from menu indicator
				menu_opt.state &= ~(State_Raised | State_Sunken);
				
				QProxyStyle::drawComplexControl(control, &menu_opt, painter, widget);
			}
			
			return;
		}
	}
	
	// Use default behavior for other complex controls
	QProxyStyle::drawComplexControl(control, option, painter, widget);
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
