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

#include "configurationwidget.h"
#include "guiutilsns.h"

ConfigurationWidget::ConfigurationWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	general_conf = new GeneralConfigWidget;
	appearance_conf = new AppearanceConfigWidget;
	connections_conf = new ConnectionsConfigWidget;
	relationships_conf = new RelationshipConfigWidget;
	snippets_conf = new SnippetsConfigWidget;
	plugins_conf = new PluginsConfigWidget;

	QList<BaseConfigWidget *> wgt_list={ general_conf, appearance_conf, relationships_conf,
																			 connections_conf, snippets_conf, plugins_conf };

	for(auto &wgt : wgt_list)
	{
		confs_stw->addWidget(wgt);

		connect(wgt, &BaseConfigWidget::s_configurationChanged, this, [this](bool changed) {
			apply_btn->setEnabled(changed);
			revert_btn->setEnabled(changed);
		});
	}

	connect(revert_btn, &QPushButton::clicked, this, &ConfigurationWidget::revertConfiguration);
	connect(apply_btn,  &QPushButton::clicked, this, __slot(this, ConfigurationWidget::applyConfiguration));
	connect(defaults_btn,  &QPushButton::clicked, this, __slot(this, ConfigurationWidget::restoreDefaults));

	QFont fnt;
	int view_idx = GeneralConfWgt;
	QList<QToolButton *> btns = { general_tb, appearance_tb, relationships_tb,
																connections_tb, snippets_tb, plugins_tb };

	for(auto &btn : btns)
	{
		fnt = btn->font();
		fnt.setWeight(QFont::Normal);
		btn->setFont(fnt);
		GuiUtilsNs::createDropShadow(btn, 1, 1, 5);
		btn->setProperty(Attributes::ObjectId.toStdString().c_str(), view_idx++);
		connect(btn, &QToolButton::toggled, this, &ConfigurationWidget::changeCurrentView);
	}
}

ConfigurationWidget::~ConfigurationWidget()
{
	connections_conf->destroyConnections();
}

void ConfigurationWidget::changeCurrentView()
{
	QToolButton *btn = nullptr,
			*btn_sender = qobject_cast<QToolButton *>(sender());
	QList<QToolButton *> child_btns = btns_parent_wgt->findChildren<QToolButton *>();

	child_btns.removeOne(btn_sender);

	for(auto &obj : child_btns)
	{
		btn = dynamic_cast<QToolButton *>(obj);

		if(!btn)
			continue;

		btn->blockSignals(true);
		btn->setChecked(false);
		btn->blockSignals(false);
	}

	if(btn_sender)
	{
		/* Forcing the sender button to keep the checked state
		 * when the user clicks it when it is already checked */
		btn_sender->blockSignals(true);
		btn_sender->setChecked(true);
		btn_sender->blockSignals(false);

		confs_stw->setCurrentIndex(btn_sender->property(Attributes::ObjectId.toStdString().c_str()).toInt());
	}
}

void ConfigurationWidget::hideEvent(QHideEvent *event)
{
	general_tb->setChecked(true);
}

void ConfigurationWidget::showEvent(QShowEvent *)
{
	snippets_conf->snippet_txt->updateLineNumbers();
}

void ConfigurationWidget::revertConfiguration()
{
	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
		{
			if(conf_wgt->isConfigurationChanged())
				conf_wgt->loadConfiguration();
		}

		emit s_configurationReverted();
		qApp->restoreOverrideCursor();
	}
	catch(Exception &)
	{}
}

void ConfigurationWidget::checkChangedConfiguration()
{
	for(auto &conf_wgt : confs_stw->findChildren<BaseConfigWidget *>())
	{
		if(conf_wgt->isConfigurationChanged())
		{
			int res = Messagebox::confirm(tr("Some configuration parameters were changed! How do you wish to proceed?"),
																		Messagebox::YesNoButtons, tr("Apply"), tr("Discard"));

			if(res == Messagebox::Accepted)
				applyConfiguration();
			else
				revertConfiguration();

			break;
		}
	}
}

void ConfigurationWidget::applyConfiguration()
{
	BaseConfigWidget *conf_wgt = nullptr;
	bool curr_escape_comments = BaseObject::isEscapeComments();

	qApp->setOverrideCursor(Qt::WaitCursor);

	for(int i = GeneralConfWgt; i <= SnippetsConfWgt; i++)
	{
		conf_wgt = qobject_cast<BaseConfigWidget *>(confs_stw->widget(i));

		if(conf_wgt->isConfigurationChanged())
			conf_wgt->saveConfiguration();
	}

	general_conf->applyConfiguration();
	relationships_conf->applyConfiguration();

	emit s_configurationChanged();

	qApp->restoreOverrideCursor();
}

void ConfigurationWidget::loadConfiguration()
{
	BaseConfigWidget *config_wgt = nullptr;

	for(int i=GeneralConfWgt; i <= PluginsConfWgt; i++)
	{
		try
		{
			config_wgt = qobject_cast<BaseConfigWidget *>(confs_stw->widget(i));
			config_wgt->loadConfiguration();
		}
		catch(Exception &e)
		{
			if(e.getErrorCode()==ErrorCode::PluginsNotLoaded)
			{
				Messagebox::error(e, __PRETTY_FUNCTION__, __FILE__, __LINE__);
			}
			else
			{
				Messagebox msg_box;				
				Exception ex = Exception(Exception::getErrorMessage(ErrorCode::ConfigurationNotLoaded).arg(e.getExtraInfo()),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);

				msg_box.show(ex, QString("%1 %2").arg(ex.getErrorMessage(), tr("In some cases restore the default settings related to it may solve the problem. Would like to do that?")),
										 Messagebox::AlertIcon, Messagebox::YesNoButtons, tr("Restore"), "", "", GuiUtilsNs::getIconPath("refresh"));

				if(msg_box.isAccepted())
					config_wgt->restoreDefaults();
			}
		}
	}
}

void ConfigurationWidget::restoreDefaults()
{
	int res = Messagebox::confirm(tr("Any modification made until now in the current section will be lost! Do you really want to restore default settings?"));

	if(Messagebox::isAccepted(res))
		qobject_cast<BaseConfigWidget *>(confs_stw->currentWidget())->restoreDefaults();
}

/*BaseConfigWidget *ConfigurationWidget::getConfigurationWidget(unsigned idx)
{
	if(idx >= static_cast<unsigned>(confs_stw->count()))
		return nullptr;

	return qobject_cast<BaseConfigWidget *>(confs_stw->widget(static_cast<unsigned>(idx)));
} */

/* template<class Widget>
Widget *ConfigurationWidget::getConfigurationWidget(unsigned int idx)
{
	if(idx >= static_cast<unsigned>(confs_stw->count()))
		return nullptr;

	return qobject_cast<Widget *>(confs_stw->widget(static_cast<unsigned>(idx)));
} */
