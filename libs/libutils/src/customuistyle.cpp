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
#include <QApplication>
#include <QPainterPath>
#include <QToolBar>
#include <QPushButton>

QMap<QStyle::PixelMetric, int> CustomUiStyle::pixel_metrics;

CustomUiStyle::CustomUiStyle() : QProxyStyle() {}

CustomUiStyle::CustomUiStyle(const QString& key) : QProxyStyle(key) {}

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

void CustomUiStyle::drawItemPixmap(QPainter *painter, const QRect& rect, int alignment, const QPixmap& pixmap) const
{
	qreal curr_opacity = painter->opacity();

	/*If opacity is low, draw grayed pixmap and return
	 *indicating a disabled state */
	if(curr_opacity < 0.9)
	{
		painter->save();
		QProxyStyle::drawItemPixmap(painter, rect, alignment, 
																createGrayMaskedPixmap(pixmap));
		painter->restore();
		return;
	}

	QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void CustomUiStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == CE_TabBarTab)
	{
		drawControlTabBarTab(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawControlTabBarTab(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	// Handle individual QTabBar tabs with flat design styling
	if(element == CE_TabBarTab && option && painter && widget)
	{
		const QStyleOptionTab *tab_opt = qstyleoption_cast<const QStyleOptionTab*>(option);

		if(tab_opt)
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);

			// Use same color scheme as QTabWidget for consistency
			QColor base_bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor - 10),
			       base_border = getStateColor(QPalette::Dark, option).lighter(MaxFactor - 10), 
						 bg_color, border_color;

			QRect tab_rect = tab_opt->rect;
			QTabBar::Shape shape = tab_opt->shape;
			bool is_selected = (tab_opt->state & State_Selected);

			// Determine colors based on tab state
			if(!(tab_opt->state & State_Enabled))
			{
				bg_color = base_bg_color.darker(MinFactor);
				border_color = base_border.darker(MinFactor);
			}
			else if(is_selected)
			{
				bg_color = base_bg_color;
				border_color = base_border;
			}
			else if(tab_opt->state & State_MouseOver) 
			{
				bg_color = base_bg_color.lighter(MinFactor - 20);
				border_color = base_border.lighter(MinFactor - 20);
			}
			else
			{
				bg_color = base_bg_color.darker(MinFactor - 10);
				border_color = base_border.darker(MinFactor - 10);
			}

			QPen border_pen(border_color, PenWidth);
			border_pen.setCosmetic(true);

			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
				tab_rect.setHeight(tab_rect.height() + TabRadius);

			// Draw tab with rounded top corners and straight base
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				QPainterPath tab_path;

				// Start at left base
				tab_path.moveTo(tab_rect.left(), tab_rect.bottom());
				// Go straight up to before top-left corner
				tab_path.lineTo(tab_rect.left(), tab_rect.top() + TabRadius);
				// Arco superior esquerdo
				tab_path.quadTo(tab_rect.left(), tab_rect.top(), 
												tab_rect.left() + TabRadius, tab_rect.top());
				// Straight line to before top-right corner
				tab_path.lineTo(tab_rect.right() - TabRadius, tab_rect.top());
				// Arco superior direito
				tab_path.quadTo(tab_rect.right(), tab_rect.top(), 
												tab_rect.right(), tab_rect.top() + TabRadius);
				// Go straight down to right base
				tab_path.lineTo(tab_rect.right(), tab_rect.bottom());
				// Close at base
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
			                             is_selected ? 
																	 getStateColor(QPalette::WindowText, option) :
																 	 getStateColor(QPalette::WindowText, option).lighter(30));

			QProxyStyle::drawControl(CE_TabBarTabLabel, &text_option, painter, widget);
			return;
		}
	}

	// For all other elements, use default behavior
	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == PE_PanelButtonTool || 
		 element == PE_PanelButtonCommand)
	{
		drawPrimitivePanelButton(element, option, painter, widget);
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

	if(element == PE_FrameGroupBox)
	{
		drawPrimitiveFrameGroupBox(element, option, painter, widget);
		return;
	}

	if(element == PE_Frame || 
			element == PE_FrameLineEdit || 
			element == PE_FrameWindow)
	{
		drawPrimitiveFrameElements(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::drawPrimitivePanelButton(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_PanelButtonTool && 
			element != PE_PanelButtonCommand) || !option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	/* Determine if the button has custom background color.
	 * If it contains a stylesheet with background-color property,
		* consider it as custom colored button */
	bool has_custom_color =
			widget && widget->styleSheet().contains("background-color");

	QPalette pal = has_custom_color ? widget->palette() : qApp->palette();
	QColor bg_color = getStateColor(pal, QPalette::Button, option),
				 border_color;

	// Check if this is a default QPushButton
	const QPushButton *push_btn = qobject_cast<const QPushButton*>(widget);
	bool is_default = push_btn && 
										push_btn->isDefault() && 
										(option->state & State_Enabled);

	if(is_default)
	{
		border_color = getStateColor(pal, QPalette::Highlight, option);
		
		// Mix Highlight with Button color for a more subtle default background
		QColor hl_color = getStateColor(pal, QPalette::Highlight, option);		
		
		bg_color = QColor(
			(hl_color.red() + bg_color.red() * 2) / 3,
			(hl_color.green() + bg_color.green() * 2) / 3,
			(hl_color.blue() + bg_color.blue() * 2) / 3
		).lighter(MinFactor);
	}
	else if(has_custom_color)
	{
		// Use QColor::lightness() to calculate lightness efficiently
		int lightness = bg_color.lightness();
		
		/* For light colors (luminance > 128), make border darker
		 * For dark colors (luminance <= 128), make border lighter */
		if(lightness > 128)
			// Dark border for light backgrounds
			border_color = bg_color.darker(MinFactor); 
		else
			// Light border for dark backgrounds
			border_color = bg_color.lighter(MaxFactor); 
	}
	else
	{
		bg_color = getStateColor(pal, QPalette::Button, option);
		border_color = bg_color.lighter(MaxFactor);
	}

	// Disable button	state
	if(!(option->state & State_Enabled))
		border_color = bg_color.darker(MinFactor);
	// Selected button state
	else if(option->state & (State_Sunken | State_On))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}
	// Mouse hover state
	else if(option->state & State_MouseOver)
	{
		bg_color = bg_color.lighter(MaxFactor);

		if(!is_default) // Only lighten border for non-default buttons
			border_color = border_color.lighter(MaxFactor);
	}

	// Special focus border (overrides other border colors except for default buttons)
	if((option->state & State_HasFocus) ||
		 (option->state & State_On))
		border_color = getStateColor(pal, QPalette::Highlight, option);

	QPen border_pen(border_color, PenWidth);
	border_pen.setCosmetic(true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(option->rect, ButtonRadius, ButtonRadius);

	// Draw border
	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5),
														ButtonRadius, ButtonRadius);

	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameTabWidget(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QColor bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
	       border_color = bg_color.lighter(MidFactor);

	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}

	// Draw rectangle with rounded corners only at the base
	QRectF rect = option->rect;
	int radius = FrameRadius * 2; // Larger radius for smoothness

	QPainterPath path;
	// Straight top
	path.moveTo(rect.left(), rect.top());
	path.lineTo(rect.right(), rect.top());
	// Go straight down to before bottom-right corner
	path.lineTo(rect.right(), rect.bottom() - radius);
	// Arco inferior direito suave
	path.quadTo(rect.right(), rect.bottom(), 
							rect.right() - radius, rect.bottom());
	// Straight line to before bottom-left corner
	path.lineTo(rect.left() + radius, rect.bottom());
	// Arco inferior esquerdo suave
	path.quadTo(rect.left(), rect.bottom(), 
							rect.left(), rect.bottom() - radius);
	// Go straight up to the top
	path.lineTo(rect.left(), rect.top());

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(path);

	QPen border_pen(border_color, PenWidth);
	border_pen.setCosmetic(true);

	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path);

	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameGroupBox(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Use same color scheme as QTabWidget for consistency
	QColor bg_color = getStateColor(QPalette::Dark, option).lighter(110),
	       border_color = bg_color.lighter(MaxFactor);

	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}

	QRectF rect = option->rect;
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(rect, FrameRadius, FrameRadius);

	QPen border_pen(border_color, PenWidth);
	border_pen.setCosmetic(true);

	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 
													 FrameRadius, FrameRadius);

	painter->restore();
}

void CustomUiStyle::drawPrimitiveFrameTabBarBase(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Use same color scheme as QTabWidget for uniformity
	QColor base_bg_color = getStateColor(QPalette::Mid, option).lighter(MinFactor - 10),
	       border_color = getStateColor(QPalette::Mid, option).lighter(MaxFactor - 10);

	// Determine orientation if available
	const QStyleOptionTabBarBase *tab_base_opt = 
			qstyleoption_cast<const QStyleOptionTabBarBase*>(option);

	QRect frame_rect = option->rect;

	// Draw a subtle background that integrates with the tabs
	painter->setBrush(base_bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRect(frame_rect);

	// Draw a subtle border line only where needed
	painter->setPen(QPen(border_color, PenWidth));

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
	if(!option || !painter || !widget)
		return;

	static const QStringList rounded_classes = { "QLineEdit", "QPlainTextEdit", "QComboBox"},
	                         rect_classes = { "QTreeWidget", "QTreeView", 
																						"QTableView", "QTableWidget",
	                                          "NumberedTextEditor" };

	// Lambda to check widget class, returns 1 for rounded widgets, 2 for rectangular, 0 for no match
	auto check_widget = [&](const QWidget *wgt) -> int
	{
		if(!wgt)
			return 0;

		for(const auto& cls : rounded_classes)
		{
			if(wgt->inherits(cls.toStdString().c_str()))
				return 1; // rounded
		}

		for(const auto& cls : rect_classes)
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
		
		// Default border color
		QColor border_color = getStateColor(QPalette::Button, option).lighter(MidFactor);
		
		// Special focus border
		if(option->state & State_HasFocus)
			border_color = getStateColor(QPalette::Highlight, option);
		
		painter->setPen(QPen(border_color, PenWidth));

		if(style_type == 1)
		{
			painter->setRenderHints(QPainter::Antialiasing, true);
			painter->drawRoundedRect(option->rect, InputRadius, InputRadius);
		}
		else
			painter->drawRect(option->rect);

		painter->restore();
		return;
	}

	// Use default behavior for non-customized elements
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QPixmap CustomUiStyle::generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap& pixmap, const QStyleOption *option) const
{
	return icon_mode == QIcon::Disabled ? 
											createGrayMaskedPixmap(pixmap) :
											QProxyStyle::generatedIconPixmap(icon_mode, pixmap, option);
}

QPixmap CustomUiStyle::createGrayMaskedPixmap(const QPixmap& original) const
{
	if(original.isNull())
		return original;

	// Convert to QImage for desaturation and color blending
	QImage image = original.toImage().convertToFormat(QImage::Format_ARGB32);
	QRgb *line = nullptr, pixel;
	QColor mask_color = qApp->palette().color(QPalette::Disabled, QPalette::Window);
	int gray = 0, 
			final_r = 0, final_g = 0, final_b = 0, alpha = 0, 
			mask_r = mask_color.red(),
	    mask_g = mask_color.green(), 
			mask_b = mask_color.blue();

	// Apply desaturation (grayscale conversion) and blend with color
	for(int y = 0; y < image.height(); ++y)
	{
		// Get pointer to the start of the scan line
		line = reinterpret_cast<QRgb*>(image.scanLine(y));

		for(int x = 0; x < image.width(); ++x)
		{
			pixel = line[x];
			alpha = qAlpha(pixel);

			// Apply only to non-transparent pixels
			if(alpha > 0)
			{
				// Convert to grayscale using standard luminance formula
				gray = QColor::fromRgb(pixel).lightness(); // QColor::lightness() uses standard luminance calculation

				// Blend grayscale with the target mask color
				final_r = (gray *(1.0 - BlendFactor)) + (mask_r *BlendFactor);
				final_g = (gray *(1.0 - BlendFactor)) + (mask_g *BlendFactor);
				final_b = (gray *(1.0 - BlendFactor)) + (mask_b *BlendFactor);

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

void CustomUiStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	if(control == CC_GroupBox)
	{
		drawComplexControlGroupBox(control, option, painter, widget);
		return;
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomUiStyle::drawComplexControlGroupBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
const QStyleOptionGroupBox *group_box_opt = qstyleoption_cast<const QStyleOptionGroupBox*>(option);

	if(control != CC_GroupBox || !option || !painter || !widget)
		return;

	painter->save();
	
	QRect group_rect = group_box_opt->rect,
				title_rect, frame_rect = group_rect;
	
	// Calculate title area if there's text
	bool has_title = !group_box_opt->text.isEmpty();
	
	if(has_title)
	{
		// Create bold font with 80% size to calculate text height
		QFont title_font = painter->font();
		title_font.setBold(true);
		title_font.setPointSizeF(title_font.pointSizeF() * 0.80);
		
		QFontMetrics fm(title_font);
		int text_height = fm.height();
		int padding = 3; // 3px padding above and below title
		int total_title_height = text_height + (2 * padding);
		
		// Title takes the top portion including padding
		title_rect = QRect(group_rect.left(), group_rect.top(), 
											group_rect.width(), total_title_height);
		
		// Frame starts below the title (including padding)
		frame_rect = QRect(group_rect.left(), group_rect.top() + total_title_height, 
											group_rect.width(), group_rect.height() - total_title_height);
	}
	
	// Draw the frame below the title
	if(!frame_rect.isEmpty())
	{
		QStyleOptionFrame frame_opt;
		frame_opt.QStyleOption::operator=(*group_box_opt);
		frame_opt.features = QStyleOptionFrame::None;
		frame_opt.rect = frame_rect;  // Use adjusted frame rectangle
		drawPrimitive(PE_FrameGroupBox, &frame_opt, painter, widget);
	}
	
	// Draw the title above the frame
	if(has_title && !title_rect.isEmpty())
	{
		// Create bold font with 80% size
		QFont title_font = painter->font();
		title_font.setBold(true);
		title_font.setPointSizeF(title_font.pointSizeF() * 0.80);
		painter->setFont(title_font);
		
		// Use state-aware text color
		painter->setPen(getStateColor(QPalette::WindowText, group_box_opt));
		
		// Draw the text in the title area with 3px padding (centered vertically)
		QRect text_draw_rect = title_rect.adjusted(0, 3, 0, -3); // Apply 3px padding top/bottom
		painter->drawText(text_draw_rect, 
											group_box_opt->textAlignment | Qt::AlignVCenter, 
											group_box_opt->text);
	}
	
	painter->restore();	
}

bool CustomUiStyle::isDarkPalette(const QPalette &pal)
{
	/* If text is lighter than background, it's a dark theme
	 * otherwise, it's a light theme */
	return pal.color(QPalette::WindowText).lightness() >
		   pal.color(QPalette::Window).lightness();
}

bool CustomUiStyle::isDarkPalette()
{
	return isDarkPalette(qApp->palette());
}

QColor CustomUiStyle::getStateColor(const QPalette &pal, QPalette::ColorRole role, const QStyleOption *option)
{
	if(!option)
		return pal.color(role);
	
	// Determine color group based on widget state
	QPalette::ColorGroup group = QPalette::Active;
	
	if(!(option->state & State_Enabled))
		group = QPalette::Disabled;
	else if(!(option->state & State_Active))
		group = QPalette::Inactive;
	
	return pal.color(group, role);
}

QColor CustomUiStyle::getStateColor(QPalette::ColorRole role, const QStyleOption *option)
{
	return getStateColor(qApp->palette(), role, option);
}