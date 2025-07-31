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
\ingroup libgui
\class ModelDBPickerWidget
\brief Implements the widget that allows user to choose the model or database to be used in a diff operation
*/

#ifndef MODEL_DB_PICKER_WIDGET_H
#define MODEL_DB_PICKER_WIDGET_H

#include <QWidget>
#include "ui_modeldbpickerwidget.h"

class ModelDBPickerWidget : public QWidget, public Ui::ModelDBPickerWidget {
		Q_OBJECT

	private:

	public:
		enum PickMode {
			PickModel,
			PickDatabase
		};

		explicit ModelDBPickerWidget(QWidget *parent = nullptr);

		~ModelDBPickerWidget();

		void setPickMode(PickMode pick_mode);
};

#endif
