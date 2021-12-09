#include "configtab.h"

#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QScrollArea>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>

const configtab::setting configtab::settings[] = {
    {"use_tag_history",true,nullptr},
    {"treat_gifs_as_video",false,nullptr},
    {"timestamp_format","MM/dd/yy",nullptr},
    {"theme","Default",nullptr},
    {"use_interal_image-thumbnailer",true,nullptr},
    {"use_interal_cbz-thumbnailer",true,nullptr},
    {"use_exernal_thumbnailer",true,nullptr},
    {"keybind/new-search-tab","CTRL+t",nullptr},
    {"keybind/new-import-tab","CTRL+i",nullptr},
    {"keybind/focus-tag-bar","i",nullptr},
    {"keybind/invert-selection","SHIFT+i",nullptr},
    {"keybind/focus-search-bar","a",nullptr},
    {"keybind/focus-file-grid","CTRL+f",nullptr},
    {"keybind/lock-media-overlay","CTRL+l",nullptr},
    {"keybind/toggle-mute","m",nullptr},
    {"keybind/next-file","CTRL+n",nullptr},
    {"keybind/prev-file","CTRL+p",nullptr},
    {"keybind/zoom-in","CTRL++",nullptr},
    {"keybind/zoom-out","CTRL+-",nullptr},
    {"keybind/next-page",">",nullptr},
    {"keybind/prev-page","<",nullptr},
    {"keybind/delete-file","CTRL+del",nullptr},
    {"keybind/close-tab","CTRL+w",nullptr},
    {"keybind/refresh","CTRL+r",nullptr},
    {"keybind/apply","CTRL+Return",nullptr},
    {"thumbnail_size","256",new QIntValidator(25, 999)},
    {"cbz_thumbnail_pages","3",new QIntValidator(1, 5)},
    {"keybind/focus-file-viewer","CTRL+f",nullptr},
    {"thumbnail_dir","~/.cache/PyrosQT",nullptr},
    {"show_video_remaining_time",false,nullptr},
};


QVector<configtab::binding> configtab::active_bindings;

configtab::configtab(QTabWidget *parent) :
    Tab(parent)
{

    QBoxLayout *page_layout;

    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout();
    QHBoxLayout *button_layout = new QHBoxLayout();
    QPushButton *apply_button = new QPushButton("Apply");

    set_title("Config");
    button_column = new QVBoxLayout();
    pages =  new QStackedWidget();


    vbox->addLayout(hbox);
    vbox->addLayout(button_layout);
    button_layout->insertStretch(-1);
    button_layout->addWidget(apply_button);

    hbox->addLayout(button_column);
    hbox->addWidget(pages);


    page_layout = new_page("General");
    {
        create_checkbox_settings_entry(page_layout,"Use tag history",TAG_HISTORY);
        create_checkbox_settings_entry(page_layout,"Use video player for gifs",GIFS_AS_VIDEO);
        create_lineedit_settings_entry(page_layout,"Timestamp format",TIMESTAMP);
        create_checkbox_settings_entry(page_layout,"Show Remaing time for videos instead of duration",SHOW_REMAINING_TIME);
        create_combo_settings_entry(page_layout,"Theme",THEME,{"Default" ,"Dark Theme"});
        page_layout->insertStretch(-1);
    }

    page_layout = new_page("Tags");
    {
        QBoxLayout *sub_layout = create_header(page_layout,"Tag Colors",sub_header_size);
        SettingArrayList *arraylist = new SettingArrayList(sub_layout->widget(),"tagcolor",{{"prefix","Tag",false},{"color","Color",true}});
        page_layout->addWidget(arraylist);
        setting_array_items.append(arraylist);

        page_layout->insertStretch(-1);
    }

    page_layout = new_page("Files");
    {
        QBoxLayout *sub_layout = create_header(page_layout,"File Color",sub_header_size);
        SettingArrayList *arraylist = new SettingArrayList(sub_layout->widget(),"filecolor",{{"prefix","Mime/Type",false},{"color","Tag Color",true}});
        page_layout->addWidget(arraylist);
        setting_array_items.append(arraylist);

        sub_layout = create_header(page_layout,"Thumbnails",sub_header_size);
        create_lineedit_settings_entry(sub_layout,"Thumbnail storage directory",THUMBNAIL_DIR);
        create_lineedit_settings_entry(sub_layout,"Thumbnail size",THUMBNAIL_SIZE);
        create_checkbox_settings_entry(sub_layout,"Use interal image thumbnailer",USE_INTERNAL_IMAGE_THUMBNAILER);
        create_checkbox_settings_entry(sub_layout,"Use interal cbz/zip thumbnailer",USE_CBZ_THUMBNAILER);
        create_lineedit_settings_entry(sub_layout,"Number of pages to show in cbz/zip thumbnail",CBZ_THUMB_PAGE_COUNT);
        create_checkbox_settings_entry(sub_layout,"Use external thumbnailers from /usr/share/thumbnailers/",USE_EXTERNAL_THUMBNAILER);

        page_layout->insertStretch(-1);
    }
    page_layout = new_page("Key Binds");
    {
        QBoxLayout *sub_layout = create_header(page_layout,"General",sub_header_size);
        create_lineedit_settings_entry(sub_layout,"Focus tag bar",KEY_FOCUS_TAG_BAR);
        create_lineedit_settings_entry(sub_layout,"Delete file",KEY_DELETE_FILE);
        create_lineedit_settings_entry(sub_layout,"Apply",KEY_APPLY);

        sub_layout = create_header(page_layout,"Tabs",sub_header_size);
        create_lineedit_settings_entry(sub_layout,"New Search tab",KEY_NEW_SEARCH);
        create_lineedit_settings_entry(sub_layout,"New Import tab",KEY_NEW_IMPORT);
        create_lineedit_settings_entry(sub_layout,"Close Tab",KEY_CLOSE_TAB);

        sub_layout = create_header(page_layout,"Search",sub_header_size);
        create_lineedit_settings_entry(sub_layout,"Invert file selection",KEY_INVERT_SELECTION);
        create_lineedit_settings_entry(sub_layout,"Focus search bar",KEY_FOCUS_SEARCH_BAR);
        create_lineedit_settings_entry(sub_layout,"Focus file grid",KEY_FOCUS_FILE_GRID);
        create_lineedit_settings_entry(sub_layout,"Refresh",KEY_REFRESH);

        sub_layout = create_header(page_layout,"File Viewer",sub_header_size);
        create_lineedit_settings_entry(sub_layout,"Focus file viewer",KEY_FOCUS_FILE_VIEWER);
        create_lineedit_settings_entry(sub_layout,"Next file",KEY_NEXT_FILE);
        create_lineedit_settings_entry(sub_layout,"Previous file",KEY_PREV_FILE);
        create_lineedit_settings_entry(sub_layout,"Zoom in",KEY_ZOOM_IN);
        create_lineedit_settings_entry(sub_layout,"Zoom out",KEY_ZOOM_OUT);
        create_lineedit_settings_entry(sub_layout,"Next page",KEY_NEXT_PAGE);
        create_lineedit_settings_entry(sub_layout,"Previous page",KEY_PREV_PAGE);
        create_lineedit_settings_entry(sub_layout,"Toggle Mute",KEY_TOGGLE_MUTE);
        create_lineedit_settings_entry(sub_layout,"Lock Overlay",KEY_LOCK_MEDIA_VIEWER_OVERLAY);

        page_layout->insertStretch(-1);
    }


    button_column->insertStretch(-1);
    config_buttons[0]->click();

    setLayout(vbox);

    QAction *apply_bind = create_binding(KEY_APPLY,"Insert",this);

    connect(apply_bind,&QAction::triggered,this,&configtab::apply);
    connect(apply_button, &QPushButton::clicked,this,&configtab::apply);
}

configtab::~configtab()
{
}

QVariant configtab::setting_value(Setting setting)
{
    QSettings set;
    QVariant value = set.value(settings[setting].name,settings[setting].default_val);
    QString data = value.toString();
    int pos = 0;
    if (settings[setting].validator == nullptr || settings[setting].validator->validate(data,pos) == QValidator::Acceptable)
        return value;
    else
        return settings[setting].default_val;
}

QString configtab::setting_name(Setting setting)
{
    return settings[setting].name;
}

QAction *configtab::create_binding(Setting set,QString name,QWidget *widget)
{
    QAction *action = new QAction(name,widget);
    action->setShortcut(QKeySequence(setting_value(set).toString()));
    widget->addAction(action);
    active_bindings.push_back({action,set});
    return action;
}

QBoxLayout *configtab::new_page(QString title)
{
    QPushButton *button = new QPushButton(title);
    QScrollArea *scroll_area = new QScrollArea();
    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *widget = new QWidget();

    QBoxLayout *sub_layout;

    button->setFlat(true);
    connect(button,&QPushButton::clicked,this,&configtab::set_page);

    config_buttons.push_back(button);
    button_column->addWidget(button);
    widget->setLayout(layout);


    sub_layout = create_header(layout,title,header_size);

    scroll_area->setFrameShadow(QFrame::Sunken);
    scroll_area->setFrameStyle(QFrame::StyledPanel);
    scroll_area->setWidget(widget);
    scroll_area->setWidgetResizable(true);

    pages->addWidget(scroll_area);

    return sub_layout;
}

void configtab::set_page()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    for (int i = 0; i < config_buttons.length(); i++) {
        if (config_buttons[i] == button){
            config_buttons[i]->setEnabled(false);
            pages->setCurrentIndex(i);
        } else {
            config_buttons[i]->setEnabled(true);
        }
    }
}


QBoxLayout *configtab::create_header(QBoxLayout *layout,QString text, int size)
{
    QLabel *header = new QLabel(text);
    QFont font = QFont();
    QVBoxLayout *sub_layout = new QVBoxLayout();

    font.setPointSize(size);
    header->setFont(font);
    layout->insertSpacing(-1,8);
    layout->addWidget(header);
    layout->insertSpacing(-1,3);

    sub_layout->setContentsMargins(left_margin,0,0,0);
    layout->addLayout(sub_layout);
    return sub_layout;

}


void configtab::create_checkbox_settings_entry(QBoxLayout *layout,QString display_text,Setting set)
{
    QCheckBox *checkbox = new QCheckBox(display_text);
    QFont font = QFont();
    font.setPointSize(font_size);

    checkbox->setFont(font);
    checkbox->setChecked(setting_value(set).toBool());

    settings_items.append({checkbox,BOOL,set});
    layout->addWidget(checkbox);
}

void configtab::create_lineedit_settings_entry(QBoxLayout *layout,QString display_text,Setting set)
{
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(display_text+":");
    QLineEdit *text_box = new QLineEdit();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addWidget(text_box);
    label->setFont(font);
    text_box->setText(setting_value(set).toString());

    settings_items.append({text_box,STRING,set});
    layout->addLayout(container);
}


void configtab::create_combo_settings_entry(QBoxLayout *layout,QString display_text,Setting set,QStringList combo_items)
{
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(display_text+":");
    QComboBox *combobox = new QComboBox();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addStretch(-1);
    container->addWidget(combobox);
    label->setFont(font);

    QString selected_item = setting_value(set).toString();
    combobox->addItems(combo_items);
    combobox->setCurrentText(selected_item);

    settings_items.append({combobox,COMBO,set});
    layout->addLayout(container);

}

void configtab::apply()
{
    QSettings settings;

    foreach(settings_item item,settings_items){
        QVariant value;
        if (item.type == STRING){
            QLineEdit *lineedit = qobject_cast<QLineEdit *>(item.widget);
            value = lineedit->text();
        } else if (item.type == BOOL){
            QCheckBox *checkbox = qobject_cast<QCheckBox *>(item.widget);
            value = checkbox->checkState();
        } else if (item.type == COMBO){
            QComboBox *checkbox = qobject_cast<QComboBox *>(item.widget);
            value = checkbox->currentText();
        }

        int pos = 0;
        QString str = value.toString();
        QValidator *validator = this->settings[item.setting].validator;

        if (validator == nullptr || validator->validate(str,pos) == QValidator::Acceptable)
            settings.setValue(setting_name(item.setting),value);
        else
            qDebug() << "Invalid value" << str.toUtf8() << "for setting" << setting_name(item.setting) << '\n';
    }

    foreach(SettingArrayList *arraylist, setting_array_items)
        arraylist->apply();

    update_bindings();

    emit settings_changed();
}

QVector<configtab::color_setting> configtab::get_tag_colors()
{
    return get_colors("tagcolor");
}
QVector<configtab::color_setting> configtab::get_file_colors()
{
    return get_colors("filecolor");
}

QVector<configtab::color_setting> configtab::get_colors(QString group)
{
    QVector<color_setting> colors;

    QSettings settings;

    int length = settings.beginReadArray(group);
    for(int i = 0; i < length; i++){
        color_setting color;
        settings.setArrayIndex(i);
        color.starts_with = settings.value("prefix").toString();
        color.color = QColor(settings.value("color").toString());
        colors.append(color);
    }


    settings.endArray();

    return colors;
}

void configtab::update_bindings()
{
    for (int i = active_bindings.length()-1; i >= 0; i--) {
        if (active_bindings[i].action.isNull()){
            active_bindings.remove(i);
        } else {
            active_bindings[i].action->setShortcut(QKeySequence(setting_value(active_bindings[i].set).toString()));
        }
    }
}


SettingArrayList::SettingArrayList(QWidget *parent,QString array_name,QVector<Key> keys) : QFrame(parent), array_name(array_name),keys(keys)
{
    QVBoxLayout *main_layout = new QVBoxLayout();
    QHBoxLayout *button_container = new QHBoxLayout();
    QPushButton *add_button = new QPushButton("+");

    entry_container = new QVBoxLayout();

    setFrameStyle(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setLayout(main_layout);

    main_layout->addLayout(entry_container);
    button_container->addStretch(-1);
    button_container->addWidget(add_button);
    main_layout->addLayout(button_container);

    connect(add_button,SIGNAL(clicked()),this,SLOT(add_entry()));

    QSettings settings;
    int length = settings.beginReadArray(array_name);

    for (int i = 0; i < length; i++)
        add_entry(i);

    settings.endArray();

}

void
SettingArrayList::add_entry(int existing_entry)
{
    QWidget *container = new QWidget();
    QHBoxLayout *hbox = new QHBoxLayout(container);
    QPushButton *delete_button = new QPushButton("-");

    QSettings settings;
    Entry entry;

    settings.beginReadArray(array_name);
    entry.delete_button = delete_button;

    foreach(Key key,keys){
        QLineEdit *lineedit = new QLineEdit();
        lineedit->setPlaceholderText(key.placeholder);
        if (key.isColor){
            lineedit->setInputMask("HHHHHH");
            connect(lineedit,&QLineEdit::textChanged,this,&SettingArrayList::update_color);
        }

        if (existing_entry >= 0){
            settings.setArrayIndex(existing_entry);
            if (key.isColor)
                lineedit->setText(settings.value(key.name).toString().remove(0,1));
            else
                lineedit->setText(settings.value(key.name).toString());
        }

        hbox->addWidget(lineedit);
        entry.data.append(lineedit);
    }
    entries.append(entry);

    settings.endArray();
    hbox->addWidget(delete_button);
    entry_container->addWidget(container);
    hbox->setContentsMargins(0,0,0,0);
    container->setContentsMargins(0,0,0,0);

    connect(delete_button,&QPushButton::clicked,this,&SettingArrayList::delete_entry);
    connect(delete_button,&QPushButton::clicked,container,&SettingArrayList::deleteLater);
}

void SettingArrayList::update_color(const QString &text)
{
    QLineEdit* lineedit = qobject_cast<QLineEdit*>(sender());
    if (text.length() != 3 && text.length() != 6){
        lineedit->setStyleSheet("color:#ff0000");
        return;
    }

    QColor c = "#"+text;
    lineedit->setStyleSheet("color:"+c.name());

}

void SettingArrayList::delete_entry()
{
    QPushButton* del_button = qobject_cast<QPushButton*>(sender());

    for (int i = 0; i < entries.length(); i++)
        if (entries[i].delete_button == del_button)
            entries.remove(i);

}

void SettingArrayList::apply()
{
    QSettings settings;
    settings.beginWriteArray(array_name);
    for (int i = 0; i < entries.length(); i++){
        settings.setArrayIndex(i);
        for (int j = 0; j < keys.length(); j++){
            QString str = entries[i].data.at(j)->text();
            settings.setValue(keys[j].name,str);
            if (keys[j].isColor)
                settings.setValue(keys[j].name,"#"+str);
            else
                settings.setValue(keys[j].name,str);
        }
    }

    settings.endArray();

}
