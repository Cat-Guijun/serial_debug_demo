#ifndef CLICKABLECOMBOBOX_H
#define CLICKABLECOMBOBOX_H

#include <QComboBox>
#include <QMouseEvent>


class ClickableCombobox : public QComboBox
{
    Q_OBJECT
public:
    explicit ClickableCombobox(QWidget *parent = nullptr);


protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void clickComboBox();

};



#endif // CLICKABLECOMBOBOX_H
