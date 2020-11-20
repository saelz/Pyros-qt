#include <iostream>
#include <QAction>

#include "searchtab.h"
#include "ui_searchtab.h"

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
    create_title(tags);
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

    ui->search_tags->setTagType(PYROS_TAG_EXT);

    QAction *refresh_bind = new QAction("refresh",this);
    QAction *search_bind = new QAction("insert search",this);
    QAction *tagbar_bind = new QAction("insert tagbar",this);
    QAction *invert_bind = new QAction("invert tagbox",this);
    QAction *fileview_bind = new QAction("select fileview",this);

    refresh_bind->setShortcut(QKeySequence("CTRL+r"));
    search_bind->setShortcut(QKeySequence("a"));
    tagbar_bind->setShortcut(QKeySequence("i"));
    invert_bind->setShortcut(QKeySequence("SHIFT+i"));
    fileview_bind->setShortcut(QKeySequence("CTRL+f"));

    connect(refresh_bind, &QAction::triggered,ui->file_view, &FileView::refresh);
    connect(search_bind,  &QAction::triggered,this, &SearchTab::select_search_bar);
    connect(tagbar_bind,  &QAction::triggered,this, &SearchTab::select_tag_bar);
    connect(invert_bind,  &QAction::triggered,ui->file_view, &FileView::invertSelection);
    connect(fileview_bind,&QAction::triggered,this, &SearchTab::select_file_view);

    addAction(refresh_bind);
    addAction(search_bind);
    addAction(tagbar_bind);
    addAction(invert_bind);
    addAction(fileview_bind);

    connect(ui->search_button, &QPushButton::released,ui->searchbar, &TagLineEdit::process_tag);

    connect(ui->searchbar, &TagLineEdit::reset,this, &SearchTab::clear);
    connect(ui->searchbar, &TagLineEdit::tag_entered,ui->file_view, &FileView::search);
    connect(ui->searchbar, &TagLineEdit::tag_entered,ui->search_tags, &TagView::add_tags);
    connect(ui->searchbar, &TagLineEdit::tag_entered,this, &SearchTab::create_title);

    connect(ui->search_tags, &TagView::removeTag,ui->file_view, &FileView::remove_tag_from_search);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_view, &FileView::add_tag);

    connect(ui->file_tags,&TagView::removeTag,ui->file_view,&FileView::remove_tag);

    connect(ui->file_view, &FileView::activated, this, &SearchTab::create_new_viewer_tab);
    connect(ui->file_view, &FileView::new_files, this, &SearchTab::set_file_count);
    connect(ui->file_view->selectionModel(), &QItemSelectionModel::currentChanged ,this, &SearchTab::set_tag_view);
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


void SearchTab::set_tag_view(const QModelIndex &current, const QModelIndex &previous)
{

    Q_UNUSED(previous)
    if (current.isValid()){
        PyrosFile *pFile = ui->file_view->file(current);
        ui->file_tags->setTagsFromFile(pFile);

        if (pFile != nullptr){
            QSettings settings;
            QDateTime timestamp;
            QLocale locale = this->locale();
            timestamp.setTime_t(pFile->import_time);

            ui->data_file_mime->setText(pFile->mime);
            ui->data_file_size->setText(locale.formattedDataSize(pFile->file_size));
            ui->data_file_time->setText(timestamp.toString(settings.value("timestamp_format","MM/dd/yy").toString()));
        }
    } else{
        ui->file_tags->clear();
        clear_file_data();
    }


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


    if (indexes.count() == 0)
        return;

    QModelIndex  index = indexes.last();

    if (indexes.count() > 1){
        ui->data_current_file->setStyleSheet("QLabel {color : cyan; }");
        ui->data_current_file->setText(QString::number(indexes.count()) +" /");
    } else {
        ui->data_current_file->setStyleSheet("");
        ui->data_current_file->setText(QString::number(ui->file_view->file_model->indexToNum(index)+1) +" /");
    }

}
