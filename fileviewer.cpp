#include "fileviewer.h"
#include "pyrosqt.h"
#include "pyrosdb.h"

#include "ui_fileviewer.h"

#include "tagtreemodel.h"
#include "zip_reader.h"

#include <QImageReader>
#include <QMovie>
#include <QBitmap>
#include <QComboBox>
#include <QResizeEvent>
#include <QSettings>
#include <QScrollBar>


FileViewer::Viewer::Viewer(QLabel *label) : m_label(label){}

void FileViewer::Viewer::resize(int width,int height,FileViewer::SCALE_TYPE scale)
{
    if (boundry_height != height ||
            boundry_width != width ||
            scale_type != scale){

        boundry_height = height;
        boundry_width = width;
        scale_type = scale;
        update_size();
    }
}


class Video_Viewer : public FileViewer::Viewer
{
    mpv_widget *m_mpv = nullptr;
public:
    Video_Viewer(mpv_widget *mpv): FileViewer::Viewer(nullptr),m_mpv(mpv){}

    ~Video_Viewer()
    {
        m_mpv->stop();
    }

    void set_file(char *path) override
    {
        m_mpv->set_file(path);

    }
};

class Image_Viewer : public FileViewer::Viewer
{
protected:
    QPixmap m_img;
    double zoom_level;
    double zoom_increment = .25;

public:
    Image_Viewer(QLabel *label): FileViewer::Viewer(label){
        m_label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        zoom_level = 1;
    }

    ~Image_Viewer() override
    {
        m_label->setPixmap(QPixmap());
    }

    void set_file(char *path) override
    {
        zoom_level = 1;
        m_img = QPixmap(path);
    }

    virtual QSize size(){
        return m_img.size();
    };

    virtual void set_size(QSize newsize){
        QPixmap scaledimg;
        scaledimg = m_img.scaled(newsize,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        m_label->setPixmap(scaledimg);
    };

    virtual void resize(int width, int height, FileViewer::SCALE_TYPE scale) override{
        if (scale_type != scale)
            zoom_level = 1;

        if (zoom_level != 1)
            return;

        FileViewer::Viewer::resize(width,height,scale);
    }

    void update_size() override
    {
        QSize newsize;

        newsize = size();
        switch (scale_type){
        case FileViewer::HEIGHT:
            newsize.scale(newsize.width()*zoom_level,
                  boundry_height*zoom_level,
                  Qt::KeepAspectRatio);
            break;
        case FileViewer::WIDTH:
            newsize.scale(boundry_width*zoom_level,
                  newsize.height()*zoom_level,
                  Qt::KeepAspectRatio);
            break;
        case FileViewer::BOTH:
            newsize.scale(boundry_width*zoom_level,
                  boundry_height*zoom_level,
                  Qt::KeepAspectRatio);
            break;
        case FileViewer::ORIGINAL:
            newsize.scale(newsize.width()*zoom_level,
                  newsize.height()*zoom_level,
                  Qt::KeepAspectRatio);
            break;
        }

        set_size(newsize);

    }

    void zoom_in() override
    {
        if ((zoom_level += zoom_increment) >= 3)
            zoom_level = 3;

        update_size();
    }

    void zoom_out() override
    {
        if ((zoom_level -= zoom_increment) <= 0)
            zoom_level = zoom_increment;

        update_size();
    }

    QString get_info() override
    {
        QSize imgsize = size();

        return QString::number(imgsize.width())+"x"+QString::number(imgsize.height());
    }
};

class Movie_Viewer : public Image_Viewer{
    QMovie *movie = nullptr;
    QSize orignal_size;
public:
    Movie_Viewer(QLabel *label) : Image_Viewer(label){

    }
    ~Movie_Viewer(){
        if (movie != nullptr)
            delete movie;
    }
    void set_file(char *path) override{
         movie = new QMovie(path);

        m_label->setMovie(movie);
        movie->start();
        orignal_size = movie->currentImage().size();

    }
    QSize size() override{
        return orignal_size;
    }

    void set_size(QSize newsize) override{
        if (movie == nullptr)
            return;
        movie->stop();
        movie->setScaledSize(newsize);
        movie->start();
    }

    QString get_info() override
    {
        if (movie == nullptr)
            return "";
        return "Frames:"+QString::number(movie->frameCount())+
                " "+Image_Viewer::get_info();
    }
};


class Cbz_Viewer : public Image_Viewer{
    zip_reader reader;
    int current_page = 0;
public:
    Cbz_Viewer(QLabel *label): Image_Viewer(label){}

    void set_file(char *path) override{
        reader.read_file(path);

        if (reader.isValid)
            read_page();
        else
            m_label->setText("error reading cbz file");
    }

    void read_page(){
        QByteArray data = reader.get_file_data(current_page);
        m_img.loadFromData(data);
        update_size();
    }

    void update_size() override{
        if (reader.isValid)
            Image_Viewer::update_size();
    }

    void next_page() override{
        if (!reader.isValid || current_page+1 >= reader.file_count())
            return;
        current_page++;
        read_page();
    }

    void prev_page() override{
        if (!reader.isValid || current_page <= 0)
            return;
        current_page--;
        read_page();
    }

    QString get_info() override
    {
        if (!reader.isValid)
            return "";

        return "Page:"+QString::number(current_page+1)+"/"+QString::number(reader.file_count())+
                " "+Image_Viewer::get_info();
    }
};

class Text_Viewer : public FileViewer::Viewer{
    int font_size = 12;
public:
    Text_Viewer(QLabel *label): FileViewer::Viewer(label)
    {
        QFont f("Arial", font_size);

        m_label->setFont(f);
        m_label->setWordWrap(true);
        m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    }

    ~Text_Viewer(){
        m_label->setText("");
    }

    void set_file(char *path) override
    {
        QFile file(path);
        QString line;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd())
                line.append(stream.readLine()+"\n");

            m_label->setText(line);
        }
        file.close();

    };

    void zoom_in() override
    {
        font_size++;
        QFont f("Arial", font_size);
        m_label->setFont(f);
    }

    void zoom_out() override
    {
        if ((font_size--) == 0)
            font_size = 1;
        QFont f("Arial", font_size);
        m_label->setFont(f);
    }

    QString get_info() override
    {
        QString count = m_label->text();

        return "Word Count:"+QString::number(count.count(' ')+1);
    }

};

class Unsupported_Viewer : public Text_Viewer{
public:
    Unsupported_Viewer(QLabel *label): Text_Viewer(label){}
    void set_file(char *path) override{
        Q_UNUSED(path)
        m_label->setText("Unsupported filetype");
    }

    QString get_info() override
    {
        return "";
    }
};



FileViewer::FileViewer(QVector<PyrosFile*> files,int inital_pos,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileViewer),
    position(inital_pos)
{

    ui->setupUi(this);

    foreach(PyrosFile *pFile,files){
        m_files.push_back(Pyros_Duplicate_File(pFile));
    }

    set_file();


    QAction *next_bind;
    QAction *prev_bind;
    QAction *insert_bind;
    QAction *delete_bind;
    QAction *zoom_in_bind;
    QAction *zoom_out_bind;

    next_bind = new QAction("Next File",this);
    prev_bind = new QAction("Prev File",this);
    insert_bind = new QAction("Insert",this);
    delete_bind = new QAction("delete",this);
    zoom_in_bind = new QAction("zoom in",this);
    zoom_out_bind = new QAction("zoom out",this);

    next_bind->setShortcut(QKeySequence("CTRL+n"));
    prev_bind->setShortcut(QKeySequence("CTRL+p"));
    insert_bind->setShortcut(QKeySequence("i"));
    delete_bind->setShortcut(QKeySequence("CTRL+del"));
    next_bind->setShortcut(QKeySequence("CTRL+n"));
    prev_bind->setShortcut(QKeySequence("CTRL+p"));
    zoom_in_bind->setShortcut(QKeySequence("CTRL++"));
    zoom_out_bind->setShortcut(QKeySequence("CTRL+-"));

    connect(next_bind,   &QAction::triggered,this, &FileViewer::next_file);
    connect(prev_bind,   &QAction::triggered,this, &FileViewer::prev_file);
    connect(insert_bind, &QAction::triggered,this, &FileViewer::select_tag_bar);
    connect(delete_bind, &QAction::triggered,this, &FileViewer::delete_file);
    connect(zoom_in_bind, &QAction::triggered,this, &FileViewer::zoom_in);
    connect(zoom_out_bind, &QAction::triggered,this, &FileViewer::zoom_out);

    //connect insert_bind
    addAction(next_bind);
    addAction(prev_bind);
    addAction(insert_bind);
    addAction(delete_bind);
    addAction(zoom_in_bind);
    addAction(zoom_out_bind);

    connect(ui->fit_combo_box, &QComboBox::currentTextChanged,this, &FileViewer::update_fit);

    connect(ui->next_button,   &QPushButton::released,this, &FileViewer::next_file);
    connect(ui->prev_button,   &QPushButton::released,this, &FileViewer::prev_file);
    connect(ui->delete_button, &QPushButton::released,this, &FileViewer::delete_file);

    connect(ui->zoom_in_button,  &QPushButton::released,this, &FileViewer::zoom_in);
    connect(ui->zoom_out_button, &QPushButton::released,this, &FileViewer::zoom_out);

    connect(ui->tag_bar, &TagLineEdit::tag_entered,ui->file_tags, &TagView::add_tags);
    connect(ui->tag_bar, &TagLineEdit::tag_entered,this, &FileViewer::add_tag);

    connect(ui->file_tags, &TagView::removeTag, this,&FileViewer::remove_tag);
    connect(ui->file_tags, &TagView::new_search_with_selected_tags,this, &FileViewer::new_search_with_selected_tags);

    connect(ui->cbz_prev_page, &QPushButton::released,this, &FileViewer::cbz_prev_page);
    connect(ui->cbz_next_page, &QPushButton::released,this, &FileViewer::cbz_next_page);

    connect(this,&QWidget::destroyed,ui->mpv_player,&mpv_widget::stop);

    ui->scrollArea->installEventFilter(this);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


FileViewer::~FileViewer()
{

    if (viewer != nullptr)
        delete viewer;

    foreach(PyrosFile *pFile,m_files) Pyros_Close_File(pFile);
    delete ui;
}

void FileViewer::select_tag_bar()
{
    ui->tag_bar->setFocus(Qt::OtherFocusReason);
}

void FileViewer::set_file()
{
    m_pFile = m_files.at(position);

    ui->cbz_buttons->setVisible(false);
    ui->image_buttons->setVisible(false);
    ui->stackedWidget->setCurrentIndex(0);

    if (viewer != nullptr){
        delete viewer;
        viewer = nullptr;
    }

    if (m_pFile == nullptr || m_files.count() <= 0)
        return;

    QSettings settings;
    QString filecount = QString::number(position+1)+"/"+QString::number(m_files.count());
    ui->file_count_label->setText(filecount);



    if (!strcmp(m_pFile->mime,"image/gif") &&
            !settings.value("treat_gifs_as_video",false).toBool()){
        ui->image_buttons->setVisible(true);
        viewer = new Movie_Viewer(ui->img_label);

    } else if (!qstrcmp(m_pFile->mime,"image/gif") ||
        !qstrncmp(m_pFile->mime,"audio/",6) ||
        !qstrncmp(m_pFile->mime,"video/",6)){
        ui->stackedWidget->setCurrentIndex(1);
        viewer = new Video_Viewer(ui->mpv_player);

    } else if (!qstrncmp(m_pFile->mime,"image/",6)){
        ui->image_buttons->setVisible(true);
        viewer = new Image_Viewer(ui->img_label);

    } else if (!qstrcmp(m_pFile->mime,"application/zip") ||
               !qstrcmp(m_pFile->mime,"application/vnd.comicbook+zip")){
        ui->image_buttons->setVisible(true);
        ui->cbz_buttons->setVisible(true);
        viewer = new Cbz_Viewer(ui->img_label);

    } else if (!qstrcmp(m_pFile->mime,"text/plain")){
        viewer = new Text_Viewer(ui->img_label);

    } else {
        viewer = new Unsupported_Viewer(ui->img_label);
    }

    viewer->set_file(m_pFile->path);
    set_file_info(viewer->get_info());
    set_scale();

    ui->file_tags->setTagsFromFile(m_pFile);


}

void FileViewer::update_fit(const QString &text){
    if (text == "Fit Both")
        scale_type = SCALE_TYPE::BOTH;
    else if (text == "Fit Width")
        scale_type = SCALE_TYPE::WIDTH;
    else if (text == "Fit Height")
        scale_type = SCALE_TYPE::HEIGHT;
    else if (text == "Original")
        scale_type = SCALE_TYPE::ORIGINAL;

    set_scale();
}

void FileViewer::zoom_in(){
    if (viewer != nullptr)
        viewer->zoom_in();
}

void FileViewer::zoom_out(){
    if (viewer != nullptr)
        viewer->zoom_out();
}

void FileViewer::set_scale()
{
    if (viewer != nullptr)
        viewer->resize(ui->scrollArea->width(),ui->scrollArea->height(),scale_type);
}

void FileViewer::next_file()
{
    if (position+1 < m_files.size()){
        position++;
        set_file();
    }
}

void FileViewer::prev_file()
{
    if (position-1 >= 0){
        position--;
        set_file();
    }
}

void FileViewer::delete_file()
{
    PyrosTC *ptc = PyrosTC::get();
    PyrosFile *pFile = m_files.at(position);

    if (pFile == nullptr || m_files.size() == 0)
        return;

    QVector<QByteArray> file_hashes;
    file_hashes.append(pFile->hash);


    ptc->delete_file(pFile);
    m_files.removeAt(position);
    if (position >= m_files.size())
        position--;
    if (position < 0)
        position = 0;

    set_file();
    emit file_deleted(file_hashes);

}

void FileViewer::add_tag(QVector<QByteArray> tags)
{
    if (m_pFile == nullptr)
        return;

    PyrosTC *ptc = PyrosTC::get();
    ptc->add_tags(QByteArray(m_pFile->hash),tags);
}

void FileViewer::remove_tag(QVector<QByteArray> tags)
{
    PyrosTC *ptc = PyrosTC::get();
    ptc->remove_tags(m_pFile->hash,tags);
}


bool FileViewer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize &&
            obj == ui->scrollArea) {
        if (scale_type != SCALE_TYPE::ORIGINAL){
            set_scale();
        }
    }

    // enable scrollbars on hovor
    if (event->type() == QEvent::Enter) {
        switch (scale_type){
        case SCALE_TYPE::HEIGHT:
            ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        case SCALE_TYPE::WIDTH:
            ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        default:
            ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            break;
        }
    }

    if (event->type() == QEvent::Leave) {
        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    return QWidget::eventFilter(obj, event);
}

void FileViewer::cbz_next_page()
{
    if (viewer != nullptr){
        viewer->next_page();
        set_file_info(viewer->get_info());
        ui->scrollArea->verticalScrollBar()->setValue(0);
    }
}

void FileViewer::cbz_prev_page()
{
    if (viewer != nullptr){
        viewer->prev_page();
        set_file_info(viewer->get_info());
        ui->scrollArea->verticalScrollBar()->setValue(0);
    }
}

void FileViewer::set_file_info(QString string)
{
    ui->file_info->setText(string);
}

void FileViewer::hide_files(QVector<QByteArray> hashes){
    for (int i  = m_files.length()-1;i >= 0;i--) {
        PyrosFile *file = m_files.at(i);
        if (file == nullptr)
            continue;

        for (int j  = hashes.length()-1;j >= 0;j--) {
            if (!hashes.at(j).compare(file->hash)){
                m_files.removeAt(i);
                if (position >= m_files.size())
                    position--;
                if (position < 0)
                    position = 0;
                hashes.removeAt(j);
            }
        }

    }

    set_file();
}
