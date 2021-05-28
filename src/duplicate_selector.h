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

public:
    explicit duplicate_selector(QVector<PyrosFile*> files,QTabWidget *parent = nullptr);
    ~duplicate_selector();

public slots:
    void hide_files(QVector<QByteArray> hashes);

private:
    Ui::duplicate_selector *ui;
    int file_position = 0;

    QVector<PyrosFile*> m_files;
    QVector<DUPLICATE_STATUS> file_statuses;

    void update_file();
    void next_file();
    void prev_file();
    void apply();

    void duplicate_checked();
    void not_duplicate_checked();
    void superior_checked();

    void update_radio_buttons();

signals:
    void update_file_count(QString);
    void files_removed(QVector<QByteArray>);
};

#endif // DUPLICATE_SELECTOR_H
