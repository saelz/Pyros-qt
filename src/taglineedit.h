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
    TagCompleter(const QStringList *tags,QVector<QString> *tag_history,int relation_type, QObject * parent);
    void update(QString text);

    inline void toggle_hist_mode(){ hist_mode = !hist_mode;}

    inline void set_hist_false(){hist_mode = false;}

private:
    const QStringList *m_list;
    QStringListModel m_model;
    QVector<QString> *tag_history;
    bool hist_mode = false;
    int relation_type;
};

class TagLineEdit : public QLineEdit
{

    Q_OBJECT
public:
    TagLineEdit(QWidget *parent = nullptr);
    void set_relation_type(int flags);

private:
    void keyPressEvent(QKeyEvent *) override;
    QVector<QString> tag_history;
    QStringList *tag_list;

    int hist_location = 0;
    TagCompleter *completer = nullptr;
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
