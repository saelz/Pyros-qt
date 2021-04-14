#include "pyrosqt.h"
#include "ui_pyrosqt.h"
#include "fileimport.h"
#include "searchtab.h"
#include "fileviewer.h"
#include "pyrosdb.h"
#include "configtab.h"
#include "databasecreation.h"
#include "duplicate_selector.h"

#include <QDir>
#include <QFileDialog>
#include <QErrorMessage>
#include <QMouseEvent>
#include <QTabBar>

PyrosQT::PyrosQT(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PyrosQT)
{
    ui->setupUi(this);

    QAction *new_stab = configtab::create_binding(configtab::KEY_NEW_SEARCH,"New Search Tab",this);
    QAction *new_itab = configtab::create_binding(configtab::KEY_NEW_IMPORT,"New Import Tab",this);
    QAction *close_tab = configtab::create_binding(configtab::KEY_CLOSE_TAB,"Close Tab",this);

    connect(new_stab, &QAction::triggered,this, &PyrosQT::new_search_tab);
    connect(new_itab, &QAction::triggered,this, &PyrosQT::new_import_tab);
    connect(close_tab,&QAction::triggered,this, &PyrosQT::remove_tab_current);


    initalize_config();
    connect(ui->actionImport_Files,  &QAction::triggered,this, &PyrosQT::new_import_tab);
    connect(ui->actionNew_Search,    &QAction::triggered,this, &PyrosQT::new_search_tab);
    connect(ui->actionSettings,      &QAction::triggered,this, &PyrosQT::new_config_tab);
    connect(ui->actionNew_Database,  &QAction::triggered,this, &PyrosQT::new_database_creation_tab);
    connect(ui->actionOpen_Database, &QAction::triggered,this, &PyrosQT::open_database);

    connect(ui->tabWidget,&QTabWidget::tabCloseRequested,this, &PyrosQT::remove_tab);

    ui->tabWidget->installEventFilter(this);
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
        settings.beginWriteArray("filecolor");
        settings.setArrayIndex(0);
        settings.setValue("prefix","video");
        settings.setValue("color","#22ff22");
        settings.setArrayIndex(1);
        settings.setValue("prefix","image/gif");
        settings.setValue("color","#44cc88");
        settings.setArrayIndex(2);
        settings.setValue("prefix","audio");
        settings.setValue("color","#33bbff");
        settings.setArrayIndex(3);
        settings.setValue("prefix","application/zip");
        settings.setValue("color","#cc7722");
        settings.endArray();

        settings.beginWriteArray("tagcolor");
        settings.setArrayIndex(0);
        settings.setValue("prefix","meta:");
        settings.setValue("color","#cccccc");
        settings.endArray();

        new_database_creation_tab();
    } else {
        if (QCoreApplication::arguments().count() >=3 &&
                QCoreApplication::arguments().at(1) == "--search"){
            QVector<QByteArray> tags;

            for(int i = 2;i < QCoreApplication::arguments().count();i++)
                tags.append(QCoreApplication::arguments().at(i).toUtf8());

            new_search_tab_with_tags(tags);

        } else {
            new_search_tab();
        }

    }
    load_settings();

}

void PyrosQT::load_settings(){
    QSettings settings;
    QString theme = configtab::setting_value(configtab::THEME).toString();
    QPalette palette = QPalette();

    if (theme == "Default"){
        settings.setValue("special-tagcolor/default",QColorConstants::Black);
    } else if (theme == "Dark Theme"){
        settings.setValue("special-tagcolor/default",QColorConstants::White);
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(60, 60, 60));
        palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        palette.setColor(QPalette::ToolTipBase, Qt::black);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText, Qt::lightGray);
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

    connect(fi,&FileImport::new_search,this,&PyrosQT::new_search_tab_with_vector);
    connect(fi,&FileImport::new_search_with_tags,this,&PyrosQT::new_search_tab_with_tags);
}

void PyrosQT::new_search_tab()
{
    SearchTab *st = new SearchTab(ui->tabWidget);
    search_tab_init(st);
}
void PyrosQT::new_search_tab_with_vector(QVector<PyrosFile*>files)
{
    SearchTab *st = new SearchTab(files,ui->tabWidget);
    search_tab_init(st);
}
void PyrosQT::new_search_tab_with_tags(QVector<QByteArray>tags)
{
    SearchTab *st = new SearchTab(tags,ui->tabWidget);
    search_tab_init(st);
}

void PyrosQT::search_tab_init(SearchTab *st)
{

    connect(st,&SearchTab::create_viewer_tab,this,&PyrosQT::new_viewer_tab);
    connect(st,&SearchTab::create_new_search_with_tags,this,&PyrosQT::new_search_tab_with_tags);
    connect(st, &SearchTab::new_duplicate_selector_tab,this,&PyrosQT::new_duplicate_selector_tab);

    connect(st,&SearchTab::file_deleted,this,&PyrosQT::files_removed);
    connect(this,&PyrosQT::files_removed,st,&SearchTab::hide_files_by_hash);
}

void PyrosQT::remove_tab(int index)
{
    Tab *tab = qobject_cast<Tab*>(ui->tabWidget->widget(index));
    tab->delete_self();
}

void PyrosQT::remove_tab_current()
{
    remove_tab(ui->tabWidget->currentIndex());
}

void PyrosQT::new_viewer_tab(QVector<PyrosFile*> files,int inital_position)
{
    Tab *tab = qobject_cast<Tab*>(sender());
    FileViewer *fv = new FileViewer(files,inital_position,ui->tabWidget);
    fv->set_parent_tab(tab);

    connect(fv,&FileViewer::new_search_with_selected_tags,this,&PyrosQT::new_search_tab_with_tags);

    connect(fv,&FileViewer::file_deleted,this,&PyrosQT::files_removed);
    connect(this,&PyrosQT::files_removed,fv,&FileViewer::hide_files);
}

void PyrosQT::new_config_tab()
{
    configtab *conft = new configtab(ui->tabWidget);
    connect(conft,&configtab::settings_changed,this,&PyrosQT::load_settings);
}


void PyrosQT::new_database_creation_tab()
{
    DatabaseCreation *dc = new DatabaseCreation(ui->tabWidget);
    connect(dc,&DatabaseCreation::new_search,this,&PyrosQT::new_search_tab);
    connect(dc,&DatabaseCreation::delete_all_tabs,this,&PyrosQT::close_all_tabs);
}

void PyrosQT::new_duplicate_selector_tab(QVector<PyrosFile*> files)
{
    Tab *tab = qobject_cast<Tab*>(sender());
    duplicate_selector *ds = new duplicate_selector(files,ui->tabWidget);
    ds->set_parent_tab(tab);
    connect(ds,&duplicate_selector::files_removed,this,&PyrosQT::files_removed);
    connect(this,&PyrosQT::files_removed,ds,&duplicate_selector::hide_files);

}

void PyrosQT::close_all_tabs()
{
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
        qem.showMessage("\""+dir+"\" is not a database.");
        qem.exec();
    }

}

void PyrosQT::closeEvent(QCloseEvent *event)
{
    close_all_tabs();
    QMainWindow::closeEvent(event);
}

bool PyrosQT::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress &&
            obj == ui->tabWidget){
        QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::MiddleButton)
            remove_tab(ui->tabWidget->tabBar()->tabAt(mouse_event->pos()));
    }

    return QMainWindow::eventFilter(obj,event);
}
