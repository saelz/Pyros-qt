#ifndef PYROSQT_H
#define PYROSQT_H


#include <QMainWindow>
#include <QPointer>


class QAbstractItemModel;
class SearchTab;

QT_BEGIN_NAMESPACE
namespace Ui { class PyrosQT; }
QT_END_NAMESPACE
struct PyrosFile;

class PyrosQT : public QMainWindow
{
    Q_OBJECT

public:
    PyrosQT(QWidget *parent = nullptr);
    ~PyrosQT();

    void initalize_config();

private:
    Ui::PyrosQT *ui;

private slots:
    void load_settings();

    void new_import_tab();
    void new_search_tab();
    void new_search_tab_with_vector(QVector<PyrosFile*>files);
    void new_search_tab_with_tags(QVector<QByteArray>tags);
    void search_tab_init(SearchTab *st);
    void new_viewer_tab(QVector<PyrosFile*> files,int inital_position);
    void new_config_tab();
    void new_database_creation_tab();
    void new_duplicate_selector_tab(QVector<PyrosFile*> files);

    void open_database();

    void remove_tab(int index);
    void remove_tab_current();
    void close_all_tabs();

    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
public slots:
    void show_error(QString msg);
};
#endif // PYROSQT_H
