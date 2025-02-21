#include "clickablecombobox.h"




ClickableCombobox::ClickableCombobox(QWidget *parent)
    :QComboBox(parent)
{
      //
}


void ClickableCombobox::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton){
            emit clickComboBox();
    }
    QComboBox::mousePressEvent(event);
}
