#ifndef CONFIGTAB_H
#define CONFIGTAB_H

#include <QWidget>
#include <QPointer>
#include <QVariant>
#include <QFrame>
#include <QLineEdit>

#include "tab.h"

class QHBoxLayout;
class QVBoxLayout;
class QBoxLayout;
class QSettings;
class QPushButton;
class QCheckBox;
class QStackedWidget;
class QValidator;
class QLabel;

class ColorLineEdit : public QLineEdit{
    Q_OBJECT
public:
    ColorLineEdit(QWidget *parent = nullptr);
    ~ColorLineEdit();

private slots:
    void update_color_text(const QString &text);
    void update_color(const QString &text);

};


class SettingArrayList : public QFrame{
    Q_OBJECT

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
        TAG_HISTORY,
        GIFS_AS_VIDEO,
        TIMESTAMP,
        THEME,
        USE_INTERNAL_IMAGE_THUMBNAILER,
        USE_CBZ_THUMBNAILER,
        USE_VIDEO_THUMBNAILER,
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
        KEY_APPLY,
        KEY_FULLSCREEN,
        THUMBNAIL_SIZE,
        CBZ_THUMB_PAGE_COUNT,
        KEY_FOCUS_FILE_VIEWER,
        THUMBNAIL_DIR,
        SHOW_REMAINING_TIME,
        TAG_COLOR,
        FILE_COLOR,
        HIGHLIGHT_SIMMILAR_TAGS,
        HIGHLIGHT_COLOR,
        CHILD_HIGHLIGHT_COLOR,
    };
private:

    enum settings_type{
        BOOL,
        STRING,
        KEY,
        COMBO,
        COLOR_ARRAY,
        COLOR,
    };

    struct Setting_Item{
        Setting id;
        QString group;
        QString name;
        QString tooltip;
        QString key;
        QVariant default_val;
        settings_type type;
        QValidator*validator;
    };

    struct Setting_Group_Item{
        const Setting_Item* item;
        QWidget *widget;
    };

    class Setting_Group{
    public:
        Setting_Group(QString name,Setting_Group *parent);
        bool get_subgroup(QString name,Setting_Group *&group);
        void apply();

        QBoxLayout *layout;
        QPushButton *button;
        QString name;
        Setting_Group *parent;
        QVector<Setting_Group_Item> items;
        QVector<Setting_Group> sub_groups;
    };

    Setting_Group setting_group_top = Setting_Group("",nullptr);

    QStackedWidget *pages;
    QVBoxLayout *button_column;

    int header_size = 19;
    int sub_header_size = 14;
    int font_size = 12;

public:
    explicit configtab(QTabWidget *parent = nullptr);
    ~configtab();


    struct color_setting{
        QString glob;
        QColor color;
    };

    static QVariant setting_value(Setting setting);
    static QString setting_name(Setting setting);
    static QAction *create_binding(Setting setting,QString name,QWidget *widget);
    static QVector<color_setting> get_tag_colors();
    static QVector<color_setting> get_file_colors();
    static void update_bindings();
    static void disable_bindings();

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


    static const Setting_Item settings[];

    static QVector<binding> active_bindings;

    void new_page(Setting_Group *group);
    void set_page();
    QBoxLayout *create_header(QBoxLayout *layout,QString text,int size);


    template <typename T>
    void create_lineedit_settings_entry(QBoxLayout *layout,Setting_Group_Item &item);
    void create_keybind_settings_entry(QBoxLayout *layout,Setting_Group_Item &item);
    void create_checkbox_settings_entry(QBoxLayout *layout,Setting_Group_Item &item);
    void create_combo_settings_entry(QBoxLayout *layout, Setting_Group_Item &item);

    void apply();

    void load_setting_groups();

    static QVector<configtab::color_setting> get_colors(QString group);

signals:
    void settings_changed();
};



#endif // CONFIGTAB_H
