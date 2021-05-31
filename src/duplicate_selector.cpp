#include <QAction>
#include <QButtonGroup>

#include <pyros.h>

#include "MediaViewer/mediaviewer.h"

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

    connect(ui->duplicate_radio,&QRadioButton::clicked,this,&duplicate_selector::duplicate_checked);
    connect(ui->not_duplicate_radio,&QRadioButton::clicked,this,&duplicate_selector::not_duplicate_checked);
    connect(ui->superior_file_radio,&QRadioButton::clicked,this,&duplicate_selector::superior_checked);

    connect(ui->apply_button,&QPushButton::clicked,this,&duplicate_selector::apply);

    connect(this,&duplicate_selector::hide_files,ui->mediaviewer,&MediaViewer::hide_files);
    connect(ui->mediaviewer,&MediaViewer::position_changed,this,&duplicate_selector::set_position);
    connect(ui->mediaviewer,&MediaViewer::file_removed_at,this,&duplicate_selector::file_hidden);

    for(int i = 0;i < files.length(); i++)
        file_statuses.append(NONE);

    ui->mediaviewer->bind_keys(this);
    ui->mediaviewer->set_files(files);
    update_file();

}

duplicate_selector::~duplicate_selector()
{
    delete ui;
}

void duplicate_selector::update_file()
{
    if (file_statuses.length() <= 1){
        delete_self();
        return;
    }

    ui->duplicate_radio->setAutoExclusive(false);
    ui->duplicate_radio->setChecked(false);
    ui->duplicate_radio->setAutoExclusive(true);

    ui->not_duplicate_radio->setAutoExclusive(false);
    ui->not_duplicate_radio->setChecked(false);
    ui->not_duplicate_radio->setAutoExclusive(true);

    ui->superior_file_radio->setAutoExclusive(false);
    ui->superior_file_radio->setChecked(false);
    ui->superior_file_radio->setAutoExclusive(true);

    switch (file_statuses[position]){
    case DUPLICATE:
        ui->duplicate_radio->setChecked(true);
        break;
    case NOT_DUPLICATE:
        ui->not_duplicate_radio->setChecked(true);
        break;
    case SUPERIOR:
        ui->superior_file_radio->setChecked(true);
        break;
    case NONE:
        break;
    }

}

void duplicate_selector::duplicate_checked()
{
    file_statuses[position] = DUPLICATE;
    update_radio_buttons();
}

void duplicate_selector::superior_checked()
{
    file_statuses[position] = SUPERIOR;
    update_radio_buttons();
}

void duplicate_selector::not_duplicate_checked()
{
    file_statuses[position] = NOT_DUPLICATE;
    update_radio_buttons();
}

void duplicate_selector::update_radio_buttons()
{
    bool all_files_set = true;
    bool superior_set = false;

    foreach(DUPLICATE_STATUS status, file_statuses){
        if (status == NONE)
            all_files_set = false;
        else if (status == SUPERIOR)
            superior_set = true;
    }

    if (file_statuses.length() <= 1)
        all_files_set = false;

    ui->apply_button->setEnabled(all_files_set && superior_set);
    ui->superior_file_radio->setEnabled(!superior_set);

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

    if (duplicates.length() >= 1){
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
    position = pos;
    update_file();
}
