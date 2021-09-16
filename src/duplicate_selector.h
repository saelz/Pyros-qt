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
    };

    int position;

public:
    explicit duplicate_selector(QVector<PyrosFile*> files,QTabWidget *parent = nullptr);
    ~duplicate_selector();

private:
    Ui::duplicate_selector *ui;
    QVector<DUPLICATE_STATUS> file_statuses;

    void check_file_status();
    void apply();

private slots:
    void file_hidden(int);
    void set_position(int);
    void entry_changed(int);

signals:
    void hide_files(QVector<QByteArray>);
    void files_removed(QVector<QByteArray>);
    void set_dupe_combo_box_status(int);
    void set_apply_button_enabled(bool);
    void hide_combo_box_entry(int,bool);

};

#endif // DUPLICATE_SELECTOR_H
