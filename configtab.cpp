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


configtab::configtab(QWidget *parent) :
    QWidget(parent)
{

    QVBoxLayout *page_layout;

    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox = new QHBoxLayout();
    QHBoxLayout *button_layout = new QHBoxLayout();
    QPushButton *apply_button = new QPushButton("Apply");

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
        create_settings_entry(page_layout,"Theme","theme",{"Default" ,"Dark Theme"});
        create_checkbox_settings_entry(page_layout,"Use tag history","use_tag_history",true);
        create_checkbox_settings_entry(page_layout,"Use video player for gifs","treat_gifs_as_video",false);
        create_settings_entry(page_layout,"Timestamp format","timestamp_format","MM/dd/yy");
        page_layout->insertStretch(-1);
    }

    page_layout = new_page("Tags");
    {
        create_color_entries(page_layout,"Tag Color","tagcolor","Tag",tag_colors);
        page_layout->insertStretch(-1);
    }

    page_layout = new_page("Files");
    {
        create_color_entries(page_layout,"File Border Color","filecolor","Mime/Type",file_colors);
        create_header(page_layout,"Thumbnails",sub_header_size);
        create_checkbox_settings_entry(page_layout,"Use interal image thumbnailer","use_interal_image-thumbnailer",true);
        create_checkbox_settings_entry(page_layout,"Use interal cbz/zip thumbnailer","use_interal_cbz-thumbnailer",true);
        create_checkbox_settings_entry(page_layout,"Use external thumbnailers from /usr/share/thumbnailers/","use_exernal_thumbnailer",true);

        page_layout->insertStretch(-1);
    }

    button_column->insertStretch(-1);
    config_buttons[0]->click();

    setLayout(vbox);

    connect(apply_button,  &QPushButton::clicked,this,&configtab::apply);
}

configtab::~configtab()
{
    for(int i = 0;i < file_colors.count();i++)
        if (!file_colors[i].isNull())
            delete file_colors[i].data();
    for(int i = 0;i < tag_colors.count();i++)
        if (!tag_colors[i].isNull())
            delete tag_colors[i].data();
}


QVBoxLayout *configtab::new_page(QString title)
{
    QPushButton *button = new QPushButton(title);
    QScrollArea *scroll_area = new QScrollArea();
    QVBoxLayout *layout = new QVBoxLayout();

    button->setFlat(true);
    connect(button,&QPushButton::clicked,this,&configtab::set_page);

    config_buttons.push_back(button);
    button_column->addWidget(button);
    pages->addWidget(scroll_area);
    scroll_area->setLayout(layout);
    scroll_area->setFrameShadow(QFrame::Sunken);
    scroll_area->setFrameStyle(QFrame::StyledPanel);

    create_header(layout,title,header_size);

    return layout;
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


void configtab::create_header(QBoxLayout *layout,QString text, int size)
{
    QLabel *header = new QLabel(text);
    QFont font = QFont();
    font.setPointSize(size);
    header->setFont(font);
    layout->addWidget(header);
    layout->insertSpacing(-1,10);

}

void configtab::create_color_entries(QBoxLayout *layout,QString header,
                                     QString setting_group,QString placeholder,QVector<QPointer<color_entry>> &list)
{
    QFrame *frame = new QFrame();
    QVBoxLayout *main_layout = new QVBoxLayout();
    QVBoxLayout *entry_container = new QVBoxLayout();
    QHBoxLayout *button_container = new QHBoxLayout();
    QPushButton *add_button = new QPushButton("+");

    create_header(main_layout,header,sub_header_size);
    frame->setFrameStyle(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);
    layout->addWidget(frame);
    frame->setLayout(main_layout);
    main_layout->addLayout(entry_container);
    button_container->addStretch(-1);
    button_container->addWidget(add_button);
    main_layout->addLayout(button_container);

    entry_buttons.append({add_button,entry_container,placeholder,&list});
    connect(add_button,&QPushButton::clicked,this,&configtab::new_color_entry);

    QSettings settings;
    settings.beginGroup(setting_group);
    QStringList entries = settings.allKeys();
    foreach(QString colored_prefix,entries){
        QColor color = settings.value(colored_prefix).value<QColor>();
        list.append(new color_entry(entry_container,placeholder,colored_prefix,color.name()));
    }
    settings.endGroup();

}

void configtab::new_color_entry()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    foreach(color_entry_button entry_button,entry_buttons){
        if (entry_button.button == button){
            entry_button.entries->append(new color_entry(entry_button.entry_container,entry_button.placeholder));
        }
    }

}

void configtab::create_checkbox_settings_entry(QBoxLayout *layout,QString display_text,QString setting_name,bool default_state)
{
    QSettings settings;
    QCheckBox *checkbox = new QCheckBox(display_text);
    QFont font = QFont();
    font.setPointSize(font_size);

    checkbox->setFont(font);
    checkbox->setChecked(settings.value(setting_name,default_state).toBool());

    settings_items.append({checkbox,setting_name,BOOL});
    layout->addWidget(checkbox);
}

void configtab::create_settings_entry(QBoxLayout *layout,QString display_text,QString setting_name,QString default_text)
{
    QSettings settings;
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(display_text+":");
    QLineEdit *text_box = new QLineEdit();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addStretch(-1);
    container->addWidget(text_box);
    label->setFont(font);
    text_box->setText(settings.value(setting_name,default_text).toString());

    settings_items.append({text_box,setting_name,STRING});
    layout->addLayout(container);
}


void configtab::create_settings_entry(QBoxLayout *layout,
                      QString display_text,QString setting_name,
                      QStringList combo_items)
{
    QSettings settings;
    QHBoxLayout *container = new QHBoxLayout();
    QLabel *label = new QLabel(display_text+":");
    QComboBox *combobox = new QComboBox();
    QFont font = QFont();
    font.setPointSize(font_size);

    container->addWidget(label);
    container->addStretch(-1);
    container->addWidget(combobox);
    label->setFont(font);

    QString selected_item = settings.value(setting_name,combo_items[0]).toString();
    combobox->addItems(combo_items);
    combobox->setCurrentText(selected_item);

    settings_items.append({combobox,setting_name,COMBO});
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
        settings.setValue(item.setting_name,value);
    }

    settings.beginGroup("tagcolor");
    apply_color_entries(settings,tag_colors);
    settings.endGroup();
    settings.beginGroup("filecolor");
    apply_color_entries(settings,file_colors);
    settings.endGroup();


    emit settings_changed();
}

void configtab::apply_color_entries(QSettings &settings,QVector<QPointer<color_entry>> &entries){
    settings.remove("");
    for(int i = 0;i < entries.count();i++){
        if (entries[i].isNull())
            continue;
        color_entry *entry = entries[i].data();
        QColor c = "#"+entry->color->text();
        settings.setValue(entry->entry->text(),c);
    }
}

color_entry::color_entry(QBoxLayout *parent,QString placeholder,
               QString item,QString hex)
{

        hbox = new QHBoxLayout();
        entry = new QLineEdit(item);
        color = new QLineEdit(hex);
        delete_button = new QPushButton("-");
        entry->setPlaceholderText(placeholder);
        color->setInputMask("HHHHHH");
        color->setPlaceholderText("Hex Color");

        hbox->addWidget(entry);
        hbox->addWidget(color);
        hbox->addWidget(delete_button);
        parent->addLayout(hbox);

        connect(delete_button,&QPushButton::clicked,this,&color_entry::deleteLater);
        connect(color,&QLineEdit::textChanged,this,&color_entry::update_color);
        update_color(hex.remove(0,1));
}

void color_entry::update_color(const QString &text)
{
    if (text.length() != 3 && text.length() != 6){
        entry->setStyleSheet("");
        color->setStyleSheet("color:#ff0000");
        return;
    }

    QColor c = "#"+text;
    color->setStyleSheet("");
    entry->setStyleSheet("color:"+c.name());

}

color_entry::~color_entry()
{
    delete delete_button;
    delete color;
    delete entry;
    delete hbox;
}
