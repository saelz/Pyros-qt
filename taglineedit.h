#ifndef TAGLINEEDIT_H
#define TAGLINEEDIT_H

#include <QWidget>
#include <QLineEdit>

class TagLineEdit : public QLineEdit
{

    Q_OBJECT
public:
    TagLineEdit(QWidget *parent = nullptr);

private:
    void keyPressEvent(QKeyEvent *) override;
    QVector<QByteArray> tag_history;
    int hist_location = 0;

public slots:
    void process_tag();
private slots:
    void update_text_color(const QString &text);

signals:
    void tag_entered(QVector <QByteArray> tags);
    void reset();

};

#endif // TAGLINEEDIT_H
