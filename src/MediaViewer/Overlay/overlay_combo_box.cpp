#include <QToolTip>

#include "overlay_combo_box.h"

Overlay_Combo_Box::Overlay_Combo_Box(bool *visible_ptr,QString tooltip,Overlay *parent) : Overlay_Button("",visible_ptr,tooltip,parent)
{
    connect(this,&Overlay_Combo_Box::clicked,this,&Overlay_Combo_Box::toggle_drop_down);
    connect(this,&Overlay_Combo_Box::unselected,this,&Overlay_Combo_Box::hide_drop_down);
}

bool Overlay_Combo_Box::activate_hover(QPoint pos)
{
    int last_highlighted_entry = highlighted_entry;

    if (entries.empty())
        return false;

    if (display_dropdown){
        highlighted_entry = -1;
        for(int i = 0;i < dropdownrect.length();i++){
            if (dropdownrect[i].contains(pos)){
                if (selected_entry  > i)
                    highlighted_entry = i;
                else
                    highlighted_entry = i+1;

                break;
            }
        }

    }

    if (last_highlighted_entry != highlighted_entry){
        emit request_redraw();
        return true;
    } else {
        return Overlay_Button::activate_hover(pos);
    }

    return false;
}

bool Overlay_Combo_Box::check_hover(QPoint pos)
{
    if (Overlay_Widget::check_hover(pos))
        return true;

    if (display_dropdown)
        for(int i = 0;i < dropdownrect.length();i++)
            if (dropdownrect[i].contains(pos))
                return true;

    return false;
}

void Overlay_Combo_Box::set_entry(int value)
{
    for(int i = 0;i < entries.length();i++){
        if (entries.at(i).value == value){
            if (selected_entry != i){
                selected_entry = i;
                emit request_redraw();
                emit entry_changed(value);
            }
            return;
        }
    }
    selected_entry = -1;
}

int Overlay_Combo_Box::requested_width(QPainter &p)
{
    if ((visible == nullptr || (*visible)) && !entries.isEmpty()){
        if (selected_entry < 0 || selected_entry >= entries.length())
            return p.boundingRect(0,0,0,0,0,"     ").width()+3;
        else
            return p.boundingRect(0,0,0,0,0,entries[selected_entry].name).width()+3;
    } else {
        return 0;
    }
}

int Overlay_Combo_Box::draw(QPainter &p,int x,int y)
{
    if ((visible == nullptr || (*visible)) && !entries.isEmpty()){
        Combo_Entry current_entry;
        int used_width;

        if (selected_entry < 0 || selected_entry >= entries.length())
            current_entry = {"     ",-1,true};
        else
            current_entry = entries[selected_entry];

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

            int longest_drop_down = -1;
            int dropdown_width;

            for(int i = 0;i < entries.length();i++){
                if (entries[i].hidden || entries[i].value == current_entry.value)
                    continue;

                if (longest_drop_down == -1 || entries[longest_drop_down].name.length() < entries[i].name.length())
                    longest_drop_down = i;
            }

            if (longest_drop_down == -1)
                dropdown_width = 0;
            else
                dropdown_width = p.boundingRect(0,0,0,0,0,entries[longest_drop_down].name).width();


            for(int i = 0;i < entries.length();i++){
                if (!entries[i].hidden && entries[i].value != current_entry.value){
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
    tooltip_enabled = !display_dropdown; /*display tooltip if no dropdown*/

    emit request_redraw();
}

void Overlay_Combo_Box::hide_drop_down()
{
    if (display_dropdown)
        toggle_drop_down();
}

void Overlay_Combo_Box::add_entry(QString name,int value)
{
    entries.append({name,value,false});
}

void Overlay_Combo_Box::remove_entry(int value)
{
    for(int i = 0;i < entries.length();i++){
        if (entries[i].value == value){
            entries.removeAt(i);
            return;
        }
    }
}

void Overlay_Combo_Box::set_entry_hidden_state(int value,bool is_hidden)
{
    for(int i = 0;i < entries.length();i++){
        if (entries[i].value == value){
            entries[i].hidden = is_hidden;
            return;
        }
    }

}
