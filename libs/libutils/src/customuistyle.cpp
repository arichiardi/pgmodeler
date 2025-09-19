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
#include <QPainterPath>

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

	// Handle individual QTabBar tabs with flat design styling
	if(element == CE_TabBarTab && option && painter && widget)
	{
		const QStyleOptionTab *tab_option = qstyleoption_cast<const QStyleOptionTab *>(option);
		if(tab_option)
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			
			// Use same color scheme as QTabWidget (PE_FrameTabWidget) for consistency
			QColor base_background = qApp->palette().color(QPalette::Button).lighter(220);
			QColor base_border = qApp->palette().color(QPalette::Dark).lighter(160);
			QColor highlight_color = qApp->palette().color(QPalette::Highlight);
			
			QColor background_color = base_background;
			QColor border_color = base_border.lighter(150);
			QRect tab_rect = option->rect;
			
			// Determine tab position and orientation
			QTabBar::Shape shape = tab_option->shape;
			bool is_selected = (option->state & State_Selected);
			bool is_first = (tab_option->position == QStyleOptionTab::Beginning ||
											 tab_option->position == QStyleOptionTab::OnlyOneTab);
			bool is_last = (tab_option->position == QStyleOptionTab::End ||
											tab_option->position == QStyleOptionTab::OnlyOneTab);
			
			// Adjust colors based on tab state using same logic as QToolButton
			if(!(option->state & State_Enabled))
			{
				// Disabled state: darker than base colors
				background_color = base_background.darker(130);
				border_color = border_color.darker(130);
			}
			else if(is_selected)
			{
				// Selected tab: lighter/brighter than base for clear distinction (ACTIVE)
				background_color = base_background.lighter(110);
				border_color = highlight_color.lighter(130);
			}
			else if(option->state & State_MouseOver)
			{
				// Hover state: slightly lighter than base colors
				background_color = base_background.lighter(105);
				border_color = border_color.lighter(105);
			}
			else
			{
				// Inactive tabs: slightly darker than base for distinction
				background_color = base_background.darker(110);
				border_color = border_color;
			}
			
			// Create custom shape based on tab orientation for flat design
			QRect draw_rect = tab_rect;
			int radius = 6;
			
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				// Top tabs - desloca para baixo para esconder cantos inferiores arredondados
				draw_rect.setHeight(tab_rect.height() + radius);
				
				// Para abas inativas, diminuir altura em 5px para simular deseleção
				if(!is_selected)
				{
					draw_rect.moveTop(tab_rect.top() + 5);
					tab_rect.moveTop(tab_rect.top() + 5);
				}
			}
			else
			{
				// Para outras orientações, usar retângulo simples por enquanto
				// (implementar depois uma por vez)
			}
			
			// Draw background with rounded corners
			painter->setBrush(background_color);
			painter->setPen(Qt::NoPen);
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				painter->drawRoundedRect(draw_rect, radius, radius);
			}
			else
			{
				painter->drawRect(tab_rect);
			}
			
			// Draw border with subtle flat styling (sem linha de destaque para abas ativas)
			if(!is_selected)
			{
				// Non-selected tabs get subtle border
				painter->setPen(QPen(border_color, 1));
				painter->setBrush(Qt::NoBrush);
				if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
				{
					painter->drawRoundedRect(draw_rect, radius, radius);
				}
				else
				{
					painter->drawRect(tab_rect);
				}
			}
			// Abas ativas não têm borda adicional (apenas o background diferenciado)
			
			painter->restore();
			
			// Draw the tab text with proper contrast
			QStyleOptionTab text_option = *tab_option;
			QColor text_color = is_selected ? 
				qApp->palette().color(QPalette::WindowText) :
				qApp->palette().color(QPalette::WindowText).lighter(110);
			text_option.palette.setColor(QPalette::ButtonText, text_color);
			QProxyStyle::drawControl(CE_TabBarTabLabel, &text_option, painter, widget);
			return;
		}
	}

	// For all other elements, use default behavior without opacity changes
	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option,
																	QPainter *painter, const QWidget *widget) const
{
	// Handle QToolButton with simple border styling
	if(element == PE_PanelButtonTool && option && painter && widget)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		
		// Base colors for normal state - lighter background and contrasting border
		QColor base_background = qApp->palette().color(QPalette::Button).lighter(220);
		QColor base_border = qApp->palette().color(QPalette::Dark).lighter(160);
		
		QColor background_color = base_background;
		QColor border_color = base_border.lighter(150);
		
		// Check if widget has a custom background color (for color picker buttons)
		if(widget)
		{
			QPalette widget_palette = widget->palette();
			QColor widget_bg = widget_palette.color(QPalette::Button);
			QColor default_bg = qApp->palette().color(QPalette::Button);
			
			// If the widget has a custom background color, use it as base
			if(widget_bg != default_bg)
			{
				background_color = widget_bg;
				// Keep the same border color for consistency
			}
		}
		
		// Adjust colors based on button state using the base colors as reference
		if(!(option->state & State_Enabled))
		{
			// Disabled state: darker than base colors
			if(widget && widget->palette().color(QPalette::Button) != qApp->palette().color(QPalette::Button))
			{
				// For custom colors, just make them slightly darker
				background_color = background_color.darker(130);
			}
			else
			{
				background_color = base_background.darker(130);
			}
			border_color = border_color.darker(130);
		}
		else if(option->state & (State_Sunken | State_On))
		{
			// Pressed/Checked state: darker than base colors
			if(widget && widget->palette().color(QPalette::Button) != qApp->palette().color(QPalette::Button))
			{
				// For custom colors, just make them slightly darker
				background_color = background_color.darker(115);
			}
			else
			{
				background_color = base_background.darker(115);
			}
			border_color = border_color.darker(115);
		}
		else if(option->state & State_MouseOver)
		{
			// Hover state: slightly lighter than base colors
			if(widget && widget->palette().color(QPalette::Button) != qApp->palette().color(QPalette::Button))
			{
				// For custom colors, just make them slightly lighter
				background_color = background_color.lighter(105);
			}
			else
			{
				background_color = base_background.lighter(105);
			}
			border_color = border_color.lighter(105);
		}
		
		// Draw background with rounded corners (4px)
		painter->setBrush(background_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, 4, 4);
		
		// Desenhar borda uniforme (flat) usando retângulo arredondado
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setPen(QPen(border_color, 1));
		painter->setBrush(Qt::NoBrush);
		
		// Desenhar retângulo de borda arredondada sem ajuste
		painter->drawRoundedRect(option->rect, 4, 4);
		
		painter->restore();
		return;
	}

	// Handle QPushButton with flat design styling
	if(element == PE_PanelButtonCommand && option && painter && widget)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		
		// Use same color scheme as QToolButton for consistency
		QColor base_background = qApp->palette().color(QPalette::Button).lighter(220);
		QColor base_border = qApp->palette().color(QPalette::Dark).lighter(160);
		
		QColor background_color = base_background;
		QColor border_color = base_border.lighter(150);
		
		// Adjust colors based on button state
		if(!(option->state & State_Enabled))
		{
			background_color = base_background.darker(130);
			border_color = border_color.darker(130);
		}
		else if(option->state & (State_Sunken | State_On))
		{
			background_color = base_background.darker(115);
			border_color = border_color.darker(115);
		}
		else if(option->state & State_MouseOver)
		{
			background_color = base_background.lighter(105);
			border_color = border_color.lighter(105);
		}
		
		// Draw background with rounded corners
		painter->setBrush(background_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, 4, 4);
		
		// Draw uniform flat border
		painter->setPen(QPen(border_color, 1));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(option->rect, 4, 4);
		
		painter->restore();
		return;
	}

	// Handle QTabBar with flat design styling
	if(element == PE_FrameTabWidget && option && painter && widget)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		
		// Use same color scheme for consistency
		QColor base_background = qApp->palette().color(QPalette::Button).lighter(220);
		QColor base_border = qApp->palette().color(QPalette::Dark).lighter(160);
		
		QColor background_color = base_background;
		QColor border_color = base_border.lighter(150);
		
		// Adjust colors based on state
		if(!(option->state & State_Enabled))
		{
			background_color = base_background.darker(130);
			border_color = border_color.darker(130);
		}
		else if(option->state & State_MouseOver)
		{
			background_color = base_background.lighter(105);
			border_color = border_color.lighter(105);
		}
		
		// Draw background with rounded corners
		painter->setBrush(background_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, 4, 4);
		
		// Draw uniform flat border
		painter->setPen(QPen(border_color, 1));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(option->rect, 4, 4);
		
		painter->restore();
		return;
	}

	// Handle QTabBar base frame with improved flat design integration
	if(element == PE_FrameTabBarBase && option && painter && widget)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		
		// Use same color scheme as QTabWidget for uniformity
		QColor base_background = qApp->palette().color(QPalette::Button).lighter(220);
		QColor border_color = qApp->palette().color(QPalette::Dark).lighter(160).lighter(150);
		
		// Determine orientation if available
		const QStyleOptionTabBarBase *tab_base_option = 
			qstyleoption_cast<const QStyleOptionTabBarBase *>(option);
		
		QRect frame_rect = option->rect;
		
		// Draw a subtle background that integrates with the tabs
		painter->setBrush(base_background);
		painter->setPen(Qt::NoPen);
		painter->drawRect(frame_rect);
		
		// Draw a subtle border line only where needed
		painter->setPen(QPen(border_color, 1));
		
		if(tab_base_option)
		{
			QTabBar::Shape shape = tab_base_option->shape;
			
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				// For top tabs, draw line at bottom
				painter->drawLine(frame_rect.bottomLeft(), frame_rect.bottomRight());
			}
			else if(shape == QTabBar::RoundedSouth || shape == QTabBar::TriangularSouth)
			{
				// For bottom tabs, draw line at top
				painter->drawLine(frame_rect.topLeft(), frame_rect.topRight());
			}
			else if(shape == QTabBar::RoundedWest || shape == QTabBar::TriangularWest)
			{
				// For left tabs, draw line at right
				painter->drawLine(frame_rect.topRight(), frame_rect.bottomRight());
			}
			else if(shape == QTabBar::RoundedEast || shape == QTabBar::TriangularEast)
			{
				// For right tabs, draw line at left
				painter->drawLine(frame_rect.topLeft(), frame_rect.bottomLeft());
			}
		}
		else
		{
			// Default: draw bottom line
			painter->drawLine(frame_rect.bottomLeft(), frame_rect.bottomRight());
		}
		
		painter->restore();
		return;
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
