#include <iostream>
#include <QAction>

#include "searchtab.h"
#include "ui_searchtab.h"
#include "configtab.h"


using ct = configtab;

SearchTab::SearchTab(QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("");

}
SearchTab::SearchTab(QVector<PyrosFile*> &files,QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("Loading...");
    ui->file_view->set_files_from_vector(files);
}
SearchTab::SearchTab(QVector<QByteArray> &tags,QTabWidget *parent) :
    Tab(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("Loading...");
    ui->search_tags->add_tags(tags);
    ui->file_view->search(tags);
    create_title(tags);
}

SearchTab::~SearchTab(){
    delete ui;
}

void SearchTab::init()
{
    ui->setupUi(this);
    set_title("Search");

    ui->file_tags->setTagType(PYROS_FILE_RELATIONSHIP);
    ui->search_tags->setTagType(PYROS_SEARCH_RELATIONSHIP);
    ui->searchbar->set_relation_type(PYROS_SEARCH_RELATIONSHIP);

    ui->file_tags->append_search_options_to_contenxt_menu();

    QAction *refresh_bind  = ct::create_binding(ct::KEY_REFRESH,"refresh",this);
    QAction *search_bind   = ct::create_binding(ct::KEY_FOCUS_SEARCH_BAR,"insert search",this);
    QAction *tagbar_bind   = ct::create_binding(ct::KEY_FOCUS_TAG_BAR,"insert tagbar",this);
    QAction *invert_bind   = ct::create_binding(ct::KEY_INVERT_SELECTION,"invert tagbox",this);
    QAction *fileview_bind = ct::create_binding(ct::KEY_FOCUS_FILE_GRID,"select fileview",this);

    QHeaderView *vheader = ui->file_view->verticalHeader();
    QHeaderView *hheader = ui->file_view->horizontalHeader();

    vheader->setMinimumSectionSize(ct::setting_value(ct::THUMBNAIL_SIZE).toInt());
    vheader->setDefaultSectionSize(ct::setting_value(ct::THUMBNAIL_SIZE).toInt());

    hheader->setMinimumSectionSize(ct::setting_value(ct::THUMBNAIL_SIZE).toInt());
    hheader->setDefaultSectionSize(ct::setting_value(ct::THUMBNAIL_SIZE).toInt());

    connect(refresh_bind, &QAction::triggered,ui->file_view, &FileView::refresh);
    connect(search_bind,  &QAction::triggered,this, &SearchTab::select_search_bar);
    connect(tagbar_bind,  &QAction::triggered,this, &SearchTab::select_tag_bar);
    connect(invert_bind,  &QAction::triggered,ui->file_view, &FileView::invertSelection);
    connect(fileview_bind,&QAction::triggered,this, &SearchTab::select_file_view);

    connect(ui->search_button, &QPushButton::released,ui->searchbar, &TagLineEdit::process_tag);

    connect(ui->searchbar, &TagLineEdit::reset,this, &SearchTab::clear);
    connect(ui->searchbar, &TagLineEdit::tag_entered,ui->file_view, &FileView::search);
    connect(ui->searchbar, &TagLineEdit::tag_entered,ui->search_tags, &TagView::add_tags);
    connect(ui->searchbar, &TagLineEdit::tag_entered,this, &SearchTab::create_title);

    connect(ui->search_tags, &TagView::removeTag,ui->file_view, &FileView::remove_tag_from_search);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_view, &FileView::add_tag);
    connect(ui->tag_bar, &TagLineEdit::textChanged,ui->file_tags, &TagView::highlight_similar_tags);

    connect(ui->file_tags,&TagView::removeTag,ui->file_view,&FileView::remove_tag);
    connect(ui->file_tags,&TagView::add_tag_to_current_search,ui->search_tags,&TagView::add_tags);
    connect(ui->file_tags,&TagView::add_tag_to_current_search,ui->file_view,&FileView::search);

    connect(ui->file_view, &FileView::new_duplicate_selector_tab, this, &SearchTab::new_duplicate_selector_tab);

    connect(ui->file_view, &FileView::activated, this, &SearchTab::create_new_viewer_tab);
    connect(ui->file_view, &FileView::new_files, this, &SearchTab::set_file_count);
    connect(ui->file_view->selectionModel(), &QItemSelectionModel::selectionChanged ,this, &SearchTab::set_bottom_bar);


    connect(ui->file_tags,&TagView::new_search_with_selected_tags,this,&SearchTab::create_new_search_with_tags);
    connect(ui->search_tags,&TagView::new_search_with_selected_tags,this,&SearchTab::create_new_search_with_tags);
}

void SearchTab::set_loading_screen(QString text)
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->loading_status->setText(text);
}

void SearchTab::show_results()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void SearchTab::create_title(QVector<QByteArray> tags){
    set_loading_screen("Loading...");
    QString str = tags.first();
    set_title(str);
}

void SearchTab::set_file_count(QVector<FileModel::file_item> files)
{
    if (files.length() == 0){
        set_loading_screen("No Results");
        ui->data_file_count->setText("0");
    } else if (!ui->loading_status->text().isEmpty()){
        show_results();
        ui->data_file_count->setText(QString::number(files.length()));
    } else {
        clear();
    }
}

void SearchTab::create_new_viewer_tab(const QModelIndex &inital_index)
{
    QItemSelectionModel *select = ui->file_view->selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;

    PyrosFile *file;
    QVector<PyrosFile*> files;
    int inital_position = 0;

    if (indexes.length() == 1){
        PyrosFile *inital_file = ui->file_view->file(inital_index);
        int i = 0;
        files = ui->file_view->files();
        foreach(PyrosFile *pFile,files){
            if (inital_file == pFile)
                inital_position = i;
            i++;
        }

    } else if (indexes.length() < 1){
        return;
    } else {
        foreach(index, indexes) {
            file = ui->file_view->file(index);
            if (file != nullptr)
                files.push_back(file);

        }
    }

    emit create_viewer_tab(files,inital_position);
}


void SearchTab::clear()
{

    set_loading_screen("");
    ui->file_tags->clear();

    ui->search_tags->clear();
    ui->file_view->clear_tags();
    clear_file_data();
    ui->data_file_count->clear();
    set_title("Search");
}

void SearchTab::clear_file_data()
{
    ui->data_current_file->clear();
    ui->data_file_mime->clear();
    ui->data_file_size->clear();
    ui->data_file_time->clear();
}

void SearchTab::select_tag_bar()
{
    ui->tag_bar->setFocus(Qt::OtherFocusReason);
}

void SearchTab::select_search_bar()
{
    ui->searchbar->setFocus(Qt::OtherFocusReason);
}

void SearchTab::select_file_view()
{
    ui->file_view->setFocus(Qt::OtherFocusReason);
}

void SearchTab::set_bottom_bar(const QItemSelection &selected, const QItemSelection &deselected){
    Q_UNUSED(deselected);
    Q_UNUSED(selected);

    QItemSelectionModel *select = ui->file_view->selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    quint64 total_file_size = 0;
    quint64 earliest_timestamp = -1;
    quint64 latest_timestamp = 0;


    if (indexes.count() == 0){
        ui->file_tags->clear();
        clear_file_data();
        return;
    }

    QModelIndex  last_file = indexes.last();

    if (indexes.count() > 1){
        QString multi_select_styelsheet = "QLabel {color : cyan; }";
        ui->data_file_time->setStyleSheet(multi_select_styelsheet);
        ui->data_file_size->setStyleSheet(multi_select_styelsheet);
        ui->data_current_file->setStyleSheet(multi_select_styelsheet);
        ui->data_current_file->setText(QString::number(indexes.count()) +" /");
    } else {
        ui->data_file_time->setStyleSheet("");
        ui->data_file_size->setStyleSheet("");
        ui->data_current_file->setStyleSheet("");
        ui->data_current_file->setText(QString::number(ui->file_view->file_model->indexToNum(last_file)+1) +" /");
    }

    PyrosFile *last_pFile = nullptr;
    for	(int i = 0; i < indexes.length(); i++){
        QModelIndex  last_valid_file = indexes.at(i);
        if (ui->file_view->file(last_valid_file) != nullptr){
            last_pFile = ui->file_view->file(last_valid_file);
            total_file_size += last_pFile->file_size;

            if (earliest_timestamp >= last_pFile->import_time)
                earliest_timestamp = last_pFile->import_time;
            if (latest_timestamp <= last_pFile->import_time)
                latest_timestamp = last_pFile->import_time;

        }
    }

    if (last_pFile == nullptr){
        ui->file_tags->clear();
        clear_file_data();
        return;
    }

    QSettings settings;
    QString timestamp_earlyest_string;
    QString timestamp_latest_string;
    QDateTime timestamp;
    QLocale locale = this->locale();

    timestamp.setTime_t(earliest_timestamp);
    timestamp_earlyest_string = timestamp.toString(ct::setting_value(ct::TIMESTAMP).toString());

    timestamp.setTime_t(latest_timestamp);
    timestamp_latest_string = timestamp.toString(ct::setting_value(ct::TIMESTAMP).toString());

    if (timestamp_earlyest_string == timestamp_latest_string)
        ui->data_file_time->setText(timestamp_earlyest_string);
    else
        ui->data_file_time->setText(timestamp_earlyest_string+" - "+timestamp_latest_string);


    ui->data_file_mime->setText(last_pFile->mime);
    ui->data_file_size->setText(locale.formattedDataSize(total_file_size));
    ui->file_tags->setTagsFromFile(last_pFile);



}
