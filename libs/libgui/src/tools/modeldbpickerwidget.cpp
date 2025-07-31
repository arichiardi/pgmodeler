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

ModelDBPickerWidget::ModelDBPickerWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setPickMode(PickDatabase);
}

ModelDBPickerWidget::~ModelDBPickerWidget()
{

}

void ModelDBPickerWidget::setPickMode(PickMode pick_mode)
{
	model_ctrl_wgt->setVisible(pick_mode == PickModel);
	db_ctrl_wgt->setVisible(pick_mode == PickDatabase);
}
