#include <QPushButton>

#include "slideshowconfdialog.h"
#include "ui_slideshowconfdialog.h"

SlideshowConfDialog::SlideshowConfDialog(MediaViewer *parent) :
    QDialog(parent),
    parent(parent),
    ui(new Ui::SlideshowConfDialog)
{
    ui->setupUi(this);

    ui->view_entire_media_check_box->hide();

    ui->slide_length_box->setValue(parent->slideshow_wait_time/1000.0);
    ui->loop_check_box->setChecked(parent->slideshow_loop);
    ui->random_order_check_box->setChecked(parent->slideshow_random_order);
    ui->view_entire_media_check_box->setChecked(parent->slideshow_watch_entire_video);
}


void SlideshowConfDialog::accept()
{
    parent->slideshow_wait_time = ui->slide_length_box->value()*1000;
    parent->slideshow_loop = ui->loop_check_box->isChecked();
    parent->slideshow_random_order = ui->random_order_check_box->isChecked();
    parent->slideshow_watch_entire_video = ui->view_entire_media_check_box->isChecked();
    QDialog::accept();
}
