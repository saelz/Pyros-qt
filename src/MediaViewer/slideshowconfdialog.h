#ifndef SLIDESHOWCONFDIALOG_H
#define SLIDESHOWCONFDIALOG_H

#include <QDialog>
#include "mediaviewer.h"

namespace Ui {
class SlideshowConfDialog;
}

class SlideshowConfDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SlideshowConfDialog(MediaViewer *parent);

private:
    MediaViewer *parent;
    Ui::SlideshowConfDialog *ui;

    void accept() override;


};

#endif // SLIDESHOWCONFDIALOG_H
