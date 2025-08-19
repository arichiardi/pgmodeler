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

#include "metadatahandlingwidget.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include <QTemporaryFile>

MetadataHandlingWidget::MetadataHandlingWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	root_item = nullptr;
	model_wgt = nullptr;

	htmlitem_deleg=new HtmlItemDelegate(this);
	output_trw->setItemDelegateForColumn(0, htmlitem_deleg);

	backup_file_sel = new FileSelectorWidget(this);
	backup_file_sel->setNameFilters({tr("Objects metadata file (*%1)").arg(GlobalAttributes::ObjMetadataExt), tr("All files (*.*)")});
	backup_file_sel->setWindowTitle(tr("Select backup file"));
	settings_grid->addWidget(backup_file_sel, 3, 1);

	extract_model_sel = new ModelDbSelectorWidget(this);
	settings_grid->addWidget(extract_model_sel, 1, 1);

	apply_model_sel = new ModelDbSelectorWidget(this);
	settings_grid->addWidget(apply_model_sel, 2, 1);

	connect(extract_model_sel, &ModelDbSelectorWidget::s_selectionChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(apply_model_sel, &ModelDbSelectorWidget::s_selectionChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(backup_file_sel, &FileSelectorWidget::s_selectorChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(operation_cmb, &QComboBox::activated, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(operation_cmb, &QComboBox::activated, this, &MetadataHandlingWidget::configureSelector);
	connect(select_all_btn, &QPushButton::clicked, this, &MetadataHandlingWidget::selectAllOptions);
	connect(clear_all_btn,  &QPushButton::clicked, this, &MetadataHandlingWidget::selectAllOptions);

	configureSelector();
	enableMetadataHandling();
	showOutput(false);
}

void MetadataHandlingWidget::showOutput(bool show)
{
	output_trw->setVisible(show);
	progress_wgt->setVisible(show);
	bottom_spacer->changeSize(20, 20, QSizePolicy::Expanding, show ? QSizePolicy::Ignored : QSizePolicy::Expanding);
}

void MetadataHandlingWidget::enableMetadataHandling()
{
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());
	//merge_dup_objs_chk->setEnabled(!extract_only_rb->isChecked());

	merge_dup_objs_chk->setEnabled(op_type != OpExtractOnly);

	if(op_type == OpExtractOnly)
		merge_dup_objs_chk->setChecked(false);

	extract_model_sel->setVisible(op_type != OpRestoreBackup);
	extract_from_lbl->setVisible(op_type != OpRestoreBackup);

	apply_model_sel->setVisible(op_type != OpExtractOnly);
	apply_to_lbl->setVisible(op_type != OpExtractOnly);

	backup_file_sel->setVisible(op_type != OpExtractRestore);
	backup_file_lbl->setVisible(op_type != OpExtractRestore);

	//extract_from_cmb->setVisible(!restore_rb->isChecked());
	//extract_from_lbl->setVisible(!restore_rb->isChecked());
	//apply_to_lbl->setVisible(!extract_only_rb->isChecked());
	//apply_to_edt->setVisible(!extract_only_rb->isChecked());

	/*apply_btn->setEnabled(model_wgt &&
												(((extract_restore_rb->isChecked() && extract_from_cmb->count() > 0) ||
													(extract_only_rb->isChecked() && extract_from_cmb->count() > 0 && !backup_file_sel->getSelectedFile().isEmpty() && !backup_file_sel->hasWarning()) ||
													(restore_rb->isChecked() && !backup_file_sel->getSelectedFile().isEmpty() && !backup_file_sel->hasWarning()))));*/

	//emit s_metadataHandlingEnabled();
}

void MetadataHandlingWidget::selectAllOptions()
{
	bool check = sender() == select_all_btn;

	for(auto &chk : options_gb->findChildren<QCheckBox *>())
		chk->setChecked(check);
}

void MetadataHandlingWidget::setModelWidget(ModelWidget *model_wgt)
{
	this->model_wgt=model_wgt;

	//apply_to_edt->clear();

	//if(model_wgt)
	//{
	//	apply_to_edt->setText(QString("%1 (%2)").arg(model_wgt->getDatabaseModel()->getName())
	//												.arg(model_wgt->getFilename().isEmpty() ? tr("model not saved yet") : model_wgt->getFilename()));
	//}
}

void MetadataHandlingWidget::updateModels(const QList<ModelWidget *> &models)
{
	extract_model_sel->updateModels(models);
	apply_model_sel->updateModels(models);
}

void MetadataHandlingWidget::handleObjectsMetada()
{
	if(!backup_file_sel->getSelectedFile().isEmpty() &&
		 backup_file_sel->getSelectedFile() == model_wgt->getFilename())
		throw Exception(tr("The backup file cannot be the same as the input model!"),
										ErrorCode::Custom,	PGM_FUNC,PGM_FILE,PGM_LINE);

	QTemporaryFile tmp_file;
	QString metadata_file;
	DatabaseModel::MetaAttrOptions options = DatabaseModel::MetaNoOpts;
	DatabaseModel *extract_model=nullptr;
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());

	try
	{
		root_item = nullptr;
		output_trw->clear();

		options |= (db_metadata_chk->isChecked() ? DatabaseModel::MetaDbAttributes : DatabaseModel::MetaNoOpts);
		options |= (custom_colors_chk->isChecked() ? DatabaseModel::MetaObjsCustomColors : DatabaseModel::MetaNoOpts);
		options |= (custom_sql_chk->isChecked() ? DatabaseModel::MetaObjsCustomSql : DatabaseModel::MetaNoOpts);
		options |= (objs_positioning_chk->isChecked() ? DatabaseModel::MetaObjsPositioning : DatabaseModel::MetaNoOpts);
		options |= (objs_protection_chk->isChecked() ? DatabaseModel::MetaObjsProtection : DatabaseModel::MetaNoOpts);
		options |= (objs_sql_disabled_chk->isChecked() ? DatabaseModel::MetaObjsSqlDisabled : DatabaseModel::MetaNoOpts);
		options |= (tag_objs_chk->isChecked() ? DatabaseModel::MetaTagObjs : DatabaseModel::MetaNoOpts);
		options |= (textbox_objs_chk->isChecked() ? DatabaseModel::MetaTextboxObjs : DatabaseModel::MetaNoOpts);
		options |= (objs_fadedout_chk->isChecked() ? DatabaseModel::MetaObjsFadeOut : DatabaseModel::MetaNoOpts);
		options |= (objs_collapse_mode_chk->isChecked() ? DatabaseModel::MetaObjsCollapseMode : DatabaseModel::MetaNoOpts);
		options |= (generic_sql_objs_chk->isChecked() ? DatabaseModel::MetaGenericSqlObjs : DatabaseModel::MetaNoOpts);
		options |= (objs_aliases_chk->isChecked() ? DatabaseModel::MetaObjsAliases : DatabaseModel::MetaNoOpts);
		options |= (objs_z_stack_value_chk->isChecked() ? DatabaseModel::MetaObjsZStackValue : DatabaseModel::MetaNoOpts);
		options |= (objs_layers_config_chk->isChecked() ? DatabaseModel::MetaObjsLayersConfig : DatabaseModel::MetaNoOpts);
		options |= (merge_dup_objs_chk->isChecked() ? DatabaseModel::MetaMergeDuplicatedObjs : DatabaseModel::MetaNoOpts);

		connect(model_wgt->getDatabaseModel(), &DatabaseModel::s_objectLoaded, this, &MetadataHandlingWidget::updateProgress, Qt::UniqueConnection);

		//if(extract_restore_rb->isChecked() || extract_only_rb->isChecked())
		if(op_type == OpExtractRestore || op_type == OpExtractOnly)
		{
			//extract_model=reinterpret_cast<DatabaseModel *>(extract_from_cmb->currentData(Qt::UserRole).value<void *>());

			if(op_type == OpExtractOnly)
				metadata_file = backup_file_sel->getSelectedFile();
			else
			{
				//Configuring the temporary metadata file
				tmp_file.setFileTemplate(GlobalAttributes::getTemporaryFilePath(
																	 QString("%1_metadata_XXXXXX%2")
																	 .arg(extract_model->getName(), GlobalAttributes::ObjMetadataExt)));

				tmp_file.open();
				metadata_file=tmp_file.fileName();
				tmp_file.close();
			}

			connect(extract_model, &DatabaseModel::s_objectLoaded, this, &MetadataHandlingWidget::updateProgress, Qt::UniqueConnection);

			root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																									 UtilsNs::formatMessage(tr("Extracting metadata to file `%1'").arg(metadata_file)),
																									 QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

			extract_model->saveObjectsMetadata(metadata_file, static_cast<DatabaseModel::MetaAttrOptions>(options));

			if(/* extract_restore_rb->isChecked()*/
				 op_type == OpRestoreBackup && !backup_file_sel->getSelectedFile().isEmpty())
			{
				root_item->setExpanded(false);
				root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																										 UtilsNs::formatMessage(tr("Saving backup metadata to file `%1'").arg(backup_file_sel->getSelectedFile())),
																										 QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

				model_wgt->getDatabaseModel()->saveObjectsMetadata(backup_file_sel->getSelectedFile());
			}
		}
		else
		{
			metadata_file = backup_file_sel->getSelectedFile();
		}

		if(root_item)
			root_item->setExpanded(false);

		//if(!extract_only_rb->isChecked())
		if(op_type != OpExtractOnly)
		{
			root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																										UtilsNs::formatMessage(tr("Applying metadata from file `%1'").arg(metadata_file)),
																										QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

			model_wgt->getDatabaseModel()->loadObjectsMetadata(metadata_file, static_cast<DatabaseModel::MetaAttrOptions>(options));
			model_wgt->adjustSceneRect(false);
			model_wgt->updateSceneLayers();
			model_wgt->restoreLastCanvasPosition();
			model_wgt->setModified(true);
			model_wgt->updateObjectsOpacity();
		}

		disconnect(model_wgt->getDatabaseModel(), nullptr, this, nullptr);

		if(extract_model)
			disconnect(extract_model, nullptr, this, nullptr);

		emit s_metadataHandled();
	}
	catch(Exception &e)
	{
		QPixmap icon = QPixmap(GuiUtilsNs::getIconPath("error"));

		disconnect(model_wgt->getDatabaseModel(), nullptr, this, nullptr);

		if(extract_model)
			disconnect(extract_model, nullptr, this, nullptr);

		GuiUtilsNs::createOutputTreeItem(output_trw,
																		 UtilsNs::formatMessage(e.getErrorMessage()),
																		 icon, nullptr);

		ico_lbl->setPixmap(icon);
		progress_lbl->setText(tr("Metadata processing aborted!"));

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void MetadataHandlingWidget::showEvent(QShowEvent *)
{
	if(!model_wgt)
	{
		//apply_btn->setEnabled(false);
		//settings_tbw->setEnabled(false);
	}
}

void MetadataHandlingWidget::configureSelector()
{
	//if(extract_restore_rb->isChecked() || extract_only_rb->isChecked())
	if(operation_cmb->currentIndex() == OpExtractRestore ||
		 operation_cmb->currentIndex() == OpExtractOnly)
	{
		backup_file_sel->setFileDialogTitle(tr("Save backup file"));
		backup_file_sel->setFileMustExist(false);
		backup_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	}
	else
	{
		backup_file_sel->setFileDialogTitle(tr("Load backup file"));
		backup_file_sel->setFileMustExist(true);
		backup_file_sel->setAcceptMode(QFileDialog::AcceptOpen);
	}
}

void MetadataHandlingWidget::updateProgress(int progress, QString msg, unsigned int type_id)
{
	ObjectType obj_type=static_cast<ObjectType>(type_id);
	QString fmt_msg=UtilsNs::formatMessage(msg);
	QPixmap icon;

	if(obj_type==ObjectType::BaseObject)
	{
		if(progress==100)
			icon=QPixmap(GuiUtilsNs::getIconPath("info"));
		else
			icon=QPixmap(GuiUtilsNs::getIconPath("alert"));
	}
	else
		icon=QPixmap(GuiUtilsNs::getIconPath(obj_type));

	GuiUtilsNs::createOutputTreeItem(output_trw, fmt_msg, icon, root_item);
	progress_lbl->setText(fmt_msg);
	ico_lbl->setPixmap(icon);
	progress_pb->setValue(progress);
}
