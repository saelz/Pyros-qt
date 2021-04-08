#ifndef FILEIMPORT_H
#define FILEIMPORT_H

#include "tab.h"

#include <QWidget>
#include <QProgressBar>
#include <QTabWidget>
#include <QSortFilterProxyModel>

#include <pyros.h>

class QMenu;

class TagFileFilterProxyModel : public QSortFilterProxyModel
{
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
};

namespace Ui {
class FileImport;
}

class FileImport : public Tab
{
    Q_OBJECT

public:
    explicit FileImport(QTabWidget *parent = nullptr);
    ~FileImport();

private:
    Ui::FileImport *ui;
    QVector<QByteArray> m_import_tags;
    static QString starting_dir;
    QMenu *filecontextMenu;

private slots:

    void add_tags(QVector<QByteArray> tags);
    void remove_tags(QVector<QByteArray> tags);
    void add_files();
    void import_files();
    void remove_selected_files();
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);

    void create_file_context_menu(const QPoint &point);

    void select_tag_bar();
signals:
    void new_search(QVector<PyrosFile*>);
    void new_search_with_tags(QVector<QByteArray> tags);
};

#endif // FILEIMPORT_H
