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
    connect(ui->tags_button,  &QPushButton::clicked,this,&configtab::set_tag_page);
    connect(ui->files_button,  &QPushButton::clicked,this,&configtab::set_file_page);
    connect(ui->general_button,&QPushButton::clicked,this,&configtab::set_general_page);
    connect(ui->apply_button,  &QPushButton::clicked,this,&configtab::apply);

    connect(ui->more_tag_color_button,   &QPushButton::clicked,this,&configtab::new_color_tag);
    connect(ui->more_file_border_button, &QPushButton::clicked,this,&configtab::new_file_border);

    QSettings settings;
    ui->theme->setCurrentText(settings.value("theme","Default").toString());
    ui->use_tag_history->setChecked(settings.value("use_tag_history",true).toBool());
    ui->use_video_play_with_gifs->setChecked(settings.value("treat_gifs_as_video",false).toBool());

    settings.beginGroup("tagcolor");
    QStringList colored_tags = settings.allKeys();
    foreach(QString colored_prefix,colored_tags){
        QColor color = settings.value(colored_prefix).value<QColor>();
        ui->tag_color_box->addLayout(new_color_entry(colored_prefix,color.name(),"Tag"));
    }
    settings.endGroup();

    settings.beginGroup("filecolor");
    colored_tags = settings.allKeys();
    foreach(QString colored_prefix,colored_tags){
        QColor color = settings.value(colored_prefix).value<QColor>();
        ui->file_border_box->addLayout(new_color_entry(colored_prefix,color.name(),"Mime/Type"));
    }
    settings.endGroup();
}

configtab::~configtab()
{
    delete ui;
}

void configtab::set_tag_page()
{
    ui->stackedWidget->setCurrentIndex(2);
}


void configtab::set_file_page()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void configtab::set_general_page()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void configtab::apply()
{
    QSettings settings;
    const QString theme = ui->theme->currentText();
    settings.setValue("theme",theme);
    settings.setValue("use_tag_history",ui->use_tag_history->checkState());
    settings.setValue("treat_gifs_as_video",ui->use_video_play_with_gifs->checkState());

    settings.beginGroup("tagcolor");
    apply_color_entries(ui->tag_color_box,settings);
    settings.endGroup();
    settings.beginGroup("filecolor");
    apply_color_entries(ui->file_border_box,settings);
    settings.endGroup();

    emit settings_changed();
}

void configtab::apply_color_entries(QVBoxLayout *inital_layout,QSettings &settings){
    settings.remove("");
    for	(int i = 0; i < inital_layout->count();i++){
        QLayout *l = inital_layout->itemAt(i)->layout();

        QWidget *tag_widget = l->itemAt(0)->widget();
        QLineEdit *tag = qobject_cast<QLineEdit*>(tag_widget);
        QWidget *color_widget = l->itemAt(1)->widget();
        QLineEdit *color = qobject_cast<QLineEdit*>(color_widget);
        QColor c = "#"+color->text();
        qDebug() << tag->text() << color->text();
        settings.setValue(tag->text(),c);

    }
}

void configtab::new_color_tag()
{
        ui->tag_color_box->addLayout(new_color_entry("","","Tag"));
}
void configtab::new_file_border()
{
        ui->file_border_box->addLayout(new_color_entry("","","Tag"));
}

QHBoxLayout* configtab::new_color_entry(QString item,QString hex,QString placeholder)
{
        QHBoxLayout *hbox = new QHBoxLayout();
        QLineEdit *tag_label = new QLineEdit(item);
        QLineEdit *color_label = new QLineEdit(hex);
        QPushButton *delete_button = new QPushButton("-");
        connect(delete_button,&QPushButton::clicked,tag_label,&QLineEdit::deleteLater);
        connect(delete_button,&QPushButton::clicked,color_label,&QLineEdit::deleteLater);
        connect(delete_button,&QPushButton::clicked,delete_button,&QPushButton::deleteLater);
        connect(delete_button,&QPushButton::clicked,hbox,&QHBoxLayout::deleteLater);

        color_label->setInputMask("HHHHHH");
        color_label->setPlaceholderText("Hex Color");
        tag_label->setPlaceholderText(placeholder);
        hbox->addWidget(tag_label);
        hbox->addWidget(color_label);
        hbox->addWidget(delete_button);
        return hbox;
}
