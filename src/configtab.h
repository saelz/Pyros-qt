#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QPointer>
#include <QVariant>
#include <QFrame>

#include "tab.h"

class QHBoxLayout;
class QVBoxLayout;
class QBoxLayout;
class QSettings;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QStackedWidget;
class QValidator;

class SettingArrayList : public QFrame{
    Q_OBJECT
private:
    QString array_name;
    struct Key{
        QString name;
        QString placeholder;
        bool isColor;
    };
    struct Entry{
        QVector<QLineEdit*> data;
        QPushButton *delete_button;
    };

    QVector<Entry> entries;
    QVBoxLayout *entry_container;

    void update_color(const QString &text);

    QVector<Key> keys;

public:
    SettingArrayList(QWidget *parent,QString array_name,QVector<Key> keys);
public slots:
    void add_entry(int existing_entry = -1);
    void delete_entry();
    void apply();
};

class configtab : public Tab
{
    Q_OBJECT
public:
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
        KEY_LOCK_MEDIA_VIEWER_OVERLAY,
        KEY_TOGGLE_MUTE,
        KEY_NEXT_FILE,
        KEY_PREV_FILE,
        KEY_ZOOM_IN,
        KEY_ZOOM_OUT,
        KEY_NEXT_PAGE,
        KEY_PREV_PAGE,
        KEY_DELETE_FILE,
        KEY_CLOSE_TAB,
        KEY_REFRESH,
        THUMBNAIL_SIZE,
        CBZ_THUMB_PAGE_COUNT,
        KEY_FOCUS_FILE_VIEWER,
        THUMBNAIL_DIR,
        SHOW_REMAINING_TIME
    };
private:

    enum settings_type{
        BOOL,
        STRING,
        COMBO,
    };

    struct settings_item{
        QWidget *widget;
        settings_type type;
        Setting setting;
    };
    QVector<settings_item> settings_items;
    QVector<SettingArrayList*> setting_array_items;


    QVector<QPushButton *> config_buttons;
    QStackedWidget *pages;
    QVBoxLayout *button_column;

    int header_size = 19;
    int sub_header_size = 14;
    int font_size = 12;

public:
    explicit configtab(QTabWidget *parent = nullptr);
    ~configtab();


    struct color_setting{
        QString starts_with;
        QColor color;
    };

    static QVariant setting_value(Setting setting);
    static QString setting_name(Setting setting);
    static QAction *create_binding(Setting setting,QString name,QWidget *widget);
    static QVector<color_setting> get_tag_colors();
    static QVector<color_setting> get_file_colors();


private:
    struct binding{
        const QPointer<QAction> action;
        Setting set;
    };

    struct setting{
        QString name;
        QVariant default_val;
        QValidator*validator;
    };

    int left_margin = 15;


    static const setting settings[];

    static QVector<binding> active_bindings;

    QBoxLayout *new_page(QString title);
    void set_page();
    QBoxLayout *create_header(QBoxLayout *layout,QString text,int size);

    void create_checkbox_settings_entry(QBoxLayout *layout,QString display_text,Setting set);
    void create_lineedit_settings_entry(QBoxLayout *layout,QString display_text,Setting set);
    void create_combo_settings_entry(QBoxLayout *layout, QString display_text,Setting set, QStringList combo_items);
    void apply();
    void update_bindings();

    static QVector<configtab::color_setting> get_colors(QString group);

signals:
    void settings_changed();
};



#endif // CONFIGTAB_H
