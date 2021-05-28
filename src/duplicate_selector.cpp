#include <QAction>
#include <QButtonGroup>

#include <pyros.h>

#include "MediaViewer/Overlay/overlay.h"
#include "MediaViewer/Overlay/overlay_text.h"
#include "MediaViewer/Overlay/overlay_button.h"
#include "MediaViewer/mediaviewer.h"

#include "duplicate_selector.h"
#include "ui_duplicate_selector.h"
#include "configtab.h"
#include "pyrosdb.h"


using ct = configtab;

duplicate_selector::duplicate_selector(QVector<PyrosFile*> files,QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::duplicate_selector),
    m_files(files)
{
    ui->setupUi(this);
    set_title("Duplicate Selector");

    QAction *next_bind = ct::create_binding(ct::KEY_NEXT_FILE,"Next file",this);
    QAction *prev_bind = ct::create_binding(ct::KEY_PREV_FILE,"Previous file",this);

    connect(next_bind,   &QAction::triggered,this, &duplicate_selector::next_file);
    connect(prev_bind,   &QAction::triggered,this, &duplicate_selector::prev_file);

    connect(ui->duplicate_radio,&QRadioButton::clicked,this,&duplicate_selector::duplicate_checked);
    connect(ui->not_duplicate_radio,&QRadioButton::clicked,this,&duplicate_selector::not_duplicate_checked);
    connect(ui->superior_file_radio,&QRadioButton::clicked,this,&duplicate_selector::superior_checked);

    connect(ui->apply_button,&QPushButton::clicked,this,&duplicate_selector::apply);

    Overlay_Text *overlay_file_count = new Overlay_Text("File count",ui->mediaviewer->overlay);

    Overlay_Button *overlay_next_button = new Overlay_Button(":/data/icons/right_arrow.png",nullptr,"Next file",ui->mediaviewer->overlay);
    Overlay_Button *overlay_prev_button = new Overlay_Button(":/data/icons/left_arrow.png",nullptr,"Prev file",ui->mediaviewer->overlay);

    ui->mediaviewer->overlay->main_bar.widgets.prepend(overlay_next_button);
    ui->mediaviewer->overlay->main_bar.widgets.prepend(overlay_prev_button);

    ui->mediaviewer->overlay->main_bar.widgets.append(overlay_file_count);


    connect(overlay_next_button,&Overlay_Button::clicked,this,&duplicate_selector::next_file);
    connect(overlay_prev_button,&Overlay_Button::clicked,this,&duplicate_selector::prev_file);

    connect(this,&duplicate_selector::update_file_count,overlay_file_count,&Overlay_Text::set_text);

    for(int i = 0;i < m_files.length(); i++)
        file_statuses.append(NONE);

    ui->mediaviewer->bind_keys(this);
    update_file();

}

duplicate_selector::~duplicate_selector()
{
    foreach(PyrosFile *file,m_files)
        Pyros_Close_File(file);

    delete ui;
}

void duplicate_selector::update_file()
{
    if (m_files.length() <= 1){
        ui->mediaviewer->set_file(nullptr);
        delete_self();
        return;
    }
    PyrosFile *file = m_files.at(file_position);

    emit update_file_count(QString::number(file_position+1)+"/"+QString::number(m_files.count()));
    ui->mediaviewer->set_file(file);

    /*ui->cbz_buttons->setVisible(ui->mediaviewer->is_multipaged());
    ui->image_buttons->setVisible(ui->mediaviewer->is_resizable());

    ui->file_tags->setTagsFromFile(m_pFile);*/
    ui->duplicate_radio->setAutoExclusive(false);
    ui->duplicate_radio->setChecked(false);
    ui->duplicate_radio->setAutoExclusive(true);

    ui->not_duplicate_radio->setAutoExclusive(false);
    ui->not_duplicate_radio->setChecked(false);
    ui->not_duplicate_radio->setAutoExclusive(true);

    ui->superior_file_radio->setAutoExclusive(false);
    ui->superior_file_radio->setChecked(false);
    ui->superior_file_radio->setAutoExclusive(true);

    switch (file_statuses[file_position]){
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

void duplicate_selector::next_file()
{
    if (file_position+1 < m_files.size()){
        file_position++;
        update_file();
    }
}

void duplicate_selector::prev_file()
{
    if (file_position-1 >= 0){
        file_position--;
        update_file();
    }
}


void duplicate_selector::duplicate_checked()
{
    file_statuses[file_position] = DUPLICATE;
    update_radio_buttons();
}

void duplicate_selector::superior_checked()
{
    file_statuses[file_position] = SUPERIOR;
    update_radio_buttons();
}

void duplicate_selector::not_duplicate_checked()
{
    file_statuses[file_position] = NOT_DUPLICATE;
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

    if (m_files.length() <= 1)
        all_files_set = false;

    ui->apply_button->setEnabled(all_files_set && superior_set);
    ui->superior_file_radio->setEnabled(!superior_set);

}

void duplicate_selector::apply()
{
    QByteArray superior_file;
    QVector<QByteArray> duplicates;
    PyrosTC *ptc = PyrosTC::get();

    for(int i = 0; i < m_files.length(); i++){
        if (file_statuses[i] == SUPERIOR)
                superior_file = m_files[i]->hash;
        else if (file_statuses[i] == DUPLICATE)
                duplicates.append(m_files[i]->hash);
    }

    if (duplicates.length() >= 1){
        ptc->merge_files(superior_file,duplicates);
        emit files_removed(duplicates);
    }
    delete_self();

}

void duplicate_selector::hide_files(QVector<QByteArray> hashes){
    for (int i  = m_files.length()-1;i >= 0;i--) {
        PyrosFile *file = m_files.at(i);
        if (file == nullptr)
            continue;

        for (int j  = hashes.length()-1;j >= 0;j--) {
            if (!hashes.at(j).compare(file->hash)){
                m_files.removeAt(i);
                file_statuses.removeAt(i);
                if (file_position >= m_files.size())
                    file_position--;
                if (file_position < 0)
                    file_position = 0;
                hashes.removeAt(j);
            }
        }

    }

    update_radio_buttons();
    update_file();

}
