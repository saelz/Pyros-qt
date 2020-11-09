#include "pyrosqt.h"
#include "taglineedit.h"

#include <QCompleter>
#include <QAction>
#include <QShortcut>
#include <QSettings>
#include <QKeyEvent>

#include <pyros.h>

TagLineEdit::TagLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    QStringList wordList;
    QSettings settings;
    PyrosDB *pyrosDB = Pyros_Open_Database(settings.value("db").toByteArray());


    PyrosList *all_tags = Pyros_Get_All_Tags(pyrosDB);
    for (size_t i = 0;i < all_tags->length; i++) {
        wordList << (char*)all_tags->list[i];
    }
    Pyros_List_Free(all_tags,free);
    Pyros_Close_Database(pyrosDB);

    QCompleter *completer = new QCompleter(wordList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter(completer);
    tag_history.append("");


    connect(this, &TagLineEdit::returnPressed,this, &TagLineEdit::process_tag);
    connect(this, &TagLineEdit::returnPressed,this, &TagLineEdit::clear, Qt::QueuedConnection);

    connect(this, &TagLineEdit::textChanged,this, &TagLineEdit::update_text_color);

}

void TagLineEdit::keyPressEvent(QKeyEvent *event)
{
    QLineEdit::keyPressEvent(event);
    if (event->key() == Qt::Key::Key_Up){
        if ((++hist_location) >= tag_history.length())
            hist_location = 0;

        setText(tag_history.at(hist_location));
    } else if (event->key() == Qt::Key::Key_Down){
        if ((--hist_location) < 0)
            hist_location = 0;

        setText(tag_history.at(hist_location));
    }
}

void TagLineEdit::process_tag()
{
    QSettings settings;
    const QByteArray tag = text().toUtf8();
    hist_location = 0;


    if (tag.isEmpty()){
        emit reset();
        return;
    }
    QList<QByteArray> l = tag.split('\n');
    QVector<QByteArray> tags = l.toVector();

    if (settings.value("use_tag_history",true).toBool()){
        foreach(QByteArray hist_tag, tags){
            if ( tag_history.count() > 1){
                if (hist_tag != tag_history.at(1))
                    tag_history.insert(1,hist_tag);
            } else
                tag_history.insert(1,hist_tag);
        }
    }

    emit tag_entered(tags);
}

void TagLineEdit::update_text_color(const QString &text){
    QSettings settings;
    QString color_prefix = "color: ";

    settings.beginGroup("tagcolor");
    setStyleSheet("");
    QStringList colored_tags = settings.allKeys();
    foreach(QString colored_prefix,colored_tags){
        if (text.startsWith(colored_prefix,Qt::CaseInsensitive)){
            QColor color = settings.value(colored_prefix).value<QColor>();
            setStyleSheet(color_prefix+color.name());
        }
    }
    settings.endGroup();

}
