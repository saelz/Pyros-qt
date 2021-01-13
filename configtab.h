#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QPointer>
#include <QVariant>

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

    enum Setting{
        TAG_HISTORY = 0,
        GIFS_AS_VIDEO,
        TIMESTAMP,
        THEME,
        USE_INTERNAL_IMAGE_THUMBNAILER,
        USE_CBZ_THUMBNAILER,
        USE_EXTERNAL_THUMBNAILER,
        KEY_NEW_SEARCH,
        KEY_NEW_IMPORT,
        KEY_FOCUS_TAG_BAR,
        KEY_INVERT_SELECTION,
        KEY_FOCUS_SEARCH_BAR,
        KEY_FOCUS_FILE_GRID,
        KEY_NEXT_FILE,
        KEY_PREV_FILE,
        KEY_ZOOM_IN,
        KEY_ZOOM_OUT,
        KEY_NEXT_PAGE,
        KEY_PREV_PAGE,
        KEY_DELETE_FILE,
        KEY_CLOSE_TAB,
        KEY_REFRESH,
    };

    static QVariant setting_value(Setting setting);
    static QString setting_name(Setting setting);
    static QAction *create_binding(Setting setting,QString name,QWidget *widget);

private:
    struct binding{
        const QPointer<QAction> action;
        Setting set;
    };

    struct setting{
        QString name;
        QVariant default_val;
    };

    static const setting settings[];

    static QVector<binding> active_bindings;

    QVBoxLayout *new_page(QString title);
    void set_page();
    void create_header(QBoxLayout *layout,QString text,int size);
    void create_color_entries(QBoxLayout *layout,QString header, QString setting_group,QString placeholder,QVector<QPointer<color_entry>> &list);

    void new_color_entry();

    void create_checkbox_settings_entry(QBoxLayout *layout,QString display_text,Setting set);
    void create_lineedit_settings_entry(QBoxLayout *layout,QString display_text,Setting set);
    void create_combo_settings_entry(QBoxLayout *layout, QString display_text,Setting set, QStringList combo_items);
    void apply();
    void update_bindings();
    void apply_color_entries(QSettings &settings,QVector<QPointer<color_entry>> &entires);

signals:
    void settings_changed();
};



#endif // CONFIGTAB_H
