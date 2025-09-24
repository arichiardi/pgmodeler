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
#include <QStyleOptionSpinBox>
#include <QAbstractSpinBox>

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
		drawCETabBar(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawCETabBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
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
			QColor base_bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
			       base_border_color = base_bg_color.lighter(MidFactor), 
						 bg_color = base_bg_color, border_color = base_border_color;

			QRect tab_rect = tab_opt->rect;
			QTabBar::Shape shape = tab_opt->shape;
			bool is_selected = (tab_opt->state & State_Selected),
						is_enabled = (tab_opt->state & State_Enabled),
						is_hovered = (tab_opt->state & State_MouseOver);

			if(!is_hovered && (!is_selected || !is_enabled))
			{
				bg_color = base_bg_color.darker(!is_selected ? MidFactor : MaxFactor);
				border_color = base_border_color.darker(!is_selected ? MidFactor : MaxFactor);
			}
			else if(is_hovered && !is_selected) 
			{
				bg_color = base_bg_color.lighter(MaxFactor);
				border_color = base_border_color.lighter(MaxFactor);
			}

			QPen border_pen(border_color, PenWidth);

			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				/* If the tab is not selected we shrink it by moving the top 
				 * coordinate in 2px and the height in 4px. This will avoid
				 * overlapping borders between the tab and the tab bar base */
				if(!is_selected)
				{
					tab_rect.moveTop(tab_rect.top() + 2);
					tab_rect.setHeight(tab_rect.height() - 4);				
				}
				else
					/* For selected tabs, we reduce the height in 2px just to make their base
					 * to be aligned with the tab widget body at top */
					tab_rect.setHeight(tab_rect.height() - 2);				
				
				tab_rect.setWidth(tab_rect.width() - 1);				
				// Move down 1px to avoid clipping the tab top border
				tab_rect.translate(1, 1);
			}

			// Draw tab with rounded top corners and straight base
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				QPainterPath bg_path, border_path;

				// Start at left base
				border_path.moveTo(tab_rect.left(), tab_rect.bottom());

				border_path.lineTo(tab_rect.left(), tab_rect.top() + TabBarRadius);

				border_path.quadTo(tab_rect.left(), tab_rect.top(), 
													 tab_rect.left() + TabBarRadius, tab_rect.top());

				border_path.lineTo(tab_rect.right() - TabBarRadius, tab_rect.top());

				border_path.quadTo(tab_rect.right(), tab_rect.top(), 
												tab_rect.right(), tab_rect.top() + TabBarRadius);

				border_path.lineTo(tab_rect.right(), tab_rect.bottom());

				
				bg_path = border_path;
				bg_path.lineTo(tab_rect.left(), tab_rect.bottom());

				// Draw background
				painter->setBrush(bg_color);
				painter->setPen(Qt::NoPen);
				painter->drawPath(bg_path);

				// Draw border
				painter->setBrush(Qt::NoBrush);
				painter->setPen(is_selected ? bg_color : base_border_color);
				painter->drawLine(tab_rect.bottomLeft(), tab_rect.bottomRight());
				
				painter->setPen(border_color);
				painter->drawPath(border_path);
			}
			else
			{
				// For the other tab shapes, draw simple rectangle
				qDebug() << "CustomUiStyle::drawCETabBar(): " << shape << " not implemented, drawing rectangle instead.";
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
		drawPEButtonPanel(element, option, painter, widget);
		drawPEGenericElemFrame(PE_FrameButtonTool, option, painter, widget, ButtonRadius);
		return;
	}

	if(element == PE_PanelLineEdit)
	{
		/* Don't draw panel and frame if this LineEdit is part of a SpinBox.
		 * The method drawSpinBoxEditField handle all drawing for SpinBox controls */
		if(!widget || !qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()))
		{
			drawPELineEditPanel(element, option, painter, widget);
			drawPEGenericElemFrame(PE_FrameLineEdit, option, painter, widget, InputRadius);
		}

		return;
	}

	if(element == PE_FrameTabWidget)
	{
		drawPETabWidgetFrame(element, option, painter, widget);
		return;
	}

	if(element == PE_FrameTabBarBase)
	{
		drawPETabBarFrame(element, option, painter, widget);
		return;
	}

	if(element == PE_FrameGroupBox)
	{
		drawPEGroupBoxFrame(element, option, painter, widget);
		return;
	}

	if(element == PE_IndicatorCheckBox ||
		 element == PE_IndicatorRadioButton)
	{
		drawPECheckBoxRadioBtn(element, option, painter, widget);
		return;
	}

	if(element == PE_Frame)
	{
		// Don't draw frame if this is part of a SpinBox edit field
		if(!widget || !qobject_cast<const QAbstractSpinBox*>(widget))
		{
			drawPEGenericElemFrame(element, option, painter, widget, NoRadius);
		}
		return;
	}

	qDebug() << "drawPrimitive(): " << element;
	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::drawPEButtonPanel(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_PanelButtonTool && 
			element != PE_PanelButtonCommand) || !option || !painter || !widget)
		return;

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
	bool is_enabled = (option->state & State_Enabled),
		 is_focused = (option->state & State_HasFocus),
		 is_hovered = (option->state & State_MouseOver),
		 is_pressed = (option->state & State_Sunken),
		 is_checked = (option->state & State_On),
		 is_default = push_btn && push_btn->isDefault() && is_enabled;

	if(is_default && is_enabled)
	{
		// Mix Highlight with Button color for a more subtle default background
		QColor hl_color = getStateColor(pal, QPalette::Highlight, option);		
		
		hl_color = QColor(
			(hl_color.red() + bg_color.red() * 2) / 3,
			(hl_color.green() + bg_color.green() * 2) / 3,
			(hl_color.blue() + bg_color.blue() * 2) / 3
		);

		if(is_pressed)
			bg_color = hl_color.darker(MinFactor);
		else if(!is_pressed && is_hovered)
			bg_color = hl_color.lighter(MaxFactor);
		else if(!is_pressed && !is_hovered)
			bg_color = hl_color;
	}
	else if(!is_enabled)
		bg_color = bg_color.darker(MinFactor);
	// Pressed on state
	else if(is_pressed)
		bg_color = bg_color.darker(!is_checked ? MaxFactor : MinFactor);		
	// Toggled on state
	else if(is_checked)
		bg_color = bg_color.darker(MinFactor);
	// Mouse hover state
	else if(is_hovered)
		bg_color = bg_color.lighter(MaxFactor);
	else
		bg_color = getStateColor(pal, QPalette::Button, option);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(option->rect, ButtonRadius, ButtonRadius);

	painter->restore();
}

void CustomUiStyle::drawPELineEditPanel(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_PanelLineEdit ||  !option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Base, option);
	bool is_enabled = (option->state & State_Enabled),
			 is_hovered = (option->state & State_MouseOver);

	if(!is_enabled)
		bg_color = bg_color.darker(MidFactor);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);

	// Check if this LineEdit is part of a SpinBox
	bool is_spinbox_child = widget && qobject_cast<const QAbstractSpinBox*>(widget->parentWidget());
	
	if(is_spinbox_child)
	{
		// For SpinBox: only left side corners rounded (top-left and bottom-left)
		QPainterPath edit_path;
		QRectF rect = option->rect;
		int radius = InputRadius;

		edit_path.moveTo(rect.right(), rect.bottom());
		edit_path.lineTo(rect.left() + radius, rect.bottom());
		edit_path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);
		edit_path.lineTo(rect.left(), rect.top() + radius);
		edit_path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());
		edit_path.lineTo(rect.right(), rect.top());
		edit_path.lineTo(rect.right(), rect.bottom());

		painter->drawPath(edit_path);
	}
	else
	{
		// Normal LineEdit: all corners rounded
		painter->drawRoundedRect(option->rect, InputRadius, InputRadius);
	}

	painter->restore();
}

void CustomUiStyle::drawPETabWidgetFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameTabWidget || !option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
	       border_color = bg_color.lighter(MidFactor);

	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}

	// Draw rectangle with rounded corners only at the base
	QRectF rect = option->rect;
	int radius = TabRadius * 2; // Larger radius for smoothness

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

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

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

void CustomUiStyle::drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Use same color scheme as QTabWidget for consistency
	QColor bg_color = getStateColor(QPalette::Dark, option),
	       border_color = bg_color.lighter(MidFactor);

	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

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

void CustomUiStyle::drawPETabBarFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameTabBarBase || !option || !painter || !widget)
		return;

	// Use same color scheme as QTabWidget for uniformity
	QColor base_bg_color = getStateColor(QPalette::Mid, option).lighter(MinFactor - 10),
	       border_color = getStateColor(QPalette::Mid, option).lighter(MaxFactor - 10);

	// Determine orientation if available
	const QStyleOptionTabBarBase *tab_base_opt = 
			qstyleoption_cast<const QStyleOptionTabBarBase*>(option);

	QRect frame_rect = option->rect;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw a subtle background that integrates with the tabs
	painter->setBrush(base_bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRect(frame_rect);

	// Draw a subtle border line only where needed
	QPen border_pen(border_color, PenWidth);
	border_pen.setCosmetic(true);
	painter->setPen(border_pen);
	
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

void CustomUiStyle::drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option, 
																					 QPainter *painter, const QWidget *widget, int border_radius) const
{
	if(!option || !painter || !widget)
		return;

	bool has_custom_color =
			widget && widget->styleSheet().contains("background-color");

	const QPushButton *push_btn = 
			qobject_cast<const QPushButton*>(widget);

	QPalette pal = has_custom_color ? 
								 widget->palette() : qApp->palette();
	
	QColor border_color = getStateColor(pal, QPalette::Dark, option).lighter(MaxFactor);
	
	bool is_enabled = option->state & State_Enabled,
			 is_checked = option->state & State_On,
			 is_hover = option->state & State_MouseOver,
			 is_focused = option->state & State_HasFocus,
			 is_pressed = option->state & State_Sunken,
			 is_default = push_btn && 
										push_btn->isDefault() && 
										is_enabled;

	if(!is_enabled)
		border_color = border_color.darker(MaxFactor);
	else if(is_default || is_checked)
		border_color = getStateColor(pal, QPalette::Highlight, option);
	else if(is_focused)
		border_color = getStateColor(pal, QPalette::Highlight, option);
	else if(is_hover)
		border_color = border_color.lighter(MaxFactor);
	else if(is_pressed)
		border_color = border_color.darker(MinFactor); 
	else if(has_custom_color)
	{
		// Use QColor::lightness() to calculate lightness efficiently
		QColor bg_color = getStateColor(pal, QPalette::Base, option);
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

	painter->save();	
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRectF rect = option->rect;
	painter->setPen(QPen(border_color, PenWidth * 1.5));

	if(border_radius > 0)
		painter->drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), border_radius, border_radius);
	else
		painter->drawRect(rect.adjusted(1, 1, -1, -1));

	painter->restore();
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
		line = reinterpret_cast<QRgb *>(image.scanLine(y));

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
		drawCCGroupBox(control, option, painter, widget);
		return;
	}

	if(control == CC_SpinBox)
	{
		drawCCSpinBox(control, option, painter, widget);
		return;
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomUiStyle::drawCCGroupBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionGroupBox *group_box_opt = 
			qstyleoption_cast<const QStyleOptionGroupBox*>(option);

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

void CustomUiStyle::drawCCSpinBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionSpinBox *spin_opt = qstyleoption_cast<const QStyleOptionSpinBox*>(option);
	
	if(control != CC_SpinBox || !spin_opt || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Get the sub-control rectangles for edit field, up and down buttons
	QRect edit_field_rect = subControlRect(CC_SpinBox, spin_opt, SC_SpinBoxEditField, widget);
	QRect up_button_rect = subControlRect(CC_SpinBox, spin_opt, SC_SpinBoxUp, widget);
	QRect down_button_rect = subControlRect(CC_SpinBox, spin_opt, SC_SpinBoxDown, widget);

	// Draw edit field if visible
	if(spin_opt->subControls & SC_SpinBoxEditField && !edit_field_rect.isEmpty())
	{
		QStyleOption edit_option = *option;
		edit_option.rect = edit_field_rect.adjusted(-2, -2, 2, 2);
		drawSpinBoxEditField(&edit_option, painter, widget);
	}

	// Draw up button if visible
	if(spin_opt->subControls & SC_SpinBoxUp && !up_button_rect.isEmpty())
	{
		QStyleOption up_option = *option;
		
		// Determine button state based on active sub-control and original state
		if(spin_opt->activeSubControls & SC_SpinBoxUp)
		{
			if(option->state & State_Sunken)
				up_option.state |= State_Sunken;
			else if(option->state & State_MouseOver)
				up_option.state |= State_MouseOver;
		}
		else
		{
			// Remove mouse over and sunken states if this button is not active
			up_option.state &= ~(State_MouseOver | State_Sunken);
		}

		up_option.rect = up_button_rect.adjusted(0, -2, 0, 0);
		drawSpinBoxButton(&up_option, painter, widget, true); // true = up button
	}

	// Draw down button if visible
	if(spin_opt->subControls & SC_SpinBoxDown && !down_button_rect.isEmpty())
	{
		QStyleOption down_option = *option;
		
		// Determine button state based on active sub-control and original state
		if(spin_opt->activeSubControls & SC_SpinBoxDown)
		{
			if(option->state & State_Sunken)
				down_option.state |= State_Sunken;
			else if(option->state & State_MouseOver)
				down_option.state |= State_MouseOver;
		}
		else
		{
			// Remove mouse over and sunken states if this button is not active
			down_option.state &= ~(State_MouseOver | State_Sunken);
		}

		down_option.rect = down_button_rect.adjusted(0, 0, 0, 2);
		drawSpinBoxButton(&down_option, painter, widget, false); // false = down button
	}

	painter->restore();
}

void CustomUiStyle::drawSpinBoxEditField(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Get background color - no border needed for edit field
	QPalette pal = qApp->palette();
	QColor bg_color = getStateColor(pal, QPalette::Base, option),
		  	 border_color = getStateColor(pal, QPalette::Dark, option).lighter(MaxFactor);
	bool is_enabled = (option->state & State_Enabled),
		 	 is_focused = (option->state & State_HasFocus),
		 	 is_hovered = (option->state & State_MouseOver);

	if(!is_enabled)
	{
		bg_color = bg_color.darker(MinFactor);
		border_color = border_color.darker(MinFactor);
	}
	else if(is_focused)
		border_color = getStateColor(pal, QPalette::Highlight, option);
	else if(is_hovered)
	{
		bg_color = bg_color.lighter(MaxFactor);
		border_color = border_color.lighter(MaxFactor);
	}

	// Create custom path for edit field with specific rounded corners
	QPainterPath edit_path;
	QRectF rect = option->rect;
	int radius = InputRadius - 2;

	// Edit field: only left side corners rounded (top-left and bottom-left)
	edit_path.moveTo(rect.right(), rect.bottom());
	edit_path.lineTo(rect.left() + radius, rect.bottom());
	edit_path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - radius);
	edit_path.lineTo(rect.left(), rect.top() + radius);
	edit_path.quadTo(rect.left(), rect.top(), rect.left() + radius, rect.top());
	edit_path.lineTo(rect.right(), rect.top());
	edit_path.lineTo(rect.right(), rect.bottom());

	// Draw only background (no border for edit field)
	painter->setBrush(bg_color);
	painter->setPen(border_color);
	painter->drawPath(edit_path);

	painter->restore();
}

void CustomUiStyle::drawSpinBoxButton(const QStyleOption *option, QPainter *painter, const QWidget *widget, bool is_up_button) const
{
	if(!option || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Use the same color logic as button panels
	QPalette pal = qApp->palette();
	QColor bg_color = getStateColor(pal, QPalette::Button, option),
		   	 border_color = bg_color.lighter(MaxFactor);
	bool is_enabled = (option->state & State_Enabled),
			 is_focused = (option->state & State_HasFocus),
			 is_hovered = (option->state & State_MouseOver),
			 is_pressed = (option->state & State_Sunken);

	// Apply state-based color modifications (same as drawPEButtonPanel)
	if(!is_enabled)
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = bg_color.darker(MinFactor);
	}
	else if(is_pressed)
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}
	else if(is_hovered)
	{
		bg_color = bg_color.lighter(MaxFactor);
		border_color = border_color.lighter(MaxFactor);
	}
	else if(is_focused)
		border_color = getStateColor(pal, QPalette::Highlight, option);

	QPen border_pen(border_color, PenWidth);

	// Create custom path for button with specific rounded corners
	QPainterPath button_path;
	QRectF rect = option->rect; // Adjust for pen width
	int radius = ButtonRadius - 2;

	if(is_up_button)
	{
		// Up button: only top-right corner rounded
		rect.setHeight(rect.height() + 1); // Fix 1px gap at bottom
		button_path.moveTo(rect.left(), rect.bottom());
		button_path.lineTo(rect.left(), rect.top());
		button_path.lineTo(rect.right() - radius, rect.top());
		button_path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + radius);
		button_path.lineTo(rect.right(), rect.bottom());
		button_path.lineTo(rect.left(), rect.bottom());
	}
	else
	{
		rect.setHeight(rect.height() - 1); // Fix 1px gap at bottom
		rect.translate(0, 1); // Move down 1px to align with up button
		
		// Down button: only bottom-right corner rounded
		button_path.moveTo(rect.left(), rect.top());
		button_path.lineTo(rect.right(), rect.top());
		button_path.lineTo(rect.right(), rect.bottom() - radius);
		button_path.quadTo(rect.right(), rect.bottom(), rect.right() - radius, rect.bottom());
		button_path.lineTo(rect.left(), rect.bottom());
		button_path.lineTo(rect.left(), rect.top());
	}

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(button_path);

	// Draw border normally (all edges)
	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(button_path);

	// Draw arrow symbol
	drawSpinBoxArrow(option, painter, is_up_button);

	painter->restore();
}

void CustomUiStyle::drawSpinBoxArrow(const QStyleOption *option, QPainter *painter, bool is_up_button) const
{
	if(!option || !painter)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Use text color that adapts to the button state and theme
	QColor arrow_color = getStateColor(QPalette::ButtonText, option);
	
	// Adjust arrow color based on button state for better visibility
	if(!(option->state & State_Enabled))
		arrow_color = arrow_color.lighter(150); // Lighter for disabled state
	else if(option->state & State_Sunken)
		arrow_color = arrow_color.darker(110); // Slightly darker when pressed

	// Calculate arrow geometry - fixed size calculation to ensure consistency
	QRect button_rect = option->rect;
	QPointF center = button_rect.center();
	
	// Fixed arrow dimensions (same for both buttons) - discount 3px from each side
	qreal arrow_width = button_rect.width() * 0.6;  // Remove 6px total (3px each side)
	qreal arrow_height = button_rect.height() * 0.6;	   // Remove 6px total (3px each side)
	qreal half_width = arrow_width * 0.5;
	qreal half_height = arrow_height * 0.5;

	QPolygonF arrow;
	
	if(is_up_button)
	{
		// Up arrow (triangle pointing up) - same dimensions as down arrow
		arrow << QPointF(center.x(), center.y() - half_height)          // Top point
					<< QPointF(center.x() - half_width, center.y() + half_height)    // Bottom left
					<< QPointF(center.x() + half_width, center.y() + half_height);   // Bottom right
	}
	else
	{
		// Down arrow (triangle pointing down) - same dimensions as up arrow
		arrow << QPointF(center.x(), center.y() + half_height)          // Bottom point
					<< QPointF(center.x() - half_width, center.y() - half_height)    // Top left
					<< QPointF(center.x() + half_width, center.y() - half_height);   // Top right
	}

	// Draw the arrow
	painter->setBrush(arrow_color);
	painter->setPen(Qt::NoPen);
	painter->drawPolygon(arrow);

	painter->restore();
}

void CustomUiStyle::drawPECheckBoxRadioBtn(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_IndicatorCheckBox &&
		  element != PE_IndicatorRadioButton) || !option || !painter)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRectF obj_rect = option->rect;
	
	QColor border_color = getStateColor(QPalette::Dark, option).lighter(MidFactor);
	QColor bg_color = getStateColor(QPalette::Base, option);
	QColor ind_color = getStateColor(QPalette::WindowText, option);

	bool is_checked = (option->state & State_On),
		 	 is_pressed = (option->state & State_Sunken);

	if(is_pressed)
	{
		bg_color = bg_color.lighter(MidFactor);
		border_color = border_color.lighter(MidFactor);
	}

	// Draw checkbox background
	painter->setBrush(bg_color);
	painter->setPen(QPen(border_color, PenWidth));

	if(element == PE_IndicatorCheckBox)
		painter->drawRoundedRect(obj_rect.adjusted(0.5, 0.5, -0.5, -0.5), 2, 2);
	else // PE_IndicatorRadioButton
		painter->drawEllipse(obj_rect.adjusted(0.5, 0.5, -0.5, -0.5));

	if(is_checked)
	{
		// Draw the indicator rectangle
		painter->setBrush(ind_color);
		painter->setPen(Qt::NoPen);

		if(element == PE_IndicatorCheckBox)
			painter->drawRoundedRect(obj_rect.adjusted(3, 3, -3, -3), 1, 1);
		else
			painter->drawEllipse(obj_rect.adjusted(3, 3, -3, -3));
	}

	painter->restore();
}