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

ModelDbPickerWidget::ModelDbPickerWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setPickMode(PickDatabase);
	alert_frm->setVisible(false);

	connect(connections_cmb, &QComboBox::activated, this, &ModelDbPickerWidget::listDatabases);
	connect(database_cmb, &QComboBox::activated, this, &ModelDbPickerWidget::s_pickerChanged);
	connect(model_cmb, &QComboBox::activated, this, &ModelDbPickerWidget::s_pickerChanged);
	connect(model_cmb, &QComboBox::activated, this, &ModelDbPickerWidget::updateModelFilename);
}

ModelDbPickerWidget::~ModelDbPickerWidget()
{

}

void ModelDbPickerWidget::setPickMode(PickMode pick_mode)
{
	model_ctrl_wgt->setVisible(pick_mode == PickModel);
	db_ctrl_wgt->setVisible(pick_mode == PickDatabase);
}

Connection ModelDbPickerWidget::getCurrentConnection()
{
	if(database_cmb->currentIndex() <= 0)
		return Connection();

	return *(reinterpret_cast<Connection *>(connections_cmb->currentData().value<void *>()));
}

QString ModelDbPickerWidget::getCurrentDatabase()
{
	if(database_cmb->currentIndex() <= 0)
		return "";

	return database_cmb->currentText();
}

unsigned int ModelDbPickerWidget::getCurrentDatabaseOid()
{
	if(database_cmb->currentIndex() <= 0)
		return 0;

	return database_cmb->currentData().value<unsigned>();
}

ModelWidget *ModelDbPickerWidget::getCurrentModel()
{
	return nullptr;
}

bool ModelDbPickerWidget::isDatabaseSelected()
{
	return database_cmb->currentIndex() >= 1;
}

bool ModelDbPickerWidget::isModelSelected()
{
	return model_cmb->currentIndex() >= 1;
}

bool ModelDbPickerWidget::hasSelection()
{
	return isDatabaseSelected() || isModelSelected();
}

void ModelDbPickerWidget::updateConnections(Connection::ConnOperation def_conn_op)
{
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, def_conn_op);
	connections_cmb->setEnabled(connections_cmb->count() > 0);
	connection_lbl->setEnabled(connections_cmb->isEnabled());

	database_cmb->clear();
	database_cmb->setEnabled(false);
	database_lbl->setEnabled(false);
}

void ModelDbPickerWidget::updateModels(const QList<ModelWidget *> &models)
{
	QVariant data = model_cmb->currentData();

	model_cmb->blockSignals(true);
	model_cmb->clear();

	if(models.isEmpty())
		model_cmb->addItem(tr("No models found"));
	else
		model_cmb->addItem(tr("Found %1 model(s)").arg(models.size()));

	for(auto &model : models)
		model_cmb->addItem(model->getDatabaseModel()->getName(), QVariant::fromValue<void *>(model));

	model_cmb->blockSignals(false);

	int data_idx = model_cmb->findData(data);
	model_cmb->setCurrentIndex(data_idx < 0 ? 0 : data_idx);
	model_cmb->setEnabled(!models.isEmpty());
	model_file_edt->setEnabled(!models.isEmpty());
}

void ModelDbPickerWidget::updateModelFilename()
{
	model_file_edt->clear();

	if(model_cmb->currentIndex() <= 0)
		return;

	ModelWidget *model = reinterpret_cast<ModelWidget *>(model_cmb->currentData().value<void *>());
	model_file_edt->setText(model->getFilename().isEmpty() ? tr("(model not yet saved)") : model->getFilename());
}

void ModelDbPickerWidget::listDatabases()
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

	emit s_pickerChanged();
}

