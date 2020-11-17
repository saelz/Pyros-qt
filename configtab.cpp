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

    QSettings settings;
    ui->theme->setCurrentText(settings.value("theme","Default").toString());
    ui->use_tag_history->setChecked(settings.value("use_tag_history",true).toBool());
    ui->use_video_play_with_gifs->setChecked(settings.value("treat_gifs_as_video",false).toBool());
    ui->timestamp_format->setText(settings.value("timestamp_format","MM/dd/yy").toString());

    settings.beginGroup("tagcolor");
    QStringList colored_tags = settings.allKeys();
    foreach(QString colored_prefix,colored_tags){
        QColor color = settings.value(colored_prefix).value<QColor>();
        tag_colors.append(new color_entry(ui->tag_color_box,"Tag",colored_prefix,color.name()));
    }
    settings.endGroup();

    settings.beginGroup("filecolor");
    colored_tags = settings.allKeys();
    foreach(QString colored_prefix,colored_tags){
        QColor color = settings.value(colored_prefix).value<QColor>();
        file_colors.append(new color_entry(ui->file_border_box,"Mime/Type",colored_prefix,color.name()));
    }
    settings.endGroup();
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
    const QString theme = ui->theme->currentText();
    settings.setValue("theme",theme);
    settings.setValue("use_tag_history",ui->use_tag_history->checkState());
    settings.setValue("treat_gifs_as_video",ui->use_video_play_with_gifs->checkState());
    settings.setValue("timestamp_format",ui->timestamp_format->text());

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

void
color_entry::update_color(const QString &text)
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
