#include <QMouseEvent>
#include <QToolTip>

#include "overlay_combo_box.h"

Overlay_Combo_Box::Overlay_Combo_Box(bool *active_ptr,QString tooltip,Overlay *parent) : Overlay_Button("",active_ptr,tooltip,parent)
{
    connect(this,&Overlay_Combo_Box::clicked,this,&Overlay_Combo_Box::toggle_drop_down);
}

bool Overlay_Combo_Box::activate_hover(QMouseEvent *e)
{
    bool inital_status = highlighed;
    int last_highlighted_entry = highlighted_entry;

    if (entries.empty())
        return false;

    if (rect.contains(e->pos())){
        highlighed = true;
        if (!display_dropdown)
            QToolTip::showText(e->globalPos(),tooltip);

    } else {
        highlighed = false;
    }


    if (display_dropdown){
        highlighted_entry = -1;
        for(int i = 0;i < dropdownrect.length();i++){
            if (dropdownrect[i].contains(e->pos())){
                if (selected_entry  > i)
                    highlighted_entry = i;
                else
                    highlighted_entry = i+1;

                break;
            }
        }

    }

    if (inital_status != highlighed ||
            last_highlighted_entry != highlighted_entry){
        emit request_redraw();
    }

    return (highlighted_entry != -1 || highlighed);
}

bool Overlay_Combo_Box::check_hover(QMouseEvent *e)
{
    if (Overlay_Widget::check_hover(e))
        return true;

    if (display_dropdown)
        for(int i = 0;i < dropdownrect.length();i++)
            if (dropdownrect[i].contains(e->pos()))
                return true;

    return false;
}

int Overlay_Combo_Box::requested_width(QPainter &p)
{
    if ((active == nullptr || (*active)) && !entries.isEmpty())
        return p.boundingRect(0,0,0,0,0,entries[selected_entry].name).width()+3;
    else
        return 0;
}

int Overlay_Combo_Box::draw(QPainter &p,int x,int y)
{
    if ((active == nullptr || (*active)) && !entries.isEmpty()){
        Combo_Entry current_entry = entries[selected_entry];
        int used_width;

        rect = p.boundingRect(x,y,0,0,0,current_entry.name);

        rect.adjust(0,-rect.height(),0,-rect.height());
        if (highlighed || highlighted_entry != -1)
            p.drawRect(rect);

        p.drawText(rect,current_entry.name);
        used_width = rect.width();

        if (display_dropdown){
            QBrush bg(QColor(0,0,0,255));
            QBrush highlight_bg(QColor(40,40,40,255));
            QRect bounding_text = rect;
            y -= bounding_text.height();

            int longest_drop_down = 0;
            for(int i = 1;i < entries.length();i++)
                if (entries[longest_drop_down].name.length() < entries[i].name.length())
                    longest_drop_down = i;

            int dropdown_width = p.boundingRect(0,0,0,0,0,entries[longest_drop_down].name).width();


            for(int i = 0;i < entries.length();i++){
                if (entries[i].value != current_entry.value){
                    y -= bounding_text.height();

                    bounding_text = p.boundingRect(bounding_text.x(),y,0,0,0,entries[i].name);
                    bounding_text.setWidth(dropdown_width);

                    if (highlighted_entry == i)
                        p.fillRect(bounding_text,highlight_bg);
                    else
                        p.fillRect(bounding_text,bg);

                    p.drawText(bounding_text,entries[i].name);
                    dropdownrect.append(bounding_text);
                }
            }

        }

        return used_width+3;

    } else {
        return 0;
    }

}

void Overlay_Combo_Box::toggle_drop_down()
{
    if (entries.isEmpty())
        return;

    if (display_dropdown){
        dropdownrect.clear();
        if (highlighted_entry != -1){
            selected_entry = highlighted_entry;
            emit entry_changed(entries[highlighted_entry].value);
        }

        highlighted_entry = -1;

    }

    display_dropdown = !display_dropdown;
    emit request_redraw();
}

