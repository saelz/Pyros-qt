#include "databasecreation.h"
#include "ui_databasecreation.h"
#include "pyrosdb.h"

#include <QSettings>
#include <QErrorMessage>
#include <QDir>

#include <pyros.h>

DatabaseCreation::DatabaseCreation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DatabaseCreation)
{
    ui->setupUi(this);
    connect(ui->create_database_button,&QPushButton::clicked,this,&DatabaseCreation::create_database);
    ui->database_path->setText(QDir::homePath()+"/.local/share/pyros/main");
}

DatabaseCreation::~DatabaseCreation()
{
    delete ui;
}

void DatabaseCreation::create_database(){
    QByteArray path = ui->database_path->text().toUtf8();
    if (Pyros_Database_Exists(path.data())){
        QErrorMessage qem;
        qem.showMessage("Database \""+path+"\" already exists");
        qem.exec();
    } else {
        QSettings settings;
        PYROS_HASHTYPE hashtype = PYROS_MD5HASH;
        if (ui->hashtype->currentText() == "MD5")
            hashtype = PYROS_MD5HASH;
        else if (ui->hashtype->currentText() == "SHA1")
            hashtype = PYROS_SHA1HASH;
        else if (ui->hashtype->currentText() == "SHA256")
            hashtype = PYROS_SHA256HASH;
        else if (ui->hashtype->currentText() == "SHA512")
            hashtype = PYROS_SHA512HASH;
        else if (ui->hashtype->currentText() == "BLAKE2B")
            hashtype = PYROS_BLAKE2BHASH;
        else if (ui->hashtype->currentText() == "BLAKE2S")
            hashtype = PYROS_BLAKE2SHASH;

        PyrosTC* ptc = PyrosTC::get();
        ptc->close_db();
        PyrosDB *db = Pyros_Create_Database(path.data(),hashtype);
        Pyros_Commit(db);
        Pyros_Close_Database(db);

        settings.setValue("db",path);
        emit delete_all_tabs();
        emit new_search();
    }

}
