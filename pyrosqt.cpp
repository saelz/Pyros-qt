#include "pyrosqt.h"
#include "ui_pyrosqt.h"
#include "fileimport.h"
#include "searchtab.h"
#include "fileviewer.h"
#include "dupefinder.h"
#include "pyrosdb.h"
#include "configtab.h"
#include "databasecreation.h"

#include <QDir>
#include <QFileDialog>
#include <QErrorMessage>

PyrosQT::PyrosQT(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PyrosQT)
{
    ui->setupUi(this);

    QAction *new_stab = new QAction("New Search Tab",this);
    QAction *new_itab = new QAction("New Import Tab",this);
    QAction *close_tab = new QAction("Close Tab",this);


    new_stab->setShortcut(QKeySequence("CTRL+t"));
    new_itab->setShortcut(QKeySequence("CTRL+i"));
    close_tab->setShortcut(QKeySequence("CTRL+w"));

    connect(new_stab, &QAction::triggered,this, &PyrosQT::new_search_tab);
    connect(new_itab, &QAction::triggered,this, &PyrosQT::new_import_tab);
    connect(close_tab,&QAction::triggered,this, &PyrosQT::remove_tab_current);

    addAction(new_itab);
    addAction(new_stab);
    addAction(close_tab);

    initalize_config();
    connect(ui->actionImport_Files,   &QAction::triggered,this, &PyrosQT::new_import_tab);
    connect(ui->actionNew_Search,     &QAction::triggered,this, &PyrosQT::new_search_tab);
    connect(ui->actionSettings,       &QAction::triggered,this, &PyrosQT::new_config_tab);
    connect(ui->actionNew_Database,   &QAction::triggered,this, &PyrosQT::new_database_creation_tab);
    connect(ui->actionOpen_Database,  &QAction::triggered,this, &PyrosQT::open_database);

    connect(ui->tabWidget,&QTabWidget::tabCloseRequested,this, &PyrosQT::remove_tab);
}

PyrosQT::~PyrosQT()
{
    PyrosTC *ptc = PyrosTC::get();
    delete ptc;
    delete ui;

}


void PyrosQT::initalize_config(){
    QSettings settings;

    if (settings.value("db","") == ""){
        settings.beginGroup("filecolor");
        settings.setValue("video/",QColorConstants::Green);
        settings.setValue("image/gif",QColorConstants::Green);
        settings.setValue("audio/",QColorConstants::Blue);
        settings.endGroup();

        settings.beginGroup("tagcolor");
        settings.setValue("character:",QColorConstants::Green);
        settings.setValue("series:",QColorConstants::Red);
        settings.setValue("meta:",QColorConstants::DarkGray);
        settings.setValue("creator:",QColorConstants::Blue);
        settings.endGroup();
        new_database_creation_tab();
    } else {
        new_search_tab();
    }
    load_settings();

}

void PyrosQT::load_settings(){
    QSettings settings;
    QString theme = settings.value("theme","Default").toString();
    QPalette palette = QPalette();

    if (theme == "Default"){
        settings.setValue("special-tagcolor/default",QColorConstants::Black);
    } else if (theme == "Dark Theme"){
        settings.setValue("special-tagcolor/default",QColorConstants::White);
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(60, 60, 60));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Link, QColor(42, 130, 218));
        palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        palette.setColor(QPalette::HighlightedText, Qt::black);

    }
    QApplication::setPalette(palette);
}

void PyrosQT::new_import_tab()
{
    FileImport *fi = new FileImport(ui->tabWidget);
    ui->tabWidget->addTab(fi,"Import");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    connect(fi,&FileImport::new_search,this,&PyrosQT::new_search_tab_with_vector);
    //connect(fi,&FileImport::delete_self,this,&PyrosQT::remove_tab_by_widget);
}

void PyrosQT::new_search_tab()
{
    SearchTab *st = new SearchTab(ui->tabWidget);
    ui->tabWidget->addTab(st,"Search");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    connect(st,&SearchTab::set_title,this,&PyrosQT::set_tab_title);
    connect(st,&SearchTab::create_viewer_tab,this,&PyrosQT::new_viewer_tab);
}
void PyrosQT::new_search_tab_with_vector(QVector<PyrosFile*>files)
{
    SearchTab *st = new SearchTab(files,ui->tabWidget);
    ui->tabWidget->addTab(st,"Import");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    connect(st,&SearchTab::set_title,this,&PyrosQT::set_tab_title);
    connect(st,&SearchTab::create_viewer_tab,this,&PyrosQT::new_viewer_tab);
}

void PyrosQT::remove_tab(int index)
{
    QWidget *widget = ui->tabWidget->widget(index);
    ui->tabWidget->removeTab(index);
    delete widget;

}

void PyrosQT::remove_tab_current()
{
    remove_tab(ui->tabWidget->currentIndex());
}

void PyrosQT::set_tab_title(QString title,QWidget *sender){
    int index = ui->tabWidget->indexOf(sender);
    ui->tabWidget->setTabText(index,title);

}

void PyrosQT::new_viewer_tab(QVector<PyrosFile*> files,int inital_position)
{
    FileViewer *fv = new FileViewer(files,inital_position,ui->tabWidget);
    ui->tabWidget->addTab(fv,"Viewer");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void PyrosQT::new_config_tab()
{
    configtab *ct = new configtab();
    ui->tabWidget->addTab(ct,"Config");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    connect(ct,&configtab::settings_changed,this,&PyrosQT::load_settings);
}


void PyrosQT::new_database_creation_tab(){
    DatabaseCreation *dc = new DatabaseCreation();
    ui->tabWidget->addTab(dc,"New Database");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    connect(dc,&DatabaseCreation::new_search,this,&PyrosQT::new_search_tab);
    connect(dc,&DatabaseCreation::delete_all_tabs,this,&PyrosQT::close_all_tabs);
}


void PyrosQT::close_all_tabs(){
    while (ui->tabWidget->count() >= 1)
        remove_tab(0);
}

void PyrosQT::open_database()
{
    QByteArray dir = QFileDialog::getExistingDirectory(this, tr("Open Database"),
                            QDir::homePath()+"/.local/share/pyros/",
                            QFileDialog::ShowDirsOnly
                            | QFileDialog::DontResolveSymlinks).toUtf8();
    if (Pyros_Database_Exists(dir)){
        QSettings settings;
        PyrosTC* ptc = PyrosTC::get();
        ptc->close_db();
        close_all_tabs();
        settings.setValue("db",dir);
        new_search_tab();
    } else {
        QErrorMessage qem;
        qem.showMessage("Not a database.");
        qem.show();
    }

}
