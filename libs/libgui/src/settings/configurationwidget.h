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
\class SettingsWidget
\brief Reunites in a single form all available configuration widgets.
*/

#ifndef CONFIGURATION_WIDGET_H
#define CONFIGURATION_WIDGET_H

#include "ui_settingswidget.h"
#include "appearanceconfigwidget.h"
#include "generalconfigwidget.h"
#include "connectionsconfigwidget.h"
#include "pluginsconfigwidget.h"
#include "relationshipconfigwidget.h"
#include "snippetsconfigwidget.h"

class __libgui SettingsWidget: public QWidget, public Ui::SettingsWidget {
	Q_OBJECT

	private:	
		GeneralConfigWidget *general_conf;
		AppearanceConfigWidget *appearance_conf;
		ConnectionsConfigWidget *connections_conf;
		RelationshipConfigWidget *relationships_conf;
		SnippetsConfigWidget *snippets_conf;
		PluginsConfigWidget *plugins_conf;
		
		void hideEvent(QHideEvent *event);
		void showEvent(QShowEvent *);
		
	public:
		enum ConfWidgetsId {
			GeneralConfWgt,
			AppearanceConfWgt,
			RelationshipsConfWgt,
			ConnectionsConfWgt,
			SnippetsConfWgt,
			PluginsConfWgt
		};
		
		SettingsWidget(QWidget * parent = nullptr);
		virtual ~SettingsWidget();
		
		[[deprecated("Use template method getSettingsWidget<Class>() instead!")]]
		BaseConfigWidget *getConfigurationWidget(unsigned idx);

		template<class Widget>
		Widget *getSettingsWidget(unsigned idx);
		
	public slots:
		void applyConfiguration();
		void loadConfiguration();
		void reject();
		
	private slots:
		void restoreDefaults();
		void changeCurrentView();

	signals:
		void s_invalidateModelsRequested();
};

#endif
