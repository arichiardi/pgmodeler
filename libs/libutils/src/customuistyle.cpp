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
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qpainterpath.h>
#include <qstyleoption.h>
#include "enumtype.h"

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
			// Use same color scheme as QTabWidget for consistency
			/* QColor base_bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
			       base_border_color = base_bg_color.lighter(MidFactor), 
						 bg_color = base_bg_color, border_color = base_border_color; */

			QColor bg_color = getStateColor(QPalette::Dark, option),
				   border_color = getStateColor(QPalette::Midlight, option),
				   text_color = getStateColor(QPalette::Text, option);

			WidgetState wgt_st(option, widget);

			QRect tab_rect = tab_opt->rect;
			QTabBar::Shape shape = tab_opt->shape;
			
			/* bool is_selected = (tab_opt->state & State_Selected),
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
			} */

			QPen border_pen(border_color, PenWidth);

			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				/* If the tab is not selected we shrink it by moving the top 
				 * coordinate in 2px and the height in 4px. This will avoid
				 * overlapping borders between the tab and the tab bar base */
				//if(!is_selected)
				if(!(tab_opt->state & State_Selected))
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

			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);

			// Draw tab with rounded top corners and straight base
			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				QPainterPath bg_path, border_path;

				bg_path = border_path = createControlShape(tab_rect, TabBarRadius, 
										TopLeft | TopRight, 0, 0, 0, 0, BottomEdge);

				bg_path.lineTo(tab_rect.left(), tab_rect.bottom());

				// Draw background
				painter->setBrush(bg_color);
				painter->setPen(Qt::NoPen);
				painter->drawPath(bg_path);

				// Draw border
				//painter->setBrush(Qt::NoBrush);
				//painter->setPen(is_selected ? bg_color : base_border_color);
				//painter->drawLine(tab_rect.bottomLeft(), tab_rect.bottomRight());
				
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
			QStyleOptionTab text_opt = *tab_opt;

			/* text_option.palette.setColor(QPalette::ButtonText,
			                             is_selected ? 
																	 getStateColor(QPalette::WindowText, option) :
																 	 getStateColor(QPalette::WindowText, option).lighter(30)); */

			text_opt.palette.setColor(QPalette::ButtonText, text_color);
			QProxyStyle::drawControl(CE_TabBarTabLabel, &text_opt, painter, widget);

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

	/* if(element == PE_FrameTabBarBase)
	{
		drawPETabBarFrame(element, option, painter, widget);
		return;
	} */

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
	/* bool has_custom_color =
					widget && widget->styleSheet().contains("background-color"); */

	WidgetState wgt_st(option, widget);

	QPalette pal = wgt_st.has_custom_color ?
									widget->palette() : qApp->palette();

	QColor bg_color = getStateColor(pal, QPalette::Button, option);
		   //border_color = getStateColor(pal, QPalette::Midlight, option);

	if(wgt_st.is_default && wgt_st.is_enabled)
	{
		bg_color = getStateColor(pal, QPalette::Highlight, option).darker(MinFactor);		

		if(wgt_st.is_pressed)
			bg_color = bg_color.darker(MinFactor);
		else if(!wgt_st.is_pressed && wgt_st.is_hovered)
			bg_color = bg_color.lighter(MaxFactor);
		else if(!wgt_st.is_pressed && !wgt_st.is_hovered)
			bg_color = bg_color.lighter(MidFactor);

		//border_color = bg_color.lighter(MidFactor);
	}
	// Pressed on state
	else if(wgt_st.is_pressed)
	{
		bg_color = getStateColor(pal, QPalette::Dark, option);
		//border_color = getStateColor(pal, QPalette::Button, option);
	}
	// Toggled on state
	else if(wgt_st.is_checked)
	{
		bg_color = getStateColor(pal, QPalette::Mid, option);
		//border_color = getStateColor(pal, QPalette::Midlight, option);
	}
	// Mouse hover state
	else if(wgt_st.is_hovered)
	{
		bg_color = getStateColor(pal, QPalette::Midlight, option);
		//border_color = getStateColor(pal, QPalette::Mid, option);
	}
	
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
	WidgetState wgt_st(option, widget);

	// Check if this LineEdit is part of a SpinBox
	bool is_spinbox_child = 
		widget && qobject_cast<const QAbstractSpinBox*>(widget->parentWidget());
	
	// For spinbox children, only round left corners
	CornerFlag corner_flags = is_spinbox_child ? 
		(CustomUiStyle::TopLeft | CustomUiStyle::BottomLeft) : CustomUiStyle::AllCorners;
	
	QPainterPath shape = createControlShape(option->rect, InputRadius, corner_flags);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(bg_color);

	painter->setPen(Qt::NoPen);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPETabWidgetFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameTabWidget || !option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
			   border_color = getStateColor(QPalette::Midlight, option).darker(MinFactor);
	
	QRectF rect = option->rect;
	int radius = TabRadius * 2; // Larger radius for smoothness

	// Only round bottom corners for tabs (open at top)
	QPainterPath path = createControlShape(option->rect, radius, 
		(CustomUiStyle::BottomLeft | CustomUiStyle::BottomRight),
		0.5, 0.5, -0.5, -0.5, None);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(path);

	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path);

	painter->restore();
}

void CustomUiStyle::drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Dark, option),
	       border_color = bg_color.lighter(MidFactor);

	if(!(option->state & State_Enabled))
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners));

	QPen border_pen(border_color, PenWidth);
	border_pen.setCosmetic(true);

	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners,
	                         0.5, 0.5, -0.5, -0.5));

	painter->restore();
}

void CustomUiStyle::drawPETabBarFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	#warning "This method is currently not used."
	if(element != PE_FrameTabBarBase || !option || !painter || !widget)
		return;

	QColor base_bg_color = getStateColor(QPalette::Mid, option).lighter(MinFactor - 10),
	       border_color = getStateColor(QPalette::Mid, option).lighter(MaxFactor - 10);

	const QStyleOptionTabBarBase *tab_base_opt = 
			qstyleoption_cast<const QStyleOptionTabBarBase*>(option);

	QRect frame_rect = option->rect;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(base_bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRect(option->rect);

	painter->restore();
}

void CustomUiStyle::drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option, 
																					 QPainter *painter, const QWidget *widget, int border_radius) const
{
	if(!option || !painter || !widget)
		return;

	QColor border_color = getStateColor(QPalette::Midlight, option);

	WidgetState wgt_st(option, widget);

	if(wgt_st.is_default && wgt_st.is_enabled)
	{
		border_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);

		if(wgt_st.is_pressed)
			border_color = border_color.darker(MinFactor);
		else if(!wgt_st.is_pressed && wgt_st.is_hovered)
			border_color = border_color.lighter(MaxFactor);
		else if(!wgt_st.is_pressed && !wgt_st.is_hovered)
			border_color = border_color.lighter(MidFactor);
	}
	else if(wgt_st.is_pressed)
		border_color = getStateColor(QPalette::Button, option);
	else if(wgt_st.is_checked)
		border_color = getStateColor(QPalette::Midlight, option);
	else if(wgt_st.is_hovered)
		border_color = getStateColor(QPalette::Light, option);
	else if (wgt_st.is_focused)
		border_color = getStateColor(QPalette::Highlight, option);
	else if(wgt_st.has_custom_color)
	{
		border_color = getStateColor(QPalette::Button, option);
		border_color = border_color.lighter(QColor(border_color).lightness() < 128 ? MidFactor : MaxFactor);
	}

	QPainterPath shape;
	
	if(border_radius > 0)
		shape = createControlShape(option->rect, border_radius, 
															 CustomUiStyle::AllCorners, 0.5, 0.5, -0.5, -0.5);
	else
		shape = createControlShape(option->rect, 0, 
															CustomUiStyle::NoCorners, 1, 1, -1, -1);
	painter->save();	
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(QPen(border_color, PenWidth * 1.5));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(shape);
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

	QRect edit_field_rect = subControlRect(CC_SpinBox, spin_opt, SC_SpinBoxEditField, widget);	
	QStyleOption aux_option = *option;
	bool is_enabled = (option->state & State_Enabled),
			 is_focused = (option->state & State_HasFocus),
			 is_pressed = (option->state & State_Sunken),
			 is_hovered = (option->state & State_MouseOver);
	
	// Draw edit field if visible
	if(spin_opt->subControls & SC_SpinBoxEditField && !edit_field_rect.isEmpty())
	{
		aux_option = *option;
		aux_option.rect = edit_field_rect.adjusted(-2, -2, 2, 2);
		drawSpinBoxEditField(&aux_option, painter, widget);
	}

	// Draw up buttons if visible
	drawSpinBoxButton(spin_opt, painter, widget, SC_SpinBoxUp);
	drawSpinBoxButton(spin_opt, painter, widget, SC_SpinBoxDown);

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
	QPainterPath edit_path = createControlShape(option->rect, InputRadius - 2, 
																				CustomUiStyle::TopLeft | CustomUiStyle::BottomLeft);

	// Draw only background (no border for edit field)
	painter->setBrush(bg_color);
	painter->setPen(border_color);
	painter->drawPath(edit_path);

	painter->restore();
}

void CustomUiStyle::drawSpinBoxButton(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, QStyle::SubControl btn_sc_id) const
{
	if(!option || !painter || !widget || 
		 (btn_sc_id != SC_SpinBoxUp && btn_sc_id != SC_SpinBoxDown))
		return;

	/* Create a copy of the option to modify some
	 * properties for button drawing */
	QStyleOptionSpinBox btn_opt = *option;

	// Get button rectangle according to sub-control id
	if(btn_sc_id == SC_SpinBoxUp)
		btn_opt.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
	else
		btn_opt.rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

	if(option->activeSubControls & btn_sc_id)
	{
		if(option->state & State_Sunken)
			btn_opt.state |= State_Sunken;
		else if(option->state & State_MouseOver)
			btn_opt.state |= State_MouseOver;
	}
	else
	{
		// Remove mouse over and sunken states if this button is not active
		btn_opt.state &= ~(State_MouseOver | State_Sunken);
	}

	// Use the same color logic as button panels
	QPalette pal = qApp->palette();
	QColor bg_color = getStateColor(pal, QPalette::Button, option),
		   	 border_color = bg_color.lighter(MaxFactor);

	bool is_enabled = (btn_opt.state & State_Enabled),
			 is_focused = (btn_opt.state & State_HasFocus),
			 is_hovered = (btn_opt.state & State_MouseOver),
			 is_pressed = (btn_opt.state & State_Sunken);

	// Apply state-based color modifications (same as drawPEButtonPanel)
	if(!is_enabled)
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = bg_color.darker(MidFactor);
	}
	else if(is_focused)
		border_color = getStateColor(pal, QPalette::Highlight, option);
	else if(is_pressed)
	{
		bg_color = bg_color.darker(MaxFactor);
		border_color = border_color.darker(MaxFactor);
	}
	else if(is_hovered)
	{
		bg_color = bg_color.lighter(MaxFactor);
		border_color = border_color.lighter(MaxFactor);
	}

	QPen border_pen(border_color, PenWidth);

	QPainterPath button_path;
	QRect rect = btn_opt.rect;
	int radius = ButtonRadius - 2;

	if(btn_sc_id == SC_SpinBoxUp)
	{
		// Up button: only top-right corner rounded
		rect.setHeight(rect.height() + 1);
		rect = rect.adjusted(0, -2, 0, 0);
		button_path = createControlShape(rect, radius, CustomUiStyle::TopRight);
	}
	else
	{
		// Down button: only bottom-right corner rounded
		rect.setHeight(rect.height() - 1);
		rect.translate(0, 1);
		rect = rect.adjusted(0, 0, 0, 2);
		button_path = createControlShape(rect, radius, CustomUiStyle::BottomRight);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(button_path);

	// Draw border normally (all edges)
	painter->setPen(border_pen);
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(button_path);

	// Draw arrow symbol
	drawSpinBoxArrow(&btn_opt, painter, btn_sc_id);
	painter->restore();
}

void CustomUiStyle::drawSpinBoxArrow(const QStyleOptionSpinBox *option, QPainter *painter, QStyle::SubControl btn_sc_id) const
{
	if(!option || !painter || 
		 (btn_sc_id != SC_SpinBoxUp && btn_sc_id != SC_SpinBoxDown))
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
	
	if(btn_sc_id == SC_SpinBoxUp)
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

void CustomUiStyle::addEdgeWithCorner(QPainterPath &path, const QRectF &rect, RectEdge side, int radius) const
{
	qreal x = rect.x();
	qreal y = rect.y();
	qreal w = rect.width();
	qreal h = rect.height();

	if(side == TopEdge)
	{
		// Top edge from current position to top-right corner
		if(radius > 0)
		{
			path.lineTo(x + w - radius, y);
			path.quadTo(x + w, y, x + w, y + radius);
		}
		else
		{
			path.lineTo(x + w, y);
		}
	}
	else if(side == RightEdge)
	{
		// Right edge from current position to bottom-right corner
		if(radius > 0)
		{
			path.lineTo(x + w, y + h - radius);
			path.quadTo(x + w, y + h, x + w - radius, y + h);
		}
		else
		{
			path.lineTo(x + w, y + h);
		}
	}
	else if(side == BottomEdge)
	{
		// Bottom edge from current position to bottom-left corner
		if(radius > 0)
		{
			path.lineTo(x + radius, y + h);
			path.quadTo(x, y + h, x, y + h - radius);
		}
		else
		{
			path.lineTo(x, y + h);
		}
	}
	else if(side == LeftEdge)
	{
		// Left edge from current position to top-left corner
		if(radius > 0)
		{
			path.lineTo(x, y + radius);
			path.quadTo(x, y, x + radius, y);
		}
		else
		{
			path.lineTo(x, y);
		}
	}
}

QPainterPath CustomUiStyle::createControlShape(const QRect &rect,  int radius, CustomUiStyle::CornerFlag corners,
																							 qreal dx, qreal dy, qreal dw, qreal dh, RectEdge open_edge) const
{
	QPainterPath path;
	
	// Apply adjustments to the rectangle
	QRectF adj_rect = QRectF(rect).adjusted(dx, dy, dw, dh);
	
	qreal x = adj_rect.x(),	y = adj_rect.y(),
				w = adj_rect.width(), h = adj_rect.height();
	
	// Extract individual corner radii using bitwise operations
	int tl_radius = (corners & TopLeft) ? radius : 0,
			tr_radius = (corners & TopRight) ? radius : 0,
			bl_radius = (corners & BottomLeft) ? radius : 0,
			br_radius = (corners & BottomRight) ? radius : 0;

	// If all radii are 0 and closed, create a simple rectangle
	if(open_edge == None && radius <= 0)
	{
		path.addRect(adj_rect);
		return path;
	}
	
	if(open_edge == None)
	{
		// Closed rectangle - start from top-left, go clockwise	
		path.moveTo(x + tl_radius, y);
		addEdgeWithCorner(path, adj_rect, TopEdge, tr_radius);
		addEdgeWithCorner(path, adj_rect, RightEdge, br_radius);
		addEdgeWithCorner(path, adj_rect, BottomEdge, bl_radius);
		addEdgeWithCorner(path, adj_rect, LeftEdge, tl_radius);
	}
	// Open rectangle at top edge
	else if(open_edge == TopEdge)
	{
		// Open at top - start from top-right, go clockwise, end at top-left
		path.moveTo(x + w, y + tr_radius);
		addEdgeWithCorner(path, adj_rect, RightEdge, br_radius);
		addEdgeWithCorner(path, adj_rect, BottomEdge, bl_radius);
		addEdgeWithCorner(path, adj_rect, LeftEdge, tl_radius);
	}
	else if(open_edge == RightEdge)
	{
		// Open at right - start from bottom-right, go clockwise, end at top-right
		path.moveTo(x + w - br_radius, y + h);
		addEdgeWithCorner(path, adj_rect, BottomEdge, bl_radius);
		addEdgeWithCorner(path, adj_rect, LeftEdge, tl_radius);
		addEdgeWithCorner(path, adj_rect, TopEdge, tr_radius);
	}
	else if(open_edge == BottomEdge)
	{
		// Open at bottom - start from bottom-left, go clockwise, end at bottom-right
		path.moveTo(x, y + h - bl_radius);
		addEdgeWithCorner(path, adj_rect, LeftEdge, tl_radius);
		addEdgeWithCorner(path, adj_rect, TopEdge, tr_radius);
		addEdgeWithCorner(path, adj_rect, RightEdge, br_radius);
	}
	else if(open_edge == LeftEdge)
	{
		// Open at left - start from top-left, go clockwise, end at bottom-left
		path.moveTo(x + tl_radius, y);
		addEdgeWithCorner(path, adj_rect, TopEdge, tr_radius);
		addEdgeWithCorner(path, adj_rect, RightEdge, br_radius);
		addEdgeWithCorner(path, adj_rect, BottomEdge, bl_radius);
	}
	
	return path;
}