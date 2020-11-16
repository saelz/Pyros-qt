#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QPointer>

class QHBoxLayout;
class QVBoxLayout;
class QSettings;
class QLineEdit;
class QPushButton;

namespace Ui {
class configtab;
}

class color_entry : public QObject{
    Q_OBJECT
public:
    color_entry(QVBoxLayout *parent,QString placeholder,
            QString item = "",QString hex = "");
    ~color_entry();
    QHBoxLayout *hbox;
    QLineEdit *entry;
    QLineEdit *color;
    QPushButton *delete_button;
public slots:
    void update_color(const QString &text);
};

class configtab : public QWidget
{
    Q_OBJECT


    QVector<QPointer<color_entry>> file_colors;
    QVector<QPointer<color_entry>> tag_colors;

public:
    explicit configtab(QWidget *parent = nullptr);
    ~configtab();

private:
    Ui::configtab *ui;
    void enable_all_buttons();
    void set_tag_page();
    void set_file_page();
    void set_general_page();
    void apply();
    void new_color_tag();
    void new_file_border();
    void apply_color_entries(QSettings &settings,QVector<QPointer<color_entry>> &entires);

signals:
    void settings_changed();
};



#endif // CONFIGTAB_H
