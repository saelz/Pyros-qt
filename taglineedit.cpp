#include "pyrosdb.h"
#include "taglineedit.h"

#include <QCompleter>
#include <QAction>
#include <QShortcut>
#include <QSettings>
#include <QKeyEvent>
#include <QAbstractItemView>

#include <pyros.h>



void TagCompleter::update(QString text)   {
    QString comparison_text;

    if (text.isEmpty() || text.isNull()){
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
    foreach(QString t,m_list)
        if (t.startsWith(comparison_text))
            filtered.append(t);

    if (filtered.length() == 1){
        if (filtered.at(0) == comparison_text){
            popup()->hide();
            return;
        }
    }

    m_model.setStringList(filtered);
    complete();
}



TagLineEdit::TagLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    QSettings settings;
    PyrosTC *ptc = PyrosTC::get();

    PyrosTC::all_tags_cb cb= [&](QStringList tags){
        completer = new TagCompleter(tags, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setWidget(this);

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

        setText(tag_history.at(hist_location));
    } else {
        completer->update(text());
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

    if (text.startsWith('-')){
        QColor color = settings.value("special-tagcolor/invalid",
                      QColorConstants::Red).value<QColor>();
        setStyleSheet(color_prefix+color.name());
        return;
    }

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


void TagLineEdit::update_completion(const QString &t){
    QString new_text;

    if (this->text().startsWith('-'))
        new_text = '-';

    if (relation_type & PYROS_TAG_RELATION_FLAGS::PYROS_GLOB){
        foreach (QChar c ,t){
            if (c == '*' || c == '?' || c == '[' || c == ']'){
                new_text += '[';
                new_text += c;
                new_text += ']';

            } else {
                new_text += c;
            }
        }
    } else {
        new_text += t;
    }

    setText(new_text);
}

void TagLineEdit::set_relation_type(int flags)
{
    relation_type = flags;
}
