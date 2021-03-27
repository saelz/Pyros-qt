#ifndef DATABASECREATION_H
#define DATABASECREATION_H

#include <QWidget>
#include "tab.h"

namespace Ui {
class DatabaseCreation;
}

class DatabaseCreation : public Tab
{
    Q_OBJECT

public:
    explicit DatabaseCreation(QTabWidget *parent = nullptr);
    ~DatabaseCreation();

private:
    Ui::DatabaseCreation *ui;
private slots:
    void create_database();
signals:
    void new_search();
    void delete_all_tabs();
};

#endif // DATABASECREATION_H
