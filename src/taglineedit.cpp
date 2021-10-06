#include "pyrosdb.h"
#include "taglineedit.h"
#include "configtab.h"

#include <QCompleter>
#include <QAction>
#include <QShortcut>
#include <QSettings>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QListView>

#include <pyros.h>


using ct = configtab;

TagCompleter::TagCompleter(const QStringList *tags,QVector<QString> *tag_history, QObject * parent) :
    QCompleter(parent), m_list(tags), m_model(),tag_history(tag_history)
{
    QListView *view = (QListView *)popup();
    view->setUniformItemSizes(true);
    view->setLayoutMode(QListView::Batched);

    connect(this, QOverload<const QString &>::of(&QCompleter::activated),
        this, &TagCompleter::set_hist_false);
    setModel(&m_model);
}

void TagCompleter::update(QString text)   {
    QString comparison_text;

    if ((text.isEmpty() || text.isNull()) && !hist_mode){
        popup()->hide();
        return;
    }

    if (text.at(0) == '-'){
        if (text.length() == 1){
            popup()->hide();
            return;
        }

        comparison_text = text.mid(1);
    } else {
        comparison_text = text;
    }

    if (caseSensitivity() == Qt::CaseInsensitive)
        comparison_text = comparison_text.toLower();

    QStringList filtered;
    if (hist_mode){
        foreach(QString t,*tag_history)
            if (!t.isEmpty() && t.contains(comparison_text))
                filtered.append(t);
    } else {
        foreach(QString t,*m_list)
            if (t.startsWith(comparison_text))
                filtered.append(t);
    }

    m_model.setStringList(filtered);
    complete();
}



TagLineEdit::TagLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    PyrosTC *ptc = PyrosTC::get();

    PyrosTC::all_tags_cb cb= [&](QStringList *tags){
        completer = new TagCompleter(tags,&tag_history, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setWidget(this);
        tag_list = tags;

        connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
                this, &TagLineEdit::update_completion);
        connect(completer, QOverload<const QString &>::of(&QCompleter::highlighted),
                this, &TagLineEdit::update_completion);
    };

    ptc->get_all_tags(this,cb);

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

        if (hist_location != 0)
            setText(tag_history.at(hist_location));
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key::Key_R){
        if (completer != nullptr){
            completer->toggle_hist_mode();
            completer->update(text());
        }
    } else if (event->key() == Qt::Key::Key_Enter || event->key() == Qt::Key::Key_Return){
        if (completer != nullptr)
            completer->set_hist_false();
    } else {
        if (completer != nullptr)
            completer->update(text());
    }
}

void TagLineEdit::process_tag()
{
    QSettings settings;
    const QByteArray tag = text().toLower().toUtf8();
    hist_location = 0;


    if (tag.isEmpty()){
        emit reset();
        return;
    }

    QList<QByteArray> l = tag.split('\n');
    QVector<QByteArray> tags = l.toVector();

    if (ct::setting_value(ct::TAG_HISTORY).toBool()){
        foreach(QByteArray hist_tag, tags){
            tag_history.removeAll(hist_tag);
            tag_history.insert(1,hist_tag);
        }
    }

    emit tag_entered(tags);
}

void TagLineEdit::update_text_color(const QString &text){
    QSettings settings;
    QString color_prefix = "color: ";

    if (text.startsWith('-')){
        QColor color = settings.value("special-tagcolor/invalid",
                          QColorConstants::Red).value<QColor>();
        setStyleSheet(color_prefix+color.name());
        return;
    }

    QVector<ct::color_setting> tag_colors = ct::get_tag_colors();

    setStyleSheet("");
    foreach(ct::color_setting tag_color,tag_colors)
        if (text.startsWith(tag_color.starts_with,Qt::CaseInsensitive))
            setStyleSheet(color_prefix+tag_color.color.name());

}


void TagLineEdit::update_completion(const QString &t){
    QString new_text;

    if (this->text().startsWith('-'))
        new_text = '-';

    if (relation_type & PYROS_TAG_RELATION_FLAGS::PYROS_GLOB)
        new_text.append(PyrosTC::escape_glob_characters(t.toUtf8()));
    else
        new_text.append(t);

    setText(new_text);
}

void TagLineEdit::set_relation_type(int flags)
{
    relation_type = flags;
}
