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

#include "modeldbpickerwidget.h"
#include "settings/connectionsconfigwidget.h"
#include "databaseimporthelper.h"
#include "databaseimportwidget.h"

ModelDBPickerWidget::ModelDBPickerWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setPickMode(PickDatabase);
	alert_frm->setVisible(false);

	connect(connections_cmb, &QComboBox::activated, this, &ModelDBPickerWidget::listDatabases);
}

ModelDBPickerWidget::~ModelDBPickerWidget()
{

}

void ModelDBPickerWidget::setPickMode(PickMode pick_mode)
{
	model_ctrl_wgt->setVisible(pick_mode == PickModel);
	db_ctrl_wgt->setVisible(pick_mode == PickDatabase);
}

void ModelDBPickerWidget::updateConnections(Connection::ConnOperation def_conn_op)
{
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, def_conn_op);
	connections_cmb->setEnabled(connections_cmb->count() > 0);
	connection_lbl->setEnabled(connections_cmb->isEnabled());

	database_cmb->clear();
	database_cmb->setEnabled(false);
	database_lbl->setEnabled(false);
}

void ModelDBPickerWidget::listDatabases()
{
	try
	{
		if(connections_cmb->currentIndex() == connections_cmb->count()-1)
		{
			if(ConnectionsConfigWidget::openConnectionsConfiguration(connections_cmb, true))
			{
				updateConnections();
				emit s_connectionsUpdateRequested();
			}
		}

		Connection *conn = reinterpret_cast<Connection *>(connections_cmb->currentData().value<void *>());
		bool is_srv_supported = true;

		if(conn)
		{
			DatabaseImportHelper imp_helper;

			imp_helper.setConnection(*conn);
			DatabaseImportWidget::listDatabases(imp_helper, database_cmb);
			is_srv_supported = imp_helper.getCatalog().isServerSupported();

			if(conn->isAutoBrowseDB())
				database_cmb->setCurrentText(conn->getConnectionParam(Connection::ParamDbName));
		}
		else
			database_cmb->clear();

		database_cmb->setEnabled(database_cmb->count() > 0);
		database_lbl->setEnabled(database_cmb->isEnabled());
		alert_frm->setVisible(Connection::isDbVersionIgnored() && !is_srv_supported);
	}
	catch(Exception &e)
	{
		database_cmb->clear();
		database_cmb->setEnabled(false);
		database_lbl->setEnabled(false);
		Messagebox::error(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

