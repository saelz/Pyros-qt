#ifndef TAB_H
#define TAB_H

#include <QObject>
#include <QWidget>
#include <QString>


class QTabWidget;

class Tab : public QWidget
{
    Q_OBJECT
public:
    explicit Tab(QTabWidget *parent);

public slots:
    void delete_self();
    void set_title(QString new_title);
    void set_parent_tab(QWidget *parent);

private:
    QTabWidget *tab_widget;
    QWidget *parent_tab = nullptr;

signals:
    QString update_title();

};

#endif // TAB_H
