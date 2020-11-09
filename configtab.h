#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QSettings;

namespace Ui {
class configtab;
}

class configtab : public QWidget
{
    Q_OBJECT

public:
    explicit configtab(QWidget *parent = nullptr);
    ~configtab();

private:
    Ui::configtab *ui;
    void set_tag_page();
    void set_file_page();
    void set_general_page();
    void apply();
    void new_color_tag();
    void new_file_border();
    QHBoxLayout *new_color_entry(QString item,QString hex,QString placeholder);
    void apply_color_entries(QVBoxLayout *inital_layout,QSettings &settings);

signals:
    void settings_changed();
};

#endif // CONFIGTAB_H
