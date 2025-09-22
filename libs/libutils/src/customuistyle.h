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
		static QMap<PixelMetric, int> pixel_metrics;
    
		static constexpr qreal BlendFactor = 0.7,
													 PenWidth = 2.0;
		
		static constexpr int ButtonRadius = 3,
												 InputRadius = 4,
												 FrameRadius = 4,
												 TabRadius = 6;

		static constexpr int MinFactor = 130,
							 					 MidFactor = 145,
							 					 MaxFactor = 160;

		//! \brief Draws button elements with custom flat design
		void drawPrimitivePanelButton(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
		
		void drawPrimitiveFrameTabWidget(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
		void drawPrimitiveFrameTabBarBase(PrimitiveElement element, const QStyleOption *option,	QPainter *painter, const QWidget *widget) const;
		void drawPrimitiveFrameGroupBox(PrimitiveElement element, const QStyleOption *option,	QPainter *painter, const QWidget *widget) const;
		void drawPrimitiveFrameElements(PrimitiveElement element, const QStyleOption *option,	QPainter *painter, const QWidget *widget) const;

		void drawControlTabBarTab(ControlElement element, const QStyleOption *option,	QPainter *painter, const QWidget *widget) const;

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

		void drawComplexControlGroupBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const;

		QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *option) const override;

		//! \brief Checks if the current palette is dark (dark theme)
		static bool isDarkPalette(const QPalette& pal);

		//! \brief Checks if the current application palette is dark (dark theme)
		static bool isDarkPalette();

private:
		//! \brief Helper function to get color from palette considering widget state
		static QColor getStateColor(const QPalette& pal, QPalette::ColorRole role, const QStyleOption* option);

		//! \brief Helper function to get color from applicationpalette considering widget state
		static QColor getStateColor(QPalette::ColorRole role, const QStyleOption *option);
};

#endif
