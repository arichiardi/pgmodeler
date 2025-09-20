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


int CustomUiStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
	if(pixel_metrics.contains(metric))
		return pixel_metrics[metric];

	// Use the default pixel metric attribute value if there's no custom value defined
	return QProxyStyle::pixelMetric(metric, option, widget);
}

void CustomUiStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
	qreal curr_opacity = painter->opacity();
	
	/* If opacity is low, draw grayed pixmap and return
	 * indicating a disabled state */
	if(curr_opacity < 0.9)
	{
		painter->save();
		QProxyStyle::drawItemPixmap(painter, rect, alignment, createGrayMaskedPixmap(pixmap));
		painter->restore();
		return;
	}

	QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void CustomUiStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == CE_ToolButtonLabel)
	{
		drawControlToolButtonLabel(element, option, painter, widget);
		return;
	}

	if(element == CE_TabBarTab)
	{
		drawControlTabBarTab(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawControlToolButtonLabel(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	// Special handling for disabled QToolButton in QToolBar
	if (element == CE_ToolButtonLabel && option && !(option->state & State_Enabled))
	{
		const QStyleOptionToolButton *tb_option = qstyleoption_cast<const QStyleOptionToolButton *>(option);

		if (tb_option && widget && widget->parent() && qobject_cast<QToolBar*>(widget->parent()))
		{
			QProxyStyle::drawControl(element, option, painter, widget);
			return;
		}
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawControlTabBarTab(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	// Handle individual QTabBar tabs with flat design styling
	if(element == CE_TabBarTab && option && painter && widget)
	{
		const QStyleOptionTab *tab_opt = qstyleoption_cast<const QStyleOptionTab *>(option);

		if(tab_opt)
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			
			// Use same color scheme as QTabWidget for consistency
			QColor base_background = qApp->palette().color(QPalette::Dark).lighter(120),
						base_border = qApp->palette().color(QPalette::Dark).lighter(160),
						bg_color, border_color;
			
			QRect tab_rect = tab_opt->rect;
			QTabBar::Shape shape = tab_opt->shape;
			bool is_selected = (tab_opt->state & State_Selected);
			
			// Determine colors based on tab state
			if(!(tab_opt->state & State_Enabled))
			{
				bg_color = base_background.darker(135);
				border_color = base_border.darker(135);
			}
			else if(is_selected)
			{
				bg_color = base_background;
				border_color = base_border;
			}
			else if(tab_opt->state & State_MouseOver)
			{
				bg_color = base_background.lighter(110);
				border_color = base_border.lighter(110);
			}
			else
			{
				bg_color = base_background.darker(120);
				border_color = base_border.darker(120);
			}
			
			int radius = 6;
			QPen border_pen(border_color, 1);
			border_pen.setCosmetic(true);
			
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
				tab_rect.setHeight(tab_rect.height() + radius);
			
			// Desenhar aba com cantos superiores arredondados e base reta
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				QPainterPath tab_path;

				// Começa na base esquerda
				tab_path.moveTo(tab_rect.left(), tab_rect.bottom());
				// Sobe reto até antes do canto superior esquerdo
				tab_path.lineTo(tab_rect.left(), tab_rect.top() + radius);
				// Arco superior esquerdo
				tab_path.quadTo(tab_rect.left(), tab_rect.top(), tab_rect.left() + radius, tab_rect.top());
				// Linha reta até antes do canto superior direito
				tab_path.lineTo(tab_rect.right() - radius, tab_rect.top());
				// Arco superior direito
				tab_path.quadTo(tab_rect.right(), tab_rect.top(), tab_rect.right(), tab_rect.top() + radius);
				// Desce reto até a base direita
				tab_path.lineTo(tab_rect.right(), tab_rect.bottom());
				// Fecha na base
				tab_path.lineTo(tab_rect.left(), tab_rect.bottom());

				// Draw background
				painter->setBrush(bg_color);
				painter->setPen(Qt::NoPen);
				painter->drawPath(tab_path);
				
				// Draw border		
				painter->setPen(border_pen);	
				painter->setBrush(Qt::NoBrush);
				painter->drawPath(tab_path);
			}
			else
			{
				// For the other tab shapes, draw simple rectangle
				painter->setBrush(bg_color);
				painter->setPen(Qt::NoPen);
				painter->drawRect(tab_rect);

				painter->setPen(border_pen);	
				painter->setBrush(Qt::NoBrush);
				painter->drawRect(tab_rect);
			}
			
			painter->restore();
			
			// Draw the tab text with proper contrast
			QStyleOptionTab text_option = *tab_opt;

			text_option.palette.setColor(QPalette::ButtonText, 
																	is_selected ? qApp->palette().color(QPalette::WindowText) : 
																	qApp->palette().color(QPalette::WindowText).lighter(30));

			QProxyStyle::drawControl(CE_TabBarTabLabel, &text_option, painter, widget);
			return;
		}
	}

	// For all other elements, use default behavior
	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == PE_PanelButtonTool)
	{
		drawPrimitivePanelButtonTool(element, option, painter, widget);
		return;
	}

	if(element == PE_PanelButtonCommand)
	{
		drawPrimitivePanelButtonCommand(element, option, painter, widget);
		return;
	}

	if(element == PE_FrameTabWidget)
	{
		drawPrimitiveFrameTabWidget(element, option, painter, widget);
		return;
	}

	if(element == PE_FrameTabBarBase)
	{
		drawPrimitiveFrameTabBarBase(element, option, painter, widget);
		return;
	}

	if(element == PE_Frame || element == PE_FrameLineEdit || 
		 element == PE_FrameGroupBox || element == PE_FrameWindow)
	{
		drawPrimitiveFrameElements(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitivePanelButtonTool(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == PE_PanelButtonTool && option && painter && widget)
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		
		// Determine if widget has custom background color
		bool has_custom_color = widget && 
														widget->palette().color(QPalette::Button) != qApp->palette().color(QPalette::Button);
		
		QColor bg_color = has_custom_color ? 
															widget->palette().color(QPalette::Button) : 
															qApp->palette().color(QPalette::Button).lighter(160),
					 border_color = qApp->palette().color(QPalette::Dark).lighter(150);
		
		// Adjust colors based on button state
		if(!(option->state & State_Enabled))
		{
			bg_color = bg_color.darker(135);
			border_color = border_color.darker(135);
		}
		else if(option->state & (State_Sunken | State_On))
		{
			bg_color = bg_color.darker(120);
			border_color = border_color.darker(120);
		}
		else if(option->state & State_MouseOver)
		{
			bg_color = bg_color.lighter(145);
			border_color = border_color.lighter(145);
		}
		
		// Draw background with rounded corners
		QPen border_pen(border_color, 1);
		border_pen.setCosmetic(true);
		
		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, 5, 5);
		
		// Draw border
		painter->setPen(QPen(border_color, 1));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
		
		painter->restore();
		return;
	}

	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitivePanelButtonCommand(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget) 
		return;
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	
	// Use same color scheme as QToolButton for consistency
	QColor bg_color = qApp->palette().color(QPalette::Button).lighter(160),
				 border_color = qApp->palette().color(QPalette::Dark).lighter(169);
	
	// Adjust colors based on button state
	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(140);
		border_color = border_color.darker(140);
	}
	else if(option->state & (State_Sunken | State_On))
	{
		bg_color = bg_color.darker(125);
		border_color = border_color.darker(125);
	}
	else if(option->state & State_MouseOver)
	{
		bg_color = bg_color.lighter(185);
		border_color = border_color.lighter(185);
	}
	
	// Draw background and border with rounded corners
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(option->rect, 5, 5);
	
	painter->setPen(QPen(border_color, 1));
	painter->setBrush(Qt::NoBrush);
	painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
	
	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameTabWidget(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget) 
		return;
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	
	// Use QPalette::Dark as base for container widget - 20 points lighter for background, 60 points lighter for border
	QColor bg_color = qApp->palette().color(QPalette::Dark).lighter(120),
				 border_color = qApp->palette().color(QPalette::Dark).lighter(160);
	
	// Adjust colors based on state - QTabWidget should maintain base colors
	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(155);
		border_color = border_color.darker(155);
	}
	
	// Desenhar retângulo com cantos arredondados apenas na base
	QRectF rect = option->rect;
	int radius = 8; // Raio maior para suavidade

	QPainterPath path;
	// Topo reto
	path.moveTo(rect.left(), rect.top());
	path.lineTo(rect.right(), rect.top());
	// Desce reto até antes do canto inferior direito
	path.lineTo(rect.right(), rect.bottom() - radius);
	// Arco inferior direito suave
	path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom());
	// Linha reta até antes do canto inferior esquerdo
	path.lineTo(rect.left() + radius, rect.bottom());
	// Arco inferior esquerdo suave
	path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);
	// Sobe reto até o topo
	path.lineTo(rect.left(), rect.top());

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(path);

	QPen border_pen(border_color, 1);
	border_pen.setCosmetic(true);

	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path);
	
	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameTabBarBase(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget) 
		return;
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	
	// Use same color scheme as QTabWidget for uniformity
	QColor base_bg_color = qApp->palette().color(QPalette::Dark).lighter(120),
				 border_color = qApp->palette().color(QPalette::Dark).lighter(160);
	
	// Determine orientation if available
	const QStyleOptionTabBarBase *tab_base_opt = 
				qstyleoption_cast<const QStyleOptionTabBarBase *>(option);
	
	QRect frame_rect = option->rect;
	
	// Draw a subtle background that integrates with the tabs
	painter->setBrush(base_bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRect(frame_rect);
	
	// Draw a subtle border line only where needed
	painter->setPen(QPen(border_color, 1));
	
	if(tab_base_opt)
	{
		QTabBar::Shape shape = tab_base_opt->shape;

		// For top tabs, draw line at bottom
		if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			painter->drawLine(frame_rect.bottomLeft(), frame_rect.bottomRight());
		// For bottom tabs, draw line at top
		else if(shape == QTabBar::RoundedSouth || shape == QTabBar::TriangularSouth)
			painter->drawLine(frame_rect.topLeft(), frame_rect.topRight());
		// For left tabs, draw line at right
		else if(shape == QTabBar::RoundedWest || shape == QTabBar::TriangularWest)
			painter->drawLine(frame_rect.topRight(), frame_rect.bottomRight());
		// For right tabs, draw line at left
		else if(shape == QTabBar::RoundedEast || shape == QTabBar::TriangularEast)
			painter->drawLine(frame_rect.topLeft(), frame_rect.bottomLeft());
	}
	// Default: draw bottom line
	else
		painter->drawLine(frame_rect.bottomLeft(), frame_rect.bottomRight());
	
	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameElements(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget) return;
	
	static const QStringList rounded_classes = { "QLineEdit", "QPlainTextEdit", "QComboBox" },
													 rect_classes = { "QTreeWidget", "QTreeView", "QTableView", 
																						"QTableWidget", "NumberedTextEditor" };
	
	auto check_widget = [&](const QWidget *wgt) -> int {
		if(!wgt) 
			return 0;
		
		for(const auto &cls : rounded_classes) 
		{
			if(wgt->inherits(cls.toStdString().c_str())) 
				return 1; // rounded
		}
		
		for(const auto &cls : rect_classes)
		{
			if(wgt->inherits(cls.toStdString().c_str())) 
				return 2; // rectangular
		}
		
		return 0; // no match
	};
	
	// Check widget and its parent hierarchy
	int style_type = check_widget(widget);
	const QWidget *parent = widget->parentWidget();
	
	while(!style_type && parent)
	{
		style_type = check_widget(parent);
		parent = parent->parentWidget();
	}
	
	// If it matches a target class, customize the border
	if(style_type)
	{
		painter->save();
		painter->setPen(QPen(qApp->palette().color(QPalette::Dark).lighter(130), 1));
		
		if(style_type == 1)
		{
			painter->setRenderHints(QPainter::Antialiasing, true);
			painter->drawRoundedRect(option->rect, 6, 6);
		}
		else
			painter->drawRect(option->rect);
		
		painter->restore();
		return;
	}
	
	// Use default behavior for non-customized elements
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap CustomUiStyle::generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap &pixmap, const QStyleOption *option) const
{
	return icon_mode == QIcon::Disabled ? 
		 		 createGrayMaskedPixmap(pixmap) : 
				 QProxyStyle::generatedIconPixmap(icon_mode, pixmap, option);
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
					
			// Apply only to non-transparent pixels
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

