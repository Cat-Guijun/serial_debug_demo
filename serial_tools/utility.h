#ifndef UTILITY_H
#define UTILITY_H

#include <QEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>


class YesNoEventFilter: public QObject
{
    Q_OBJECT
public:
    bool eventFilter(QObject *watched, QEvent *event) override{
        if(event->type()==QEvent::KeyPress){
            QKeyEvent *e=dynamic_cast<QKeyEvent*>(event);
            QMessageBox* mbox=dynamic_cast<QMessageBox*>(watched);

            switch (e->key()) {
            case Qt::Key_Y: mbox->accept(); break;
            case Qt::Key_N: mbox->reject(); break;
            }
        }
        return QObject::eventFilter(watched,event);
    }
};


#endif // UTILITY_H
