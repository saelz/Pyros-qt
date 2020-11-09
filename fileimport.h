#ifndef FILEIMPORT_H
#define FILEIMPORT_H

#include <QWidget>
#include <QProgressBar>
#include <QTabWidget>
#include <pyros.h>

namespace Ui {
class FileImport;
}

class FileImport : public QWidget
{
    Q_OBJECT

public:
    explicit FileImport(QWidget *parent = nullptr);
    ~FileImport();

private:
    Ui::FileImport *ui;
    QVector<QByteArray> m_import_tags;

private slots:

    void add_tags(QVector<QByteArray> tags);
    void remove_tags(QVector<QByteArray> tags);
    void add_files();
    void import_files();
signals:
    void new_search(QVector<PyrosFile*>);
    void delete_self(QWidget*);
};

#endif // FILEIMPORT_H
