#ifndef OVERLAY_COMBO_BOX_H
#define OVERLAY_COMBO_BOX_H

#include "overlay_button.h"

class Overlay_Combo_Box : public Overlay_Button
{
    Q_OBJECT

    bool display_dropdown = false;
    QVector<QRect> dropdownrect;
public:
    Overlay_Combo_Box(bool *visible_ptr,QString tooltip,Overlay *parent);

    struct Combo_Entry{
        QString name;
        int value;
        bool hidden;
    };

    QVector<Combo_Entry> entries;
    int requested_width(QPainter &p) override;
    int draw(QPainter &p,int x, int y) override;
    bool activate_hover(QPoint local_pos) override;


    int selected_entry = 0;
    int highlighted_entry = -1;

public slots:
    bool check_hover(QPoint local_pos) override;
    void set_entry(int value);
    void add_entry(QString name,int value);
    void remove_entry(int value);
    void set_entry_hidden_state(int value,bool is_hidden);

private slots:
    void toggle_drop_down();
    void hide_drop_down();

signals:
    void entry_changed(int);

};
#endif // OVERLAY_COMBO_BOX_H
