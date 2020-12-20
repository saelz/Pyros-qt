#include "configtab.h"
#include "ui_configtab.h"

#include <QSettings>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QDebug>





configtab::configtab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::configtab)
{
    ui->setupUi(this);
    connect(ui->tags_button,   &QPushButton::clicked,this,&configtab::set_tag_page);
    connect(ui->files_button,  &QPushButton::clicked,this,&configtab::set_file_page);
    connect(ui->general_button,&QPushButton::clicked,this,&configtab::set_general_page);
    connect(ui->apply_button,  &QPushButton::clicked,this,&configtab::apply);

    connect(ui->more_tag_color_button,   &QPushButton::clicked,this,&configtab::new_color_tag);
    connect(ui->more_file_border_button, &QPushButton::clicked,this,&configtab::new_file_border);

    // GENERAL PAGE
    QSettings settings;
    ui->theme->setCurrentText(settings.value("theme","Default").toString());

    init_settings_entry("use-tag-history",ui->use_tag_history,true);
    init_settings_entry("use-tag-history",ui->use_tag_history,true);
    init_settings_entry("treat-gifs-as-video",ui->use_video_play_with_gifs,false);
    init_settings_entry("timestamp-format",ui->timestamp_format,"MM/dd/yy");

    // TAG PAGE
    create_color_entries(ui->tag_color_box,"tagcolor","Tag",tag_colors,settings);


    // FILE PAGE
    create_color_entries(ui->file_border_box,"filecolor","Mime/Type",file_colors,settings);

    init_settings_entry("use-interal-image-thumbnailer",ui->use_internal_image_thumbnailer,true);
    init_settings_entry("use-interal-cbz-thumbnailer",ui->use_internal_cbz_thumbnailer,true);
    init_settings_entry("use-exernal-thumbnailer",ui->use_external_thumbnailers,true);

    set_general_page();
}

configtab::~configtab()
{
    delete ui;

    for(int i = 0;i < file_colors.count();i++)
        if (!file_colors[i].isNull())
            delete file_colors[i].data();
    for(int i = 0;i < tag_colors.count();i++)
        if (!tag_colors[i].isNull())
            delete tag_colors[i].data();
}


void configtab::create_color_entries(QVBoxLayout *layout,
                                     QString setting_gourp,QString placeholder,QVector<QPointer<color_entry>> list,
                                     QSettings &settings)
{
    settings.beginGroup(setting_gourp);
    QStringList entries = settings.allKeys();
    foreach(QString colored_prefix,entries){
        QColor color = settings.value(colored_prefix).value<QColor>();
        list.append(new color_entry(layout,placeholder,colored_prefix,color.name()));
    }
    settings.endGroup();

}

void configtab::init_settings_entry(QString setting_name,QLineEdit *widget,QString default_str)
{
    QSettings settings;
    widget->setText(settings.value(setting_name,default_str).toString());
    settings_items.append({widget,setting_name,STRING});
}
void configtab::init_settings_entry(QString setting_name,QCheckBox *widget,bool default_state)
{
    QSettings settings;
    widget->setChecked(settings.value(setting_name,default_state).toBool());
    settings_items.append({widget,setting_name,BOOL});
}

void configtab::enable_all_buttons()
{
    ui->tags_button->setEnabled(true);
    ui->files_button->setEnabled(true);
    ui->general_button->setEnabled(true);

}

void configtab::set_tag_page()
{
    ui->stackedWidget->setCurrentIndex(2);
    enable_all_buttons();
    ui->tags_button->setEnabled(false);

}

void configtab::set_file_page()
{
    ui->stackedWidget->setCurrentIndex(1);
    enable_all_buttons();
    ui->files_button->setEnabled(false);
}

void configtab::set_general_page()
{
    ui->stackedWidget->setCurrentIndex(0);
    enable_all_buttons();
    ui->general_button->setEnabled(false);
}

void configtab::apply()
{
    QSettings settings;
    settings.setValue("theme",ui->theme->currentText());

    foreach(settings_item item,settings_items){
        QVariant value;
        if (item.type == STRING){
            QLineEdit *lineedit = qobject_cast<QLineEdit *>(item.widget);
            value = lineedit->text();
        } else if (item.type == BOOL){
            QCheckBox *checkbox = qobject_cast<QCheckBox *>(item.widget);
            value = checkbox->checkState();
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
        qDebug() << entry->entry->text() << entry->color->text();
        settings.setValue(entry->entry->text(),c);
    }
}

void configtab::new_color_tag()
{
        tag_colors.append(new color_entry(ui->tag_color_box,"Tag"));
}

void configtab::new_file_border()
{
        file_colors.append(new color_entry(ui->file_border_box,"Mime/Type"));
}

color_entry::color_entry(QVBoxLayout *parent,QString placeholder,
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
