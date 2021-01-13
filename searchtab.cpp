#include <iostream>
#include <QAction>

#include "searchtab.h"
#include "ui_searchtab.h"
#include "configtab.h"


using ct = configtab;

SearchTab::SearchTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("");

}
SearchTab::SearchTab(QVector<PyrosFile*> &files,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("Loading...");
    ui->file_view->set_files_from_vector(files);
}
SearchTab::SearchTab(QVector<QByteArray> &tags,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchTab)
{
    init();
    set_loading_screen("Loading...");
    ui->search_tags->add_tags(tags);
    ui->file_view->search(tags);
}

SearchTab::~SearchTab()
{
    delete ui;
}

void SearchTab::init()
{
    ui->setupUi(this);

    ui->search_tags->setTagType(PYROS_SEARCH_RELATIONSHIP);
    ui->searchbar->set_relation_type(PYROS_SEARCH_RELATIONSHIP);

    ui->file_tags->append_search_options_to_contenxt_menu();

    QAction *refresh_bind  = ct::create_binding(ct::KEY_REFRESH,"refresh",this);
    QAction *search_bind   = ct::create_binding(ct::KEY_FOCUS_SEARCH_BAR,"insert search",this);
    QAction *tagbar_bind   = ct::create_binding(ct::KEY_FOCUS_TAG_BAR,"insert tagbar",this);
    QAction *invert_bind   = ct::create_binding(ct::KEY_INVERT_SELECTION,"invert tagbox",this);
    QAction *fileview_bind = ct::create_binding(ct::KEY_FOCUS_FILE_GRID,"select fileview",this);

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

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_view, &FileView::add_tag);

    connect(ui->file_tags,&TagView::removeTag,ui->file_view,&FileView::remove_tag);
    connect(ui->file_tags,&TagView::add_tag_to_current_search,ui->search_tags,&TagView::add_tags);
    connect(ui->file_tags,&TagView::add_tag_to_current_search,ui->file_view,&FileView::search);

    connect(ui->file_view, &FileView::files_removed, this, &SearchTab::file_deleted);
    connect(this, &SearchTab::hide_files_by_hash, ui->file_view,&FileView::hide_files_by_hash);

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
    emit set_title(str,this);
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
    emit set_title("Search",this);
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


    if (indexes.count() == 0){
        ui->file_tags->clear();
        clear_file_data();
        return;
    }

    QModelIndex  last_file = indexes.last();


    PyrosFile *last_pFile = nullptr;
    for	(int i = indexes.length()-1;i >= 0;i--){
        QModelIndex  last_valid_file = indexes.at(i);
        last_pFile = ui->file_view->file(last_valid_file);
        if (last_pFile != nullptr)
            break;
    }

    if (last_pFile != nullptr){
        QSettings settings;
        QDateTime timestamp;
        QLocale locale = this->locale();
        timestamp.setTime_t(last_pFile->import_time);

        ui->data_file_mime->setText(last_pFile->mime);
        ui->data_file_size->setText(locale.formattedDataSize(last_pFile->file_size));
        ui->data_file_time->setText(timestamp.toString(ct::setting_value(ct::TIMESTAMP).toString()));
        ui->file_tags->setTagsFromFile(last_pFile);
    } else {
        ui->file_tags->clear();
        clear_file_data();
    }

    if (indexes.count() > 1){
        ui->data_current_file->setStyleSheet("QLabel {color : cyan; }");
        ui->data_current_file->setText(QString::number(indexes.count()) +" /");
    } else {
        ui->data_current_file->setStyleSheet("");
        ui->data_current_file->setText(QString::number(ui->file_view->file_model->indexToNum(last_file)+1) +" /");


    }

}
