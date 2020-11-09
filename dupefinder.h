#ifndef DUPEFINDER_H
#define DUPEFINDER_H

#include "pyrosqt.h"
#include <QWidget>
#include <QFutureWatcher>
#include <cstddef>


namespace Ui {
class DuplicateFinder;
}

class DupeFinder : public QWidget
{
    Q_OBJECT
public:
    explicit DupeFinder(QTabWidget *tab,QWidget *parent = nullptr);
    ~DupeFinder();

private:
    QTabWidget *parent_tab;
    Ui::DuplicateFinder *ui;
    void generatePhashes(QVector<void*> files);

    QVector<void*> original_files;
    QVector<uint64_t> file_phashes;

    QFutureWatcher<uint64_t> *pHasher;
    int progress;
private slots:
    void findDuplicates();
    void updateProgress(int);
    void compare_phashes();
    void save_phashes();
};

#endif // DUPEFINDER_H
