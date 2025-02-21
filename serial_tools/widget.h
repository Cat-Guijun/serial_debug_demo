#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QTimer>
#include <QStringListModel>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void initWidgetStatus();
    void setWidgetsEnable(QLayout *layout,bool status=false);

    void initserialCombo();

    void connectSlots();
public slots:
    void setTimeStatus();
    void hexShowRecv(bool);

    void lineBtnClickSlot();

private slots:
    void on_openComBtn_clicked(bool checked);

    void on_sendDataBtn_clicked();

    void serial_readable();

    void on_timerSendCheck_clicked(bool checked);

    void on_clearRecvBtn_clicked();

    void on_saveRecvBtn_clicked();

    void on_hidePannelBtn_clicked(bool checked);

    void on_hideHistoryBtn_clicked(bool checked);

    void comboBox_click();


    void on_loopsendBox_clicked(bool checked);

    void loopSendMsg();

    void on_resetBtn_clicked();

    void on_saveFileBtn_clicked();

    void on_loadFileBtn_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *port;
    QTimer * _timeshow;
    QTimer * _timerSend;
    QTimer * _timerloopSend;
    QStringListModel historylist;
    int _sentByte=0;
    int _recvByte=0;
    int loopBtnIndex=0;  // 记得加1；
};
#endif // WIDGET_H
