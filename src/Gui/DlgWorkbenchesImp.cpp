/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Author: Przemo Firszt <przemo@firszt.eu>                              *
 *   Based on DlgToolbars.cpp file                                         *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"
#ifndef _PreComp_
# include <QInputDialog>
#endif

#include "DlgWorkbenchesImp.h"
#include "Application.h"
#include "BitmapFactory.h"
#include "Command.h"
#include "MainWindow.h"
#include "Widgets.h"
#include "Workbench.h"
#include "WorkbenchManager.h"
#include "QListWidgetCustom.h"

using namespace Gui::Dialog;

const QString DlgWorkbenchesImp::all_workbenches = QString::fromAscii("ALL");

/* TRANSLATOR Gui::Dialog::DlgWorkbenchesImp */

DlgWorkbenchesImp::DlgWorkbenchesImp(QWidget* parent)
    : CustomizeActionPage(parent)
{
    this->setupUi(this);
    set_lw_properties(lw_enabled_workbenches);
    set_lw_properties(lw_disabled_workbenches);
    const QString lw_disabled_name = QString::fromAscii("disabled workbenches");
    lw_disabled_workbenches->setAccessibleName(lw_disabled_name);
    lw_disabled_workbenches->setSortingEnabled(true);

    QStringList enabled_wbs_list = load_enabled_workbenches();
    QStringList disabled_wbs_list = load_disabled_workbenches();
    QStringList workbenches = Application::Instance->workbenches();

    for (QStringList::Iterator it = enabled_wbs_list.begin(); it != enabled_wbs_list.end(); ++it) {
        if (workbenches.contains(*it)) {
            add_workbench(lw_enabled_workbenches, *it);
        } else {
            qDebug() << "Ignoring unknown" << *it << "workbench found in user preferences.";
        }
    }
    for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
        if (disabled_wbs_list.contains(*it)){
            add_workbench(lw_disabled_workbenches, *it);
        } else if (!enabled_wbs_list.contains(*it)){
            qDebug() << "Adding unknown " << *it << "workbench.";
            add_workbench(lw_enabled_workbenches, *it);
        }
    }
    lw_enabled_workbenches->setCurrentRow(0);
    lw_disabled_workbenches->setCurrentRow(0);
}

/** Destroys the object and frees any allocated resources */
DlgWorkbenchesImp::~DlgWorkbenchesImp()
{
}

void DlgWorkbenchesImp::set_lw_properties(QListWidgetCustom *lw)
{
    lw->setDragDropMode(QAbstractItemView::DragDrop);
    lw->setSelectionMode(QAbstractItemView::SingleSelection);
    lw->viewport()->setAcceptDrops(true);
    lw->setDropIndicatorShown(true);
    lw->setDragEnabled(true);
    lw->setDefaultDropAction(Qt::MoveAction);
}

void DlgWorkbenchesImp::add_workbench(QListWidgetCustom *lw, const QString& it)
{
    QPixmap px = Application::Instance->workbenchIcon(it);
    QString mt = Application::Instance->workbenchMenuText(it);
    QListWidgetItem *wi = (new QListWidgetItem(QIcon(px), mt));
    wi->setData(Qt::UserRole, QVariant(it));
    lw->addItem(wi);
}

void DlgWorkbenchesImp::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        retranslateUi(this);
    }
    else {
        QWidget::changeEvent(e);
    }
}

void DlgWorkbenchesImp::hideEvent(QHideEvent * event)
{
    save_workbenches();
}

void DlgWorkbenchesImp::onAddMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::onRemoveMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::onModifyMacroAction(const QByteArray& macro)
{
}

void DlgWorkbenchesImp::move_workbench(QListWidgetCustom *lwc_dest,
                                       QListWidgetItem *wi)
{
    QListWidgetItem* item = wi->clone();
    lwc_dest->addItem(item);
    lwc_dest->setCurrentItem(item);
    delete wi;
}

void DlgWorkbenchesImp::on_add_to_enabled_workbenches_btn_clicked()
{
    QListWidgetItem* ci = lw_disabled_workbenches->currentItem();
    if (ci) {
        move_workbench(lw_enabled_workbenches, ci);
    }
}

void DlgWorkbenchesImp::on_remove_from_enabled_workbenches_btn_clicked()
{
    QListWidgetItem* ci = lw_enabled_workbenches->currentItem();
    if (ci) {
        move_workbench(lw_disabled_workbenches, ci);
    }
}

void DlgWorkbenchesImp::shift_workbench(bool up)
{
    int direction;
    if (up){
        direction = -1;
    } else {
        direction = 1;
    }
    if (lw_enabled_workbenches->currentItem()) {
        int index = lw_enabled_workbenches->currentRow();
        QListWidgetItem *item = lw_enabled_workbenches->takeItem(index);
        lw_enabled_workbenches->insertItem(index + direction, item);
        lw_enabled_workbenches->setCurrentRow(index + direction);
    }
}

void DlgWorkbenchesImp::on_shift_workbench_up_btn_clicked()
{
    shift_workbench(true);
}

void DlgWorkbenchesImp::on_shift_workbench_down_btn_clicked()
{
    shift_workbench(false);
}

void DlgWorkbenchesImp::on_sort_enabled_workbenches_btn_clicked()
{
    lw_enabled_workbenches->sortItems();
}

void DlgWorkbenchesImp::on_add_all_to_enabled_workbenches_btn_clicked()
{
    while (lw_disabled_workbenches->count() > 0) {
        QListWidgetItem* item = lw_disabled_workbenches->item(0);
        move_workbench(lw_enabled_workbenches, item);
    }
}

QStringList DlgWorkbenchesImp::load_enabled_workbenches()
{
    QString enabled_wbs;
    QStringList enabled_wbs_list;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    enabled_wbs = QString::fromStdString(hGrp->GetASCII("Enabled", all_workbenches.toStdString().c_str()).c_str());
    enabled_wbs_list = enabled_wbs.split(QLatin1String(","), QString::SkipEmptyParts);

    if (enabled_wbs_list.at(0) == all_workbenches) {
        enabled_wbs_list.removeFirst();
        QStringList workbenches = Application::Instance->workbenches();
        for (QStringList::Iterator it = workbenches.begin(); it != workbenches.end(); ++it) {
            enabled_wbs_list.append(*it);
        }
        enabled_wbs_list.sort();
    }
    return enabled_wbs_list;
}

QStringList DlgWorkbenchesImp::load_disabled_workbenches()
{
    QString disabled_wbs;
    QStringList disabled_wbs_list;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    disabled_wbs = QString::fromStdString(hGrp->GetASCII("Disabled", ""));
    disabled_wbs_list = disabled_wbs.split(QLatin1String(","), QString::SkipEmptyParts);

    return disabled_wbs_list;
}

void DlgWorkbenchesImp::save_workbenches()
{
    QString enabled_wbs;
    QString disabled_wbs;
    ParameterGrp::handle hGrp;

    hGrp = App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Workbenches");
    hGrp->Clear();

    if (lw_enabled_workbenches->count() == 0) {
        enabled_wbs.append(QString::fromAscii("NoneWorkbench"));
    } else {
        for (int i = 0; i < lw_enabled_workbenches->count(); i++) {
            QVariant item_data = lw_enabled_workbenches->item(i)->data(Qt::UserRole);
            QString name = item_data.toString();
            enabled_wbs.append(name + QString::fromAscii(","));
        }
    }
    hGrp->SetASCII("Enabled", enabled_wbs.toAscii());

    for (int i = 0; i < lw_disabled_workbenches->count(); i++) {
        QVariant item_data = lw_disabled_workbenches->item(i)->data(Qt::UserRole);
        QString name = item_data.toString();
        disabled_wbs.append(name + QString::fromAscii(","));
    }
    hGrp->SetASCII("Disabled", disabled_wbs.toAscii());
}

#include "moc_DlgWorkbenchesImp.cpp"

