#ifndef DUPLICATE_SELECTOR_H
#define DUPLICATE_SELECTOR_H

#include <QWidget>
#include "tab.h"

struct PyrosFile;

namespace Ui {
class duplicate_selector;
}

class duplicate_selector : public Tab
{
    Q_OBJECT

    enum DUPLICATE_STATUS{
        SUPERIOR,
        DUPLICATE,
        NOT_DUPLICATE,
        NONE
    };

    int position;

public:
    explicit duplicate_selector(QVector<PyrosFile*> files,QTabWidget *parent = nullptr);
    ~duplicate_selector();

private:
    Ui::duplicate_selector *ui;
    QVector<DUPLICATE_STATUS> file_statuses;

    void update_file();
    void apply();

    void duplicate_checked();
    void not_duplicate_checked();
    void superior_checked();

    void update_radio_buttons();

private slots:
    void file_hidden(int);
    void set_position(int);

signals:
    void hide_files(QVector<QByteArray>);
    void files_removed(QVector<QByteArray>);

};

#endif // DUPLICATE_SELECTOR_H
