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
\class ModelDbSelectorWidget
\brief Implements the widget that allows user to choose the model or database to be used in a diff operation
*/

#ifndef MODEL_DB_SELECTOR_WIDGET_H
#define MODEL_DB_SELECTOR_WIDGET_H

#include <QWidget>
#include "ui_modeldbselectorwidget.h"
#include "connection.h"
#include "widgets/modelwidget.h"

class ModelDbSelectorWidget : public QWidget, public Ui::ModelDbSelectorWidget {
	Q_OBJECT

	private:
		void updateConnections(Connection::ConnOperation def_conn_op = Connection::OpNone);
		void updateModels(const QList<ModelWidget *> &models);

	public:
		enum SelectMode {
			SelectModel,
			SelectDatabase
		};

		explicit ModelDbSelectorWidget(QWidget *parent = nullptr);

		~ModelDbSelectorWidget() = default;

		void setSelectMode(SelectMode sel_mode);

		Connection getSelectedConnection();
		QString getSelectedDatabase();
		unsigned getSelectedDatabaseOid();
		ModelWidget *getSelectedModel();

		bool isDatabaseSelected();
		bool isModelSelected();
		bool hasSelection();

	public slots:
		void listDatabases();
		void updateModelFilename();

	signals:
		void s_connectionsUpdateRequested();
		void s_selectionChanged();

	friend class DiffToolWidget;
};

#endif
