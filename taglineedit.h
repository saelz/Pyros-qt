#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>

#include <pyros.h>

class TagCompleter : public QCompleter
{
    Q_OBJECT

public:
    inline TagCompleter(const QStringList& tags, QObject * parent) :
            QCompleter(parent), m_list(tags), m_model()
    {
        setModel(&m_model);
    }

    void update(QString text);

private:
    QStringList m_list;
    QStringListModel m_model;
};

class TagLineEdit : public QLineEdit
{

    Q_OBJECT
public:
    TagLineEdit(QWidget *parent = nullptr);
    void set_relation_type(int flags);

private:
    void keyPressEvent(QKeyEvent *) override;
    QVector<QByteArray> tag_history;
    int hist_location = 0;
    TagCompleter *completer;
    int relation_type = PYROS_TAG_RELATION_FLAGS::PYROS_FILE_RELATIONSHIP;


public slots:
    void process_tag();
    void update_text_color(const QString &text);
    void update_completion(const QString &t);

signals:
    void tag_entered(QVector <QByteArray> tags);
    void reset();

};

#endif // TAGLINEEDIT_H
