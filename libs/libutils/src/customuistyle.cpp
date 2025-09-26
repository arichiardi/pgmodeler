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
#include <QComboBox>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qpainterpath.h>
#include <qscrollbar.h>
#include <qstyleoption.h>
#include "enumtype.h"

QMap<QStyle::PixelMetric, int> CustomUiStyle::pixel_metrics;

CustomUiStyle::CustomUiStyle() : QProxyStyle() {}

CustomUiStyle::CustomUiStyle(const QString& key) : QProxyStyle(key) {}

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

void CustomUiStyle::drawCCComboBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionComboBox *combo_opt =
					qstyleoption_cast<const QStyleOptionComboBox *>(option);
	
	if(control != CC_ComboBox || !combo_opt || !painter || !widget)
		return;

	// Check if this is an editable combo box
	const QComboBox *combo_widget = qobject_cast<const QComboBox*>(widget);
	bool is_editable = combo_widget && combo_widget->isEditable();

	if(is_editable)
		// For editable combo boxes, draw custom background and border
		drawEditableComboBox(combo_opt, painter, widget);
	else
		// For non-editable combo boxes, use default implementation
		QProxyStyle::drawComplexControl(control, option, painter, widget);

	// Draw custom arrow if the drop down button is visible
	if(combo_opt->subControls & SC_ComboBoxArrow)
	{
		QRect arrow_rect = subControlRect(CC_ComboBox, combo_opt, SC_ComboBoxArrow, widget);

		if(!arrow_rect.isEmpty())
		{
			QStyleOption arrow_option = *option;
			arrow_option.rect = arrow_rect;

			// ComboBox arrow always points down
			drawControlArrow(&arrow_option, painter, widget, ArrowDown);
		}
	}
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
		title_rect.adjust(0, 3, 0, -3); // Apply 3px padding top/bottom
		painter->drawText(title_rect, 
											group_box_opt->textAlignment | Qt::AlignVCenter, 
											group_box_opt->text);
	}
	
	painter->restore();	
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

void CustomUiStyle::drawCETabBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	// Handle individual QTabBar tabs with flat design styling
	if(element == CE_TabBarTab && option && painter && widget)
	{
		const QStyleOptionTab *tab_opt = qstyleoption_cast<const QStyleOptionTab*>(option);

		if(tab_opt)
		{
			QColor bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor),
			   		 border_color = getStateColor(QPalette::Mid, option).lighter(MinFactor),
			   		 text_color = getStateColor(QPalette::ButtonText, option);
	
			WidgetState wgt_st(option, widget);
			QRect tab_rect = tab_opt->rect;
			QTabBar::Shape shape = tab_opt->shape;

			if(!wgt_st.is_selected)
			{
				bg_color = bg_color.darker(MidFactor);
				border_color = border_color.darker(MidFactor);
			}

			if(shape == QTabBar::RoundedNorth || shape == QTabBar::TriangularNorth)
			{
				/* If the tab is not selected we shrink it by moving the top 
				 * coordinate in 2px and the height in 4px. This will avoid
				 * overlapping borders between the tab and the tab bar base */
				if(!(tab_opt->state & State_Selected))
				{
					tab_rect.moveTop(tab_rect.top() + 2);
					tab_rect.setHeight(tab_rect.height() - 5);				
				}
				else
					/* For selected tabs, we reduce the height in 2px just to make their base
					 * to be aligned with the tab widget body at top */
					tab_rect.setHeight(tab_rect.height() - 3);
				
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
				painter->setPen(QPen(border_color, PenWidth));
				painter->drawPath(border_path);

				/* Workaround to avoid a line	between the tab border and the tab bar base
				 * due to anti-aliasing. Below we draw a small fill rectangle at the base
				 * of the tab so the artifact produced by antiliasing can be removed and the
				 * selected tab is visually merged with the tab widget */
				if(wgt_st.is_selected)
				{
					painter->setPen(Qt::NoPen);
					painter->setBrush(bg_color);

					QRectF rect(tab_rect.bottomLeft(),
									tab_rect.bottomRight());

					rect.translate(0.5,0);
					rect.setWidth(rect.width() - 0.5);
					rect.setHeight(rect.height() + 2.5);

					painter->drawRect(rect);
				}
			}
			else
			{
				// For the other tab shapes, draw simple rectangle
				qDebug() << "CustomUiStyle::drawCETabBar(): " << shape << " not implemented, drawing rectangle instead.";
				painter->setBrush(bg_color);
				painter->setPen(Qt::NoPen);
				painter->drawRect(tab_rect);

				painter->setPen(QPen(border_color, PenWidth));
				painter->setBrush(Qt::NoBrush);
				painter->drawRect(tab_rect);
			}

			painter->restore();

			// Draw the tab text with proper contrast
			QProxyStyle::drawControl(CE_TabBarTabLabel, tab_opt, painter, widget);

			return;
		}
	}

	// For all other elements, use default behavior
	QProxyStyle::drawControl(element, option, painter, widget);
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

	if(control == CC_ComboBox)
	{
		drawCCComboBox(control, option, painter, widget);
		return;
	}

	if(control == CC_ScrollBar)
	{
		const QStyleOptionSlider *slider_opt = qstyleoption_cast<const QStyleOptionSlider*>(option);
		if(slider_opt)
		{
			drawCCScrollBar(slider_opt, painter, widget);
			return;
		}
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomUiStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == CE_TabBarTab)
	{
		drawCETabBar(element, option, painter, widget);
		return;
	}

	// NOTE: Scroll bar elements are now handled entirely by drawCCScrollBar (CC_ScrollBar)
	// to avoid conflicts and ensure proper state management
	/*
	if(element == CE_ScrollBarAddLine ||
		 element == CE_ScrollBarSubLine ||
		 element == CE_ScrollBarAddPage ||
		 element == CE_ScrollBarSubPage ||
		 element == CE_ScrollBarSlider ||
		 element == CE_ScrollBarFirst ||
		 element == CE_ScrollBarLast)
	{
		drawCEScrollBar(element, option, painter, widget);
		return;
	}
	*/

	QProxyStyle::drawControl(element, option, painter, widget);
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
		/* Don't draw panel and frame if this LineEdit is part of a SpinBox or ComboBox.
		 * The method drawSpinBoxEditField handle all drawing for SpinBox controls
		 * and drawCCComboBox handles all drawing for ComboBox controls */
		if(!widget || 
			 (!qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()) &&
			  !qobject_cast<const QComboBox*>(widget->parentWidget())))
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
			drawPEGenericElemFrame(element, option, painter, widget, NoRadius);

		return;
	}

	qDebug() << "drawPrimitive(): " << element;
	QProxyStyle::drawPrimitive(element, option, painter, widget);
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

QPolygonF CustomUiStyle::rotatePolygon(const QPolygonF &polygon, qreal degrees)
{
	if(polygon.isEmpty())
		return polygon;

	QPointF center = polygon.boundingRect().center();

	// Create transformation matrix for rotation around center
	QTransform transform;
	transform.translate(center.x(), center.y());
	transform.rotate(degrees);
	transform.translate(-center.x(), -center.y());

	// Apply transformation to the polygon
	return transform.map(polygon);
}

void CustomUiStyle::drawControlArrow(const QStyleOption *option, QPainter *painter, const QWidget *widget, ArrowDirection direction) const
{
	if(!option || !painter)
		return;

	// Use text color that adapts to the button state and theme
	QColor arr_color = getStateColor(QPalette::ButtonText, option);
	WidgetState wgt_st(option, nullptr);
	
	// Adjust arrow color based on button state for better visibility
	if(!wgt_st.is_enabled)
		arr_color = arr_color.lighter(MidFactor); // Lighter for disabled state
	else if(wgt_st.is_pressed)
		arr_color = arr_color.darker(MinFactor); // Slightly darker when pressed

	// Calculate arrow geometry - fixed size calculation to ensure consistency
	QRect btn_rect = option->rect;
	
	// Calculate precise center to avoid rounding issues
	QPointF center = QPointF(btn_rect.x() + btn_rect.width() / 2.0,
	                         btn_rect.y() + btn_rect.height() / 2.0);
	
	// Round to pixel boundaries for crisp rendering
	qreal half_w = qRound(ArrowWidth * 0.5);
	qreal half_h = qRound(ArrowHeight * 0.5);

	// Create base arrow pointing UP (triangle pointing up)
	QPolygonF base_arrow;
	base_arrow << QPointF(center.x(), center.y() - half_h)          // Top point
	           << QPointF(center.x() - half_w, center.y() + half_h)  // Bottom left
	           << QPointF(center.x() + half_w, center.y() + half_h);  // Bottom right

	// Apply rotations based on arrow direction
	QPolygonF arrow;
	
	switch(direction)
	{
		case ArrowUp:
			// Arrow pointing up: Use base arrow
			arrow = base_arrow;
			break;
			
		case ArrowDown:
			// Arrow pointing down: Rotate base arrow 180°
			arrow = rotatePolygon(base_arrow, 180);
			break;
			
		case ArrowLeft:
			// Arrow pointing left: Rotate base arrow 270° clockwise
			arrow = rotatePolygon(base_arrow, 270);
			break;
			
		case ArrowRight:
			// Arrow pointing right: Rotate base arrow 90° clockwise
			arrow = rotatePolygon(base_arrow, 90);
			break;
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw the arrow
	painter->setBrush(arr_color);
	painter->setPen(Qt::NoPen);
	painter->drawPolygon(arrow);

	painter->restore();
}

void CustomUiStyle::drawEditableComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Get colors for background and border
	QColor bg_color = getStateColor(QPalette::Base, option);
	QColor border_color = getStateColor(QPalette::Dark, option).lighter(MaxFactor);

	WidgetState wgt_st(option, widget);

	if(!wgt_st.is_enabled)
	{
		bg_color = bg_color.darker(MinFactor);
		border_color = border_color.darker(MinFactor);
	}
	else if(wgt_st.is_focused)
		border_color = getStateColor(QPalette::Highlight, option);
	else if(wgt_st.is_hovered)
	{
		bg_color = bg_color.lighter(MaxFactor);
		border_color = border_color.lighter(MaxFactor);
	}

	// Create shape with all corners rounded
	QPainterPath combo_shape = createControlShape(option->rect, InputRadius, AllCorners);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(combo_shape);

	// Draw border
	QPainterPath border_shape = createControlShape(option->rect, InputRadius, AllCorners,
	                                               0.5, 0.5, -0.5, -0.5);
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(border_shape);

	painter->restore();
}

void CustomUiStyle::drawPEButtonPanel(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_PanelButtonTool && 
			element != PE_PanelButtonCommand) || !option || !painter || !widget)
		return;

	WidgetState wgt_st(option, widget);
	QColor bg_color = getStateColor(QPalette::Button, option);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.has_custom_color)
			bg_color = getStateColor(widget->palette(), QPalette::Button, option);
		else if(wgt_st.is_default || wgt_st.is_checked)
		{
			bg_color = getStateColor(QPalette::Highlight, option).darker(MidFactor);		

			if(wgt_st.is_pressed)
				bg_color = bg_color.darker(MinFactor);
			else if(!wgt_st.is_pressed && wgt_st.is_hovered)
				bg_color = bg_color.lighter(MaxFactor);
			else if(!wgt_st.is_pressed && !wgt_st.is_hovered)
				bg_color = bg_color.lighter(MidFactor);
		}
		else if(wgt_st.is_pressed)
			bg_color = getStateColor(QPalette::Dark, option);
		else if(wgt_st.is_hovered)
			bg_color = getStateColor(QPalette::Light, option);
	}
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(option->rect, ButtonRadius, ButtonRadius);
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
	WidgetState wgt_st(option, widget);
	
	QColor border_color = getStateColor(QPalette::Dark, option).lighter(MidFactor);
	QColor bg_color = getStateColor(QPalette::Base, option);
	QColor ind_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);

	if(!wgt_st.is_enabled)
		ind_color = getStateColor(QPalette::Mid, option);
	else if(wgt_st.is_pressed)
	{
		ind_color = getStateColor(QPalette::Midlight, option);
		bg_color = bg_color.lighter(MidFactor);
		border_color = border_color.lighter(MidFactor);
	}

	// Draw checkbox background
	painter->setBrush(bg_color);
	painter->setPen(QPen(border_color, PenWidth));

	obj_rect.adjust(0.5, 0.5, -0.5, -0.5);	
	obj_rect.translate(0, 1); // Ensure using float coordinates

	if(element == PE_IndicatorCheckBox)
		painter->drawRoundedRect(obj_rect, 2, 2);
	else // PE_IndicatorRadioButton
		painter->drawEllipse(obj_rect);

	if(wgt_st.is_checked)
	{
		// Draw the indicator rectangle
		painter->setBrush(ind_color);
		painter->setPen(Qt::NoPen);
		obj_rect.adjust(2, 2, -2, -2);	

		if(element == PE_IndicatorCheckBox)
			painter->drawRoundedRect(obj_rect, 1, 1);
		else
			painter->drawEllipse(obj_rect);
	}

	painter->restore();
}

void CustomUiStyle::drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option, 
																					 QPainter *painter, const QWidget *widget, int border_radius) const
{
	if(!option || !painter || !widget)
		return;

	QColor border_color = getStateColor(QPalette::Midlight, option);

	WidgetState wgt_st(option, widget);

	/* Detecting if its a line edit frame, because some states 
	 * are not rendered for it, like hover and pressed */
	bool is_edit_frm = (element == PE_FrameLineEdit),
			is_basic_frm = (element == PE_Frame);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.has_custom_color)
		{
			border_color = getStateColor(widget->palette(),QPalette::Button, option);
			border_color = border_color.lighter(QColor(border_color).lightness() < 128 ? MidFactor : MaxFactor);
		}
		else if(wgt_st.is_default || wgt_st.is_checked)
		{
			border_color = getStateColor(QPalette::Highlight, option).lighter(MinFactor - 10);

			if(wgt_st.is_pressed)
				border_color = border_color.darker(MinFactor);
			else if(!wgt_st.is_pressed && wgt_st.is_hovered)
				border_color = border_color.lighter(MaxFactor);
			else if(!wgt_st.is_pressed && !wgt_st.is_hovered)
				border_color = border_color.lighter(MidFactor);
		}
		else if(wgt_st.is_pressed && !is_edit_frm && !is_basic_frm)
			border_color = getStateColor(QPalette::Button, option);
		else if(wgt_st.is_hovered && !is_edit_frm && !is_basic_frm)
			border_color = getStateColor(QPalette::Light, option).lighter(MinFactor);
		else if (wgt_st.is_focused)
			border_color = getStateColor(QPalette::Highlight, option);
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
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameGroupBox || !option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Dark, option).lighter(MinFactor - 10),
	       border_color = getStateColor(QPalette::Mid, option).lighter(MinFactor - 10);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners));

	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners,
	                         0.5, 0.5, -0.5, -0.5));

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
			   border_color = getStateColor(QPalette::Mid, option).lighter(MinFactor);

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

void CustomUiStyle::drawCCScrollBar(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	WidgetState wgt_st(option, widget);
	
	// Use more subtle colors for scroll bar to reduce prominence
	QColor bg_color = getStateColor(QPalette::Button, option).darker(MinFactor),
				 border_color = getStateColor(QPalette::Midlight, option).darker(MinFactor);

	// Apply state-based color modifications with reduced luminosity
	if(!wgt_st.is_enabled)
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = border_color.darker(MidFactor);
	}
	else if(wgt_st.is_pressed)
	{
		bg_color = getStateColor(QPalette::Dark, option);
		border_color = getStateColor(QPalette::Mid, option);
	}
	else if(wgt_st.is_hovered)
	{
		// Reduced hover brightness to be more subtle
		bg_color = bg_color.lighter(XMinFactor); 
		border_color = border_color.lighter(XMinFactor);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw the scroll bar background/groove
	QRectF sub_ctrl_rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);

	if(!sub_ctrl_rect.isEmpty())
	{
		painter->setBrush(bg_color.darker(MidFactor));
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);
		
		// Draw groove border
		painter->setPen(QPen(border_color.darker(MidFactor), PenWidth));
		painter->setBrush(Qt::NoBrush);

		sub_ctrl_rect.adjust(0.5, 0.5, -0.5, -0.5);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);
	}

	// Draw the scroll bar handle/slider
	sub_ctrl_rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);

	if(!sub_ctrl_rect.isEmpty())
	{
		// Apply special highlighting for pressed/hovered handle
		QColor slider_bg = bg_color,
					 slider_border = border_color;
		
		if(option->activeSubControls & SC_ScrollBarSlider)
		{
			if(wgt_st.is_pressed)
			{
				slider_bg = getStateColor(QPalette::Dark, option);
				slider_border = getStateColor(QPalette::Mid, option);
			}
			else if(wgt_st.is_hovered)
			{
				slider_bg = slider_bg.lighter(MaxFactor);
				slider_border = slider_border.lighter(MaxFactor);
			}
		}

		painter->setBrush(slider_bg);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);
		
		// Draw handle border
		painter->setPen(QPen(slider_border, PenWidth));
		painter->setBrush(Qt::NoBrush);

		sub_ctrl_rect.adjust(0.5, 0.5, -0.5, -0.5);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);
	}

	// Draw scroll bar buttons (AddLine and SubLine)
	QRect addline_rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarAddLine, widget),
				subline_rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSubLine, widget);

	// Draw AddLine button (down/right arrow)
	if(!addline_rect.isEmpty())
	{
		QColor btn_bg = bg_color,
					 btn_border = border_color;
		
		// Apply state-based colors if this button is active
		if(option->activeSubControls & SC_ScrollBarAddLine)
		{
			if(wgt_st.is_pressed)
			{
				btn_bg = getStateColor(QPalette::Dark, option);
				btn_border = getStateColor(QPalette::Mid, option);
			}
			else if(wgt_st.is_hovered)
			{
				btn_bg = btn_bg.lighter(MaxFactor);
				btn_border = btn_border.lighter(MaxFactor);
			}
		}

		// Draw button background
		painter->setBrush(btn_bg);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(addline_rect, ScrollBarRadius, ScrollBarRadius);

		// Draw button border
		painter->setPen(QPen(btn_border, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(addline_rect).adjusted(0.5, 0.5, -0.5, -0.5), 
																			 ScrollBarRadius, ScrollBarRadius);		// Create option for arrow drawing with proper state
		QStyleOption arrow_opt = *option;
		arrow_opt.rect = addline_rect;

		if(option->activeSubControls & SC_ScrollBarAddLine)
		{
			if(wgt_st.is_pressed)
				arrow_opt.state |= State_Sunken;
			else if(wgt_st.is_hovered)
				arrow_opt.state |= State_MouseOver;
		}
		else
		{
			arrow_opt.state &= ~(State_MouseOver | State_Sunken);
		}

		// Draw arrow - AddLine is bottom/right button
		const QScrollBar *scrollbar = qobject_cast<const QScrollBar*>(widget);
		bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
		ArrowDirection arrow_dir = is_horizontal ? ArrowRight : ArrowDown;
		drawControlArrow(&arrow_opt, painter, widget, arrow_dir);
	}

	// Draw SubLine button (up/left arrow)
	if(!subline_rect.isEmpty())
	{
		QColor btn_bg = bg_color,
				btn_border = border_color;

		// Apply state-based colors if this button is active
		if(option->activeSubControls & SC_ScrollBarSubLine)
		{
			if(wgt_st.is_pressed)
			{
				btn_bg = getStateColor(QPalette::Dark, option);
				btn_border = getStateColor(QPalette::Mid, option);
			}
			else if(wgt_st.is_hovered)
			{
				btn_bg = btn_bg.lighter(MaxFactor);
				btn_border = btn_border.lighter(MaxFactor);
			}
		}

		// Draw button background
		painter->setBrush(btn_bg);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(subline_rect, ScrollBarRadius, ScrollBarRadius);

		// Draw button border
		painter->setPen(QPen(btn_border, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(subline_rect).adjusted(0.5, 0.5, -0.5, -0.5), 
																			 ScrollBarRadius, ScrollBarRadius);		// Create option for arrow drawing with proper state
		QStyleOption arrow_opt = *option;
		arrow_opt.rect = subline_rect;
		if(option->activeSubControls & SC_ScrollBarSubLine)
		{
			if(wgt_st.is_pressed)
				arrow_opt.state |= State_Sunken;
			else if(wgt_st.is_hovered)
				arrow_opt.state |= State_MouseOver;
		}
		else
		{
			arrow_opt.state &= ~(State_MouseOver | State_Sunken);
		}

		// Draw arrow - SubLine is top/left button
		const QScrollBar *scrollbar = qobject_cast<const QScrollBar*>(widget);
		bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
		ArrowDirection arrow_dir = is_horizontal ? ArrowLeft : ArrowUp;
		drawControlArrow(&arrow_opt, painter, widget, arrow_dir);
	}

	painter->restore();
}

void CustomUiStyle::drawCEScrollBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Handle scroll bar control elements
	switch(element)
	{
		case CE_ScrollBarAddLine:
		case CE_ScrollBarSubLine:
		{
			// Cast to complex option to access activeSubControls
			const QStyleOptionComplex *complex_option = qstyleoption_cast<const QStyleOptionComplex*>(option);
			
			// Create a copy of the option to modify state for proper hover/pressed detection
			QStyleOption btn_opt = *option;
			
			// Determine the correct SubControl for this element
			QStyle::SubControl sub_control = (element == CE_ScrollBarAddLine) ? SC_ScrollBarAddLine : SC_ScrollBarSubLine;
			
			// Check if this specific button is active (hovered or pressed)
			bool is_active = complex_option && (complex_option->activeSubControls & sub_control);
			
			if(is_active)
			{
				// Apply the correct state based on the main option state
				if(option->state & State_Sunken)
					btn_opt.state |= State_Sunken;
				else if(option->state & State_MouseOver)
					btn_opt.state |= State_MouseOver;
			}
			else
			{
				// Remove hover/pressed states if this button is not active
				btn_opt.state &= ~(State_MouseOver | State_Sunken);
			}

			// Get colors based on the modified button state
			WidgetState wgt_st(&btn_opt, widget);
			QColor bg_color = getStateColor(QPalette::Button, &btn_opt);
			QColor border_color = getStateColor(QPalette::Midlight, &btn_opt);

			// Apply state-based color modifications
			if(!wgt_st.is_enabled)
			{
				bg_color = bg_color.darker(MidFactor);
				border_color = border_color.darker(MidFactor);
			}
			else if(wgt_st.is_pressed)
			{
				bg_color = getStateColor(QPalette::Dark, &btn_opt);
				border_color = getStateColor(QPalette::Mid, &btn_opt);
			}
			else if(wgt_st.is_hovered)
			{
				bg_color = bg_color.lighter(MaxFactor);
				border_color = border_color.lighter(MaxFactor);
			}

			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);

			// Draw button background
			painter->setBrush(bg_color);
			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(option->rect, ScrollBarRadius, ScrollBarRadius);

			// Draw button border
			painter->setPen(QPen(border_color, PenWidth));
			painter->setBrush(Qt::NoBrush);
			painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 
																								 ScrollBarRadius, ScrollBarRadius);			// Use drawControlArrow to draw the arrow with proper state
			// Convert SubControl to ArrowDirection
			ArrowDirection arrow_dir;
			if(sub_control == SC_ScrollBarAddLine)
			{
				// AddLine: bottom/right button
				const QScrollBar *scrollbar = qobject_cast<const QScrollBar*>(widget);
				bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
				arrow_dir = is_horizontal ? ArrowRight : ArrowDown;
			}
			else // SC_ScrollBarSubLine
			{
				// SubLine: top/left button  
				const QScrollBar *scrollbar = qobject_cast<const QScrollBar*>(widget);
				bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
				arrow_dir = is_horizontal ? ArrowLeft : ArrowUp;
			}
			
			drawControlArrow(&btn_opt, painter, widget, arrow_dir);
			
			painter->restore();
			break;
		}
		
		case CE_ScrollBarSlider:
		{
			// This is handled by drawCCScrollBar, but we can provide fallback
			WidgetState wgt_st(option, widget);
			QColor bg_color = getStateColor(QPalette::Button, option);
			QColor border_color = getStateColor(QPalette::Midlight, option);

			if(!wgt_st.is_enabled)
			{
				bg_color = bg_color.darker(MidFactor);
				border_color = border_color.darker(MidFactor);
			}
			else if(wgt_st.is_pressed)
			{
				bg_color = getStateColor(QPalette::Dark, option);
				border_color = getStateColor(QPalette::Mid, option);
			}
			else if(wgt_st.is_hovered)
			{
				bg_color = bg_color.lighter(MaxFactor);
				border_color = border_color.lighter(MaxFactor);
			}

			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);

			painter->setBrush(bg_color);
			painter->setPen(Qt::NoPen);
			painter->drawRoundedRect(option->rect, ScrollBarRadius, ScrollBarRadius);

			painter->setPen(QPen(border_color, PenWidth));
			painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5), 
																			 ScrollBarRadius, ScrollBarRadius);			painter->restore();
			break;
		}
		
		default:
			// For other scroll bar elements, use default rendering
			QProxyStyle::drawControl(element, option, painter, widget);
			break;
	}
}

void CustomUiStyle::drawSpinBoxButton(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, QStyle::SubControl btn_sc_id) const
{
	if(!option || !painter || !widget || 
		 (btn_sc_id != SC_SpinBoxUp && btn_sc_id != SC_SpinBoxDown))
		return;

	/* Create a copy of the option to modify some
	 * properties for button drawing */
	QStyleOptionSpinBox btn_opt = *option;
	WidgetState wgt_st(option, widget);
	QRect rect;

	// Get button rectangle according to sub-control id
	if(btn_sc_id == SC_SpinBoxUp)
		rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
	else
		rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

	if(option->activeSubControls & btn_sc_id)
	{
		if(wgt_st.is_pressed)
			btn_opt.state |= State_Sunken;
		else if(wgt_st.is_hovered)
			btn_opt.state |= State_MouseOver;
	}
	else
		// Remove mouse over and sunken states if this button is not active
		btn_opt.state &= ~(State_MouseOver | State_Sunken);

	// Use the same color logic as button panels
	QColor bg_color = getStateColor(QPalette::Button, option),
		   	 border_color = getStateColor(QPalette::Midlight, option);

	// Apply state-based color modifications (same as drawPEButtonPanel)
	if(!wgt_st.is_enabled)
	{
		bg_color = bg_color.darker(MidFactor);
		border_color = bg_color.darker(MidFactor);
	}
	else if(wgt_st.is_focused)
		border_color = getStateColor(QPalette::Highlight, option);
	else if(wgt_st.is_pressed)
	{
		bg_color = getStateColor(QPalette::Dark, option);
		border_color = getStateColor(QPalette::Mid, option);
	}
	else if(wgt_st.is_hovered)
	{
		bg_color = bg_color.lighter(MaxFactor);
		border_color = border_color.lighter(MaxFactor);
	}

	QPainterPath btn_path;
	int radius = ButtonRadius - 2;

	if(btn_sc_id == SC_SpinBoxUp)
	{
		// Up button: only top-right corner rounded, extend slightly upward
		rect.adjust(0, -2, 0, 1);
		btn_path = createControlShape(rect, radius, CustomUiStyle::TopRight);
	}
	else
	{
		// Down button: only bottom-right corner rounded, extend slightly downward
		rect.adjust(0, 0, 0, 2);
		btn_path = createControlShape(rect, radius, CustomUiStyle::BottomRight);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(btn_path);

	// Draw border normally (all edges)
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(btn_path);

	// Draw arrow symbol
	btn_opt.rect = rect;
	ArrowDirection arrow_dir = (btn_sc_id == SC_SpinBoxUp) ? ArrowUp : ArrowDown;
	drawControlArrow(&btn_opt, painter, widget, arrow_dir);
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

QPixmap CustomUiStyle::generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap& pixmap, const QStyleOption *option) const
{
	return icon_mode == QIcon::Disabled ? 
											createGrayMaskedPixmap(pixmap) :
											QProxyStyle::generatedIconPixmap(icon_mode, pixmap, option);
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
