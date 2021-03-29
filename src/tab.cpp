#include <QTabWidget>

#include "tab.h"

Tab::Tab(QTabWidget *parent) :
    QWidget(parent),tab_widget(parent)
{
    tab_widget->insertTab(tab_widget->currentIndex()+1,this,"Tab");
    tab_widget->setCurrentIndex(tab_widget->currentIndex()+1);
}

void Tab::set_title(QString title)
{
    int index = tab_widget->indexOf(this);
    int max_title = 20;

    if (title.length() > max_title)
        title = title.left(max_title-3)+"...";

    tab_widget->setTabText(index,title);

}

void Tab::delete_self()
{
    int index = tab_widget->indexOf(parent_tab);
    if (index != -1)
        tab_widget->setCurrentIndex(index);

    tab_widget->removeTab(tab_widget->indexOf(this));
    deleteLater();
}

void Tab::set_parent_tab(QWidget *parent)
{
    parent_tab = parent;
}
