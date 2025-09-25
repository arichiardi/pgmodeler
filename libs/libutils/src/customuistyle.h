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

/**
\ingroup libutils
\class CustomUiStyle
\brief Implements a derivative class of QProxyStyle that can be used to override the application UI style at rendering level.
One instance of this class is created in the moment the application is instantiated and the custom style is applied to
all widgets in the application.
*/

#ifndef CUSTOM_UI_STYLE_H
#define CUSTOM_UI_STYLE_H

#include "utilsglobal.h"
#include <QProxyStyle>
#include <QMap>
#include <QStyleOption>
#include <QPainter>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QIcon>

class __libutils CustomUiStyle : public QProxyStyle {
	private:
		// Enum to control which side of the path should be open (only one side at a time)
		enum RectEdge {
			None = 0,    // Closed path (default)
			LeftEdge,        // Open on the left edge
			TopEdge,         // Open on the top edge
			RightEdge,       // Open on the right edge
			BottomEdge       // Open on the bottom edge
		};

				// Enum to control which corners should have rounded edges using bitwise operations
		enum CornerFlag: unsigned {
			NoCorners = 0,
			TopLeft = 1,
			TopRight = 2,
			BottomLeft = 4,
			BottomRight = 8,
			AllCorners = TopLeft | TopRight | BottomLeft | BottomRight
		};

		static QMap<PixelMetric, int> pixel_metrics;
    
		static constexpr qreal BlendFactor = 0.7,
													 PenWidth = 1;
		
		static constexpr int NoRadius = 0,
												 ButtonRadius = 4,
												 InputRadius = 5,
												 FrameRadius = 4,
												 TabRadius = 2,
												 TabBarRadius = 5;

		static constexpr int MinFactor = 120,
							 					 MidFactor = 135,
							 					 MaxFactor = 150;

		// Generic method to create QPainterPath with configurable corner radius and open sides
		QPainterPath createControlShape(const QRect &rect, int radius, CornerFlag corners = AllCorners,
																		qreal dx = 0, qreal dy = 0, qreal dw = 0, qreal dh = 0,
																		RectEdge open_edge = None) const;

		// Draws primitive elements (PE) of buttons
		void drawPEButtonPanel(PrimitiveElement element, const QStyleOption *option,
													 QPainter *painter, const QWidget *widget) const;
		
		// Draws primitive elements (PE) of line edits
		void drawPELineEditPanel(PrimitiveElement element, const QStyleOption *option,
														 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of tabs, group boxes and other framed elements
		void drawPETabWidgetFrame(PrimitiveElement element, const QStyleOption *option,
															QPainter *painter, const QWidget *widget) const;

		void drawPETabBarFrame(PrimitiveElement element, const QStyleOption *option,
													 QPainter *painter, const QWidget *widget) const;
		
		void drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option,	
														 QPainter *painter, const QWidget *widget) const;
		
		 void drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option,	
																 QPainter *painter, const QWidget *widget, int border_radius) const;

		// Draws control elements (CE) of tab bars
		void drawCETabBar(ControlElement element, const QStyleOption *option,	
											QPainter *painter, const QWidget *widget) const;
		
		// Draws complex control (CC) of group boxes and spin boxes
		void drawCCGroupBox(ComplexControl control, const QStyleOptionComplex *option,
												QPainter *painter, const QWidget *widget) const;

		void drawCCSpinBox(ComplexControl control, const QStyleOptionComplex *option,
											 QPainter *painter, const QWidget *widget) const;

		// Draws SpinBox sub-components with specialized styling
		void drawSpinBoxEditField(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
		void drawSpinBoxButton(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, QStyle::SubControl btn_sc_id) const;
		void drawSpinBoxArrow(const QStyleOptionSpinBox *option, QPainter *painter, QStyle::SubControl btn_sc_id) const;

		// Draws primitive elements (PE) of checkboxes and radio buttons
		void drawPECheckBoxRadioBtn(PrimitiveElement element, const QStyleOption *option,
																 QPainter *painter, const QWidget *widget) const;

		// Helper method to add edge with optional rounded corner to QPainterPath
		void addEdgeWithCorner(QPainterPath &path, const QRectF &rect, RectEdge side, int radius) const;

		//! \brief Helper function to get color from palette considering widget state
		static QColor getStateColor(const QPalette& pal, QPalette::ColorRole role, const QStyleOption* option);

		//! \brief Helper function to get color from application palette considering widget state
		static QColor getStateColor(QPalette::ColorRole role, const QStyleOption *option);

		static std::tuple<QColor, QColor, QColor>
					 getStateColors(const QPalette &palette, const QStyleOption *option, const QWidget *widget);

		static std::tuple<QColor, QColor, QColor>
					 getStateColors(const QStyleOption *option, const QWidget *widget);

	public:
		CustomUiStyle();

		CustomUiStyle(const QString &key);

		virtual ~CustomUiStyle() = default;

		/*! \brief Defines a custom pixel metric attribute value globally.
		 * Which means, all instances of this class will share the same pixel metrics values */
		static void setPixelMetricValue(PixelMetric metric, int value);

		int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0) const override;

		QPixmap createGrayMaskedPixmap(const QPixmap &original) const;

	 	void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override;

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;

		QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

		//! \brief Checks if the current palette is dark (dark theme)
		static bool isDarkPalette(const QPalette& pal);

		//! \brief Checks if the current application palette is dark (dark theme)
		static bool isDarkPalette();
};

#endif
