#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QPointer>

class QHBoxLayout;
class QVBoxLayout;
class QBoxLayout;
class QSettings;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QStackedWidget;

class color_entry : public QObject{
    Q_OBJECT
public:
    color_entry(QBoxLayout *parent,QString placeholder,
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

    enum settings_type{
        BOOL,
        STRING,
        COMBO,
    };

    struct color_entry_button{
        QPushButton *button;
        QVBoxLayout *entry_container;
        QString placeholder;
        QVector <QPointer<color_entry>>   *entries;
    };

    struct settings_item{
        QWidget *widget;
        QString setting_name;
        settings_type type;
    };
    QVector<settings_item> settings_items;

    QVector<QPointer<color_entry>> file_colors;
    QVector<QPointer<color_entry>> tag_colors;

    QVector<color_entry_button> entry_buttons;
    QVector<QPushButton *> config_buttons;
    QStackedWidget *pages;
    QVBoxLayout *button_column;

    int header_size = 19;
    int sub_header_size = 14;
    int font_size = 12;

public:
    explicit configtab(QWidget *parent = nullptr);
    ~configtab();

private:

    QVBoxLayout *new_page(QString title);
    void set_page();
    void create_header(QBoxLayout *layout,QString text,int size);
    void create_color_entries(QBoxLayout *layout,QString header, QString setting_group,QString placeholder,QVector<QPointer<color_entry>> &list);

    void new_color_entry();

    void create_checkbox_settings_entry(QBoxLayout *layout,QString
                               display_text,QString setting_name,
                               bool default_state);
    void create_settings_entry(QBoxLayout *layout,
                               QString display_text,QString setting_name,
                               QString default_str);
    void create_settings_entry(QBoxLayout *layout,
                               QString display_text,QString setting_name,
                               QStringList combo_items);
    void apply();
    void apply_color_entries(QSettings &settings,QVector<QPointer<color_entry>> &entires);

signals:
    void settings_changed();
};



#endif // CONFIGTAB_H
