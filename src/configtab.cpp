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
#include <QKeyEvent>
#include <QKeySequenceEdit>

const configtab::Setting_Item configtab::settings[] = {
    {TAG_HISTORY,"General","Use tag history",
     "Whether or not pressing up in a line edit will show previously entered tags",
     "use_tag_history",true,BOOL,
     nullptr},

    {GIFS_AS_VIDEO,"General","Use video player for gifs",
     "Use video player for gifs instead of image viewer",
     "treat_gifs_as_video",false,BOOL,
     nullptr},

    {TIMESTAMP,"General","Timestamp format",
     "",
     "timestamp_format","MM/dd/yy",STRING,
     nullptr},

    {SHOW_REMAINING_TIME,"General","Show remaining time for videos instead of duration",
     "",
     "show_video_remaining_time",false,BOOL,
     nullptr},

    {THEME,"General","Theme",
     "",
     "theme",QStringList(std::initializer_list<QString>({"Default","Dark Theme"})),COMBO,
     nullptr},

    {HIGHLIGHT_SIMMILAR_TAGS,"Tags.Tag Highlighting","Highlight tags similar to the tag currently being typed",
     "",
    "highlight-similar-tags",true,BOOL,
    nullptr},

    {HIGHLIGHT_COLOR,"Tags.Tag Highlighting","Simmilar tag highlight color",
     "Highlight color for tags that start with the text currently being typed",
    "highlight-color",QColor(100,100,230),COLOR,
    nullptr},

    {CHILD_HIGHLIGHT_COLOR,"Tags.Tag Highlighting","Related tag highlight color",
     "Highlight color for tags with related tags that start with the text currently being typed",
    "highlight-related-color",QColor(230,100,230),COLOR,
    nullptr},

    {TAG_COLOR,"Tags.Tag Color","Tag",
     "",
     "tagcolor",QVariant(),COLOR_ARRAY,
     nullptr},

    {THUMBNAIL_DIR,"Files.Thumbnails","Location used for thumbnail storage",
     "Directory to store file thumbnails",
     "thumbnail_dir","~/.cache/PyrosQT",STRING,
     nullptr},

    {FILE_COLOR,"Files.File Border","Mime/Type",
     "",
     "filecolor",QVariant(),COLOR_ARRAY,
     nullptr},

    {THUMBNAIL_SIZE,"Files.Thumbnails","Thumbnails size",
     "",
     "thumbnail_size","256",STRING,
     new QIntValidator(25, 999)},

    {USE_INTERNAL_IMAGE_THUMBNAILER,"Files.Thumbnails","Use internal image thumbnailer",
     "If disabled will use an exteral image thumnailer (if one is available)",
     "use_interal_image-thumbnailer",true,BOOL,
     nullptr},

    {USE_CBZ_THUMBNAILER,"Files.Thumbnails","Use internal cbz/zip thumbnailer",
     "If disabled will use an exteral cbz/zip thumnailer (if one is available)",
     "use_interal_cbz-thumbnailer",true,BOOL,
     nullptr},

    {CBZ_THUMB_PAGE_COUNT,"Files.Thumbnails","Number of pages to show in cbz/zip thumbnail",
     "",
     "cbz_thumbnail_pages","3",STRING,
     new QIntValidator(1, 5)},

    {USE_EXTERNAL_THUMBNAILER,"Files.Thumbnails","Use external thumbnailers from /usr/share/thumbnailers",
     "",
     "use_exernal_thumbnailer",true,BOOL,
     nullptr},

    {KEY_FOCUS_TAG_BAR,"Key Binds.General","Focus tag bar",
     "",
     "keybind/focus-tag-bar","i",KEY,
     nullptr},

    {KEY_DELETE_FILE,"Key Binds.General","Delete file",
     "",
     "keybind/delete-file","CTRL+del",KEY,
     nullptr},

    {KEY_APPLY,"Key Binds.Tabs","Apply",
     "",
     "keybind/apply","CTRL+Return",KEY,
     nullptr},

    {KEY_NEW_SEARCH,"Key Binds.Tabs","New Search tab",
     "",
     "keybind/new-search-tab","CTRL+t",KEY,
     nullptr},

    {KEY_NEW_IMPORT,"Key Binds.Tabs","New Import tab",
     "",
     "keybind/new-import-tab","CTRL+i",KEY,
     nullptr},

    {KEY_CLOSE_TAB,"Key Binds.Tabs","Close tab",
     "",
     "keybind/close-tab","CTRL+w",KEY,
     nullptr},

    {KEY_INVERT_SELECTION,"Key Binds.Search","Invert file selection",
     "",
     "keybind/invert-selection","SHIFT+i",KEY,
     nullptr},

    {KEY_FOCUS_SEARCH_BAR,"Key Binds.Search","Focus search bar",
     "",
     "keybind/focus-search-bar","a",KEY,
     nullptr},

    {KEY_FOCUS_FILE_GRID,"Key Binds.Search","Focus file grid",
     "",
     "keybind/focus-file-grid","CTRL+f",KEY,
     nullptr},

    {KEY_REFRESH,"Key Binds.Search","refresh",
     "",
     "keybind/refresh","CTRL+r",KEY,
     nullptr},

    {KEY_FOCUS_FILE_VIEWER,"Key Binds.File Viewer","Focus File Viewer",
     "",
     "keybind/focus-file-viewer","CTRL+f",KEY,
     nullptr},

    {KEY_NEXT_FILE,"Key Binds.File Viewer","Next file",
     "",
     "keybind/next-file","CTRL+n",KEY,
     nullptr},

    {KEY_PREV_FILE,"Key Binds.File Viewer","Previous file",
     "",
     "keybind/prev-file","CTRL+p",KEY,
     nullptr},

    {KEY_ZOOM_IN,"Key Binds.File Viewer","Zoom in",
     "",
     "keybind/zoom-in","CTRL++",KEY,
     nullptr},

    {KEY_ZOOM_OUT,"Key Binds.File Viewer","Zoom out",
     "",
     "keybind/zoom-out","CTRL+-",KEY,
     nullptr},

    {KEY_NEXT_PAGE,"Key Binds.File Viewer","Next page",
     "",
     "keybind/next-page",">",KEY,
     nullptr},

    {KEY_PREV_PAGE,"Key Binds.File Viewer","Previous page",
     "",
     "keybind/prev-page","<",KEY,
     nullptr},

    /*{KEY_FULLSCREEN,"Key Binds.File Viewer","Toggle Fullscreen",
     "keybind/fullscreen","CTRL+SHIFT+f",KEY,
     nullptr},*/

    {KEY_TOGGLE_MUTE,"Key Binds.File Viewer","Toggle Mute",
     "",
     "keybind/toggle-mute","m",KEY,
     nullptr},

    {KEY_LOCK_MEDIA_VIEWER_OVERLAY,"Key Binds.File Viewer","Lock Overlay to stop it from automatically hiding",
     "",
     "keybind/lock-media-overlay","CTRL+l",KEY,
     nullptr},
};


QVector<configtab::binding> configtab::active_bindings;

configtab::Setting_Group::Setting_Group(QString name,Setting_Group *parent):name(name),parent(parent){}
bool configtab::Setting_Group::get_subgroup(QString name,configtab::Setting_Group *&group)
{
    for (int i = 0; i < sub_groups.count(); i++) {
        if (sub_groups.at(i).name == name){
            group = &sub_groups[i];
            return false;
        }
    }

    sub_groups.append(Setting_Group(name,this));
    group = &sub_groups.last();
    return true;
}

void configtab::Setting_Group::apply()
{
    QSettings settings;
    QVariant value;
    foreach(Setting_Group_Item item,items){
        switch (item.item->type){
        case STRING:{
            QLineEdit *lineedit = qobject_cast<QLineEdit *>(item.widget);
            value = lineedit->text();
            break;
        }
        case KEY:{
            QKeySequenceEdit *kse = qobject_cast<QKeySequenceEdit *>(item.widget);
            value = kse->keySequence().toString();
            break;
        }
        case COLOR:{
            QLineEdit *lineedit = qobject_cast<QLineEdit *>(item.widget);
            value = lineedit->text();
            break;
        }
        case BOOL:{
            continue;
            QCheckBox *checkbox = qobject_cast<QCheckBox *>(item.widget);
            value = checkbox->checkState();
            break;
        }
        case COMBO:{
            QComboBox *combobox = qobject_cast<QComboBox *>(item.widget);
            value = combobox->currentText();
            break;
        }
        case COLOR_ARRAY:
            SettingArrayList *arraylist = qobject_cast<SettingArrayList *>(item.widget);
            arraylist->apply();
            break;
        }

        int pos = 0;
        QString str = value.toString();
        QValidator *validator = item.item->validator;

        if (validator == nullptr || validator->validate(str,pos) == QValidator::Acceptable)
            settings.setValue(item.item->key,value);
        else
            qDebug() << "Invalid value" << str.toUtf8() << "for setting" << item.item->key << '\n';
    }

    foreach(Setting_Group sub_group,sub_groups)
        sub_group.apply();
}

configtab::configtab(QTabWidget *parent) :
    Tab(parent)
{
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

    load_setting_groups();

    button_column->insertStretch(-1);
    setting_group_top.sub_groups.at(0).button->click();

    setLayout(vbox);

    QAction *apply_bind = create_binding(KEY_APPLY,"Insert",this);

    connect(apply_bind,&QAction::triggered,this,&configtab::apply);
    connect(apply_button, &QPushButton::clicked,this,&configtab::apply);
}

configtab::~configtab(){}

QVariant configtab::setting_value(Setting setting_id)
{
    QSettings set;
    QVariant value;
    QString data;
    Setting_Item setting;
    for (unsigned i= 0; i < sizeof(settings)/sizeof(*settings); i++) {
        if (settings[i].id == setting_id){
            value = set.value(settings[i].key,settings[i].default_val);
            data = value.toString();
            int pos = 0;
            if (settings[i].validator == nullptr || settings[i].validator->validate(data,pos) == QValidator::Acceptable)
                return value;
            else
                return settings[i].default_val;

        }
    }
    return QVariant();
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

void configtab::new_page(Setting_Group *group)
{
    QPushButton *button = new QPushButton(group->name);
    QScrollArea *scroll_area = new QScrollArea();
    QVBoxLayout *layout = new QVBoxLayout();
    QWidget *widget = new QWidget();

    QBoxLayout *sub_layout;

    button->setFlat(true);
    connect(button,&QPushButton::clicked,this,&configtab::set_page);

    group->button = button;
    button_column->addWidget(button);
    widget->setLayout(layout);


    sub_layout = create_header(layout,group->name,header_size);

    scroll_area->setFrameShadow(QFrame::Sunken);
    scroll_area->setFrameStyle(QFrame::StyledPanel);
    scroll_area->setWidget(widget);
    scroll_area->setWidgetResizable(true);

    pages->addWidget(scroll_area);

    group->layout = sub_layout;
}

void configtab::set_page()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    for (int i = 0; i < setting_group_top.sub_groups.length(); i++) {
        Setting_Group group = setting_group_top.sub_groups.at(i);
        if (group.button == button){
            group.button->setEnabled(false);
            pages->setCurrentIndex(i);
        } else {
            group.button->setEnabled(true);
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

void configtab::create_checkbox_settings_entry(QBoxLayout *layout,Setting_Group_Item &item)
{
    QCheckBox *checkbox = new QCheckBox(item.item->name);
    QFont font = QFont();
    font.setPointSize(font_size);

    checkbox->setToolTip(item.item->tooltip);
    checkbox->setFont(font);
    checkbox->setChecked(setting_value(item.item->id).toBool());

    layout->addWidget(checkbox);
    item.widget = checkbox;
}
template <typename T>
void configtab::create_lineedit_settings_entry(QBoxLayout *layout, Setting_Group_Item &item)
{
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(item.item->name+":");
    T *text_box = new T();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addWidget(text_box);
    label->setToolTip(item.item->tooltip);
    label->setFont(font);
    text_box->setText(setting_value(item.item->id).toString());

    layout->addLayout(container);
    item.widget = text_box;
}

void configtab::create_keybind_settings_entry(QBoxLayout *layout, Setting_Group_Item &item)
{
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(item.item->name+":");
    QKeySequenceEdit *text_box = new QKeySequenceEdit();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addWidget(text_box);
    label->setFont(font);
    label->setToolTip(item.item->tooltip);
    //text_box->setText(setting_value(item.item->id).toString());
    text_box->setKeySequence(setting_value(item.item->id).toString());

    layout->addLayout(container);
    item.widget = text_box;
}
void configtab::create_combo_settings_entry(QBoxLayout *layout,Setting_Group_Item &item)
{
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(item.item->name+":");
    QComboBox *combobox = new QComboBox();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addStretch(-1);
    container->addWidget(combobox);
    label->setFont(font);
    label->setToolTip(item.item->tooltip);

    QStringList combo_items = item.item->default_val.toStringList();
    combobox->addItems(combo_items);
    combobox->setCurrentText(setting_value(item.item->id).toString());

    layout->addLayout(container);
    item.widget = combobox;

}

void configtab::apply()
{
    QSettings settings;

    setting_group_top.apply();

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
        color.glob = settings.value("prefix").toString().toLower();
        color.color = QColor(settings.value("color").toString());
        colors.append(color);
    }


    settings.endArray();

    return colors;
}

void configtab::update_bindings()
{
    for (int i = active_bindings.length()-1; i >= 0; i--) {
        if (active_bindings[i].action.isNull())
            active_bindings.remove(i);
        else
            active_bindings[i].action->setShortcut(QKeySequence(setting_value(active_bindings[i].set).toString().toLower(),QKeySequence::SequenceFormat::PortableText));
    }
}

void configtab::disable_bindings()
{
    for (int i = active_bindings.length()-1; i >= 0; i--) {
        if (active_bindings[i].action.isNull())
            active_bindings.remove(i);
         else
            active_bindings[i].action->setShortcut(QKeySequence(""));
    }
}


void configtab::load_setting_groups()
{
    Setting_Group *current_group = &setting_group_top;
    int depth = 0;

    for (unsigned i= 0; i < sizeof(settings)/sizeof(*settings); i++) {
        reset:
        if (settings[i].group == current_group->name){
            current_group->items.append({&(settings[i]),nullptr});
            switch (settings[i].type){
            case BOOL:
                create_checkbox_settings_entry(current_group->layout,current_group->items.last());
                break;
            case STRING:
                create_lineedit_settings_entry<QLineEdit>(current_group->layout,current_group->items.last());
                break;
            case KEY:
                create_keybind_settings_entry(current_group->layout,current_group->items.last());
                break;
            case COMBO:
                create_combo_settings_entry(current_group->layout,current_group->items.last());
                break;
            case COLOR:
                create_lineedit_settings_entry<ColorLineEdit>(current_group->layout,current_group->items.last());
                break;
            case COLOR_ARRAY:
                SettingArrayList *arraylist = new SettingArrayList(current_group->layout->widget(),settings[i].key,{{"prefix",settings[i].name,false},{"color","Color",true}});
                current_group->layout->addWidget(arraylist);
                current_group->items.last().widget = arraylist;
                break;

            }
        } else if (settings[i].group.startsWith(current_group->name+'.') || current_group->name.isEmpty()){
            QStringList groups = settings[i].group.split(".");
            depth++;
            QString subgroup_name;

            if (current_group->name.isEmpty()){
                if (groups.length() == 0)
                    subgroup_name = settings[i].group;
                else
                    subgroup_name = groups.at(depth-1);
            } else {
                subgroup_name = current_group->name;
                subgroup_name.append(".");
                subgroup_name.append(groups.at(depth-1));
            }

            if (current_group->get_subgroup(subgroup_name,current_group)){
                if (depth == 1)
                    new_page(current_group);
                else
                    current_group->layout = create_header(current_group->parent->layout,groups.last(),sub_header_size);
            }
            goto reset;
        } else {
            depth--;
            if (depth == 0)
                current_group->layout->insertStretch(-1);
            current_group = current_group->parent;
            goto reset;
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
        QLineEdit *lineedit;

        if (key.isColor)
            lineedit = new ColorLineEdit();
        else
            lineedit = new QLineEdit();

        lineedit->setPlaceholderText(key.placeholder);

        if (existing_entry >= 0){
            settings.setArrayIndex(existing_entry);
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
            settings.setValue(keys[j].name,str);
        }
    }

    settings.endArray();

}

ColorLineEdit::ColorLineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this,&QLineEdit::textChanged,this,&ColorLineEdit::update_color);
    connect(this,&QLineEdit::textEdited,this,&ColorLineEdit::update_color_text);
}

ColorLineEdit::~ColorLineEdit(){}

void ColorLineEdit::update_color_text(const QString &text)
{
    QString new_str = "#";
    for	(int i = 0; i < text.length();i++){
        if ((text.at(i).toLower() >= 'a' && text.at(i).toLower() <= 'f') ||
                (text.at(i) >= '0' && text.at(i) <= '9'))
            new_str.append(text.at(i).toLower());
        if (new_str.length() >= 7)
            break;
    }

    setText(new_str);
}

void ColorLineEdit::update_color(const QString &)
{

    if (text().length() != 4 && text().length() != 7)
        setStyleSheet("color:#ff0000");
    else
        setStyleSheet("color:"+QColor(text()).name());
}
