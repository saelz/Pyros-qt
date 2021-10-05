#include <QAction>
#include <QButtonGroup>

#include <pyros.h>

#include "MediaViewer/mediaviewer.h"
#include "MediaViewer/Overlay/overlay.h"
#include "MediaViewer/Overlay/overlay_combo_box.h"

#include "duplicate_selector.h"
#include "ui_duplicate_selector.h"
#include "configtab.h"
#include "pyrosdb.h"


using ct = configtab;

duplicate_selector::duplicate_selector(QVector<PyrosFile*> files,QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::duplicate_selector)
{
    ui->setupUi(this);
    set_title("Duplicate Selector");

    connect(this,&duplicate_selector::hide_files,ui->mediaviewer,&MediaViewer::hide_files);
    connect(ui->mediaviewer,&MediaViewer::position_changed,this,&duplicate_selector::set_position);
    connect(ui->mediaviewer,&MediaViewer::file_removed_at,this,&duplicate_selector::file_hidden);

    for(int i = 0;i < files.length(); i++)
        file_statuses.append(NOT_DUPLICATE);

    ui->mediaviewer->bind_keys(this);
    ui->mediaviewer->set_files(files);

    Overlay_Combo_Box *dupe_combo_box = new Overlay_Combo_Box(nullptr,"Duplicate Status",ui->mediaviewer->overlay);
    Overlay_Button *apply_button = new Overlay_Button(":/data/icons/checkmark.png",nullptr,QString(),ui->mediaviewer->overlay,false,"","Apply");

    dupe_combo_box->add_entry("Not Duplicate",NOT_DUPLICATE);
    dupe_combo_box->add_entry("Duplicate",DUPLICATE);
    dupe_combo_box->add_entry("Superior File",SUPERIOR);
    dupe_combo_box->set_entry(NOT_DUPLICATE);

    connect(this,&duplicate_selector::set_dupe_combo_box_status,dupe_combo_box,&Overlay_Combo_Box::set_entry);
    connect(dupe_combo_box,&Overlay_Combo_Box::entry_changed,this,&duplicate_selector::entry_changed);
    connect(apply_button,&Overlay_Button::clicked,this,&duplicate_selector::apply);
    connect(this,&duplicate_selector::set_apply_button_enabled,apply_button,&Overlay_Button::set_enabled);
    connect(this,&duplicate_selector::hide_combo_box_entry,dupe_combo_box,&Overlay_Combo_Box::set_entry_hidden_state);

    ui->mediaviewer->overlay->main_bar.widgets.prepend(dupe_combo_box);
    ui->mediaviewer->overlay->main_bar.widgets.append(apply_button);

    check_file_status();
}

duplicate_selector::~duplicate_selector()
{
    delete ui;
}

void duplicate_selector::apply()
{
    QByteArray superior_file;
    QVector<QByteArray> duplicates;
    PyrosTC *ptc = PyrosTC::get();

    for(int i = 0; i < file_statuses.length(); i++){
        const PyrosFile *file = ui->mediaviewer->file_at(i);

        if (file == nullptr)
            continue;

        if (file_statuses[i] == SUPERIOR)
                superior_file = file->hash;
        else if (file_statuses[i] == DUPLICATE)
                duplicates.append(file->hash);
    }

    if (duplicates.length() >= 1 && !superior_file.isEmpty()){
        ptc->merge_files(superior_file,duplicates);
        emit files_removed(duplicates);
    }
    delete_self();

}

void duplicate_selector::file_hidden(int pos)
{
    file_statuses.removeAt(pos);
}

void duplicate_selector::set_position(int pos)
{
    if (file_statuses.length() <= 1){
        delete_self();
        return;
    }
    position = pos;
    emit set_dupe_combo_box_status(file_statuses[pos]);
}

void duplicate_selector::entry_changed(int value)
{
    file_statuses[position] = DUPLICATE_STATUS(value);
    check_file_status();
}

void duplicate_selector::check_file_status()
{
    bool superior_file_found = false;
    bool duplicate_file_found = false;
    foreach(DUPLICATE_STATUS status,file_statuses){
        if (status == SUPERIOR)
            superior_file_found = true;
        else if (status == DUPLICATE)
            duplicate_file_found = true;
    }

    if (superior_file_found)
        emit hide_combo_box_entry(SUPERIOR,true);
    else
        emit hide_combo_box_entry(SUPERIOR,false);

    emit set_apply_button_enabled(duplicate_file_found && superior_file_found);
}
