#include "widget.h"
#include "ui_widget.h"
#include "utility.h"
#include "clickablecombobox.h"
#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <memory>
#include <fstream>
#include <QMessageBox>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    ,historylist(QStringList(),this)
{
    ui->setupUi(this);
    port=new QSerialPort(this);
    _timeshow=new QTimer(this);
    _timerSend=new QTimer(this);
    _timerloopSend=new QTimer(this);
    this->setLayout(ui->gridLayoutGlobal);
    initWidgetStatus();
}


Widget::~Widget()
{
    delete ui;
}

void Widget::initserialCombo()
{
    QComboBox * serialCombo=ui->serialCombo;
    serialCombo->clear();
    QList<QSerialPortInfo> ports=QSerialPortInfo::availablePorts();
    for(auto port :ports){
        if(port.isValid())
        {
            serialCombo->addItem(port.portName());
        }
    }
}


void Widget::setTimeStatus(){
    QDateTime now=QDateTime::currentDateTime();
//    std::istringstream s_in;
    QDate date=now.date();
    QTime time=now.time();
    QString timestring = QString("%1-%2-%3 %4:%5:%6")
            .arg(date.year())
            .arg(date.month(), 2, 10, QChar('0'))
            .arg(date.day(), 2, 10, QChar('0'))
            .arg(time.hour(), 2, 10, QChar('0'))
            .arg(time.minute(), 2, 10, QChar('0'))
            .arg(time.second(), 2, 10, QChar('0'));

    ui->timeshow_label->setText(timestring);
}


void Widget::connectSlots(){
    // 初始化下面的时间label,用定时器
    QObject::connect(_timeshow,&QTimer::timeout,this,&Widget::setTimeStatus);
    _timeshow->start(1000);

    // bind 发送消息定时器
    QObject::connect(_timerSend,&QTimer::timeout,this,&Widget::on_sendDataBtn_clicked);
    QObject::connect(_timerloopSend,&QTimer::timeout,this,&Widget::loopSendMsg);

    // checked hex显示
    QObject::connect(ui->hexshowCheck,QOverload<bool>::of(&QCheckBox::clicked),this,&Widget::hexShowRecv);


    // 绑定comoBox和串口刷新程序
    QObject::connect(ui->serialCombo,&ClickableCombobox::clickComboBox,this,&Widget::comboBox_click);

    // connect 右上角每个按钮的点击时间
    // pushButton_line1 lineEdit_line8    checkBox_line1
    for(int i=1;i<9;i++){
//        qDebug()<<QString("pushButton_line%1").arg(i);
        QPushButton* btn=findChild<QPushButton*>(QString("pushButton_line%1").arg(i));
//        qDebug()<< btn->objectName() << btn->text();
        QObject::connect(btn,&QPushButton::clicked,this,&Widget::lineBtnClickSlot);
    }

}


void Widget::initWidgetStatus()
{
    // 设置listview的model

    ui->historyList->setModel(&historylist);
    ui->historyList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 初始化combo和一些按钮的状态
    initserialCombo();
    ui->baudCombo->clear();
    QList<qint32> baudlist=QSerialPortInfo::standardBaudRates();
    for(auto baudrate : baudlist){
        ui->baudCombo->addItem(QString().setNum(baudrate));
    }
    ui->baudCombo->setCurrentIndex(baudlist.indexOf(115200));
    ui->dataBitCombo->setCurrentIndex(ui->dataBitCombo->count()-1);


    // 按钮状态
    setWidgetsEnable(dynamic_cast<QLayout *>(ui->lrightbottom));
    for(int i=1;i<=8;i++){
        QHBoxLayout* line=ui->boxMultiText->findChild<QHBoxLayout*>(QString("top_line%1").arg(i));
        setWidgetsEnable(line);
    }

    connectSlots();


}

void Widget::setWidgetsEnable(QLayout *layout,bool status)
{
    for(int i=0;i<layout->count();i++)
    {
        auto item=layout->itemAt(i);
        if(item->widget() && !item->widget()->objectName().contains("label_1") &&
              item->widget()->objectName() != "sendTextlineEdit"  && !item->widget()->objectName().contains("lineEdit_line")){
                item->widget()->setEnabled(status);
        }
    }
}



void Widget::on_openComBtn_clicked(bool checked)
{
    if(checked){
        QString portname=ui->serialCombo->currentText();
        port->setPortName(portname);
        port->setBaudRate(ui->baudCombo->currentText().toInt());
        switch (ui->dataBitCombo->currentText().toUInt()) {
            case 5: port->setDataBits(QSerialPort::Data5);break;
            case 6: port->setDataBits(QSerialPort::Data6);break;
            case 7: port->setDataBits(QSerialPort::Data7);break;
            case 8: port->setDataBits(QSerialPort::Data8);break;
            default: break;
        }
        port->setParity(QSerialPort::NoParity);
        switch (ui->stopCombo->currentIndex()) {
            case 0: port->setStopBits(QSerialPort::OneStop); break;
            case 1: port->setStopBits(QSerialPort::OneAndHalfStop); break;
            case 2: port->setStopBits(QSerialPort::TwoStop); break;
        default: break;
        }

        if(port->open(QIODevice::ReadWrite)){
            // 打开了串口
            // 关联signal
            QObject::connect(port,&QSerialPort::readyRead,this,&Widget::serial_readable);

            // 把左侧combo ban    combogroup_1 到6
            for(int i=1;i<7;i++){
                QLayout *la=ui->RBbox->findChild<QLayout*>(QString("combogroup_%1").arg(i));
                setWidgetsEnable(la);
            }
            // 打开之前关闭的按钮
            setWidgetsEnable(dynamic_cast<QLayout *>(ui->lrightbottom),true);
            for(int i=1;i<=8;i++){
                QHBoxLayout* line=ui->boxMultiText->findChild<QHBoxLayout*>(QString("top_line%1").arg(i));
                setWidgetsEnable(line,true);
            }

            ui->comStatusLabel->setText(QString("%1 opened").arg(portname));
            ui->openComBtn->setText("关闭串口");
        }else{
            QMessageBox openfailBox(QMessageBox::Warning,QString("出错啦"),
                                    QString("打卡串口失败,串口被占用或者已拔出"));
            openfailBox.addButton(QString("确定"),QMessageBox::AcceptRole);
            openfailBox.exec();
        }
        return;
    }
    // 下面的逻辑是点击关闭串口
    port->close();

    for(int i=1;i<7;i++){
        QLayout *la=ui->RBbox->findChild<QLayout*>(QString("combogroup_%1").arg(i));
        setWidgetsEnable(la,true);
    }

    setWidgetsEnable(dynamic_cast<QLayout *>(ui->lrightbottom));
    for(int i=1;i<=8;i++){
        QHBoxLayout* line=ui->boxMultiText->findChild<QHBoxLayout*>(QString("top_line%1").arg(i));
        setWidgetsEnable(line);
    }

    ui->comStatusLabel->setText(QString("%1 closed").arg(port->portName()));
    ui->openComBtn->setText("打开串口");

}

void Widget::on_sendDataBtn_clicked()
{
    if(ui->sendTextlineEdit->text().isEmpty()){
        ui->send_label->setText("No input");
        return;
    }
    QString sendText=ui->sendTextlineEdit->text();
    // 看看是不是HEX发送 和发送newline
    if(ui->hexSendCheck->isChecked()){
        //
        //
        QRegularExpression hexReg("[A-F0-9a-f].*?");
        std::unique_ptr<QRegularExpressionValidator> validator(new QRegularExpressionValidator(hexReg,nullptr));
        int pos = 0;
        if(validator->validate(sendText,pos)){
            // sendText.toUtf8().toHex()
            QByteArray hexdata=QByteArray::fromHex(sendText.toUtf8());  // ff字符串变来
            _sentByte+= port->write(hexdata);
            ui->send_label->setText(
                        QString("Sent: %1").arg(_sentByte)
                        );
        }
        else{
            ui->comStatusLabel->setText("Error Input!");
        }

    }else{
        // string 发送

        if(ui->sendNewlineCheck->isChecked()){
            sendText.append(QString("\r\n"));
        }
        _sentByte+=port->write(sendText.toLatin1());
        ui->send_label->setText(
                    QString("Sent: %1").arg(_sentByte)
                    );
        // send的记录 加入
        // QModelIndex indexA = model->index(0, 0, QModelIndex());
//        QVariant
        auto nolinetext=ui->sendTextlineEdit->text();
        if(historylist.rowCount()==0 || historylist.data(historylist.index(historylist.rowCount()-1,0)).toString()!=nolinetext)
        {
            int rowcount=historylist.rowCount();
            historylist.insertRow(rowcount);
            historylist.setData(historylist.index(rowcount,0),
                                nolinetext);
            // 滚动进度条
            ui->historyList->scrollToBottom();
        }

    }
}

void Widget::serial_readable()
{
    //这里找了很久的问题：hex显示ff的时候，取消hex显示再变回来出问题。
    QByteArray recvdata=port->readAll();
    _recvByte+=recvdata.size();
    ui->recv_label->setText(QString("Received:")+QString().setNum(_recvByte));
    if(ui->hexshowCheck->isChecked()){  //      Oxff
       recvdata=recvdata.toHex().toUpper();  //变成Hex字符串。
    }
        // string接收
        // 发送一个char
    QString oldtext= ui->textRecv->toPlainText();
    if(ui->recvTimerCheck->isChecked()){
        oldtext+=QString("\r\n[%1]").arg(ui->timeshow_label->text());
    }

    QString recvText=QString(recvdata);
    if(ui->autoNewLineCheck->isChecked()){
    #ifdef Q_OS_WIN
         recvText=recvText.prepend("\r\n");
    #elif defined(Q_OS_LINUX)
        recvText=recvText.prepend("\n");
    #endif
    }
    ui->textRecv->setText(oldtext+recvText);
    ui->textRecv->moveCursor(QTextCursor::End);
}


void Widget::hexShowRecv(bool checked){
    //
    QString recvText=ui->textRecv->toPlainText();// ee ff
    if(recvText.isEmpty()) return;
    if(checked){
        // OByteArray::fromHex  是把一个'ff'  解码成 'x'
        // QByteArray::toHex  是把值变成 'x'.  变成 'ff'
        // 全部转换成大写，每两个后面添加一个空格
        QByteArray recvdata=recvText.toLatin1();
//        qDebug()<<"------"<<recvdata.size();
        recvText=QString::fromLatin1(recvdata.toHex()).toUpper();
        QString tempText;
        for(int i=0;i<recvText.size();i+=2){
            tempText+=recvText.mid(i,2)+QString(QChar(' '));
        }
        recvText=tempText;
    }else{
        // 去除空格
        recvText=recvText.remove(QChar(' '));
        QByteArray recvdata=recvText.toLatin1(); // ff
        qDebug()<<"------"<<recvdata.size();   // ff
        recvText=QString::fromLatin1(QByteArray::fromHex(recvdata));
        qDebug()<<"------"<<recvText.size();
    }
    ui->textRecv->setText(recvText);
}




void Widget::on_timerSendCheck_clicked(bool checked)
{
    if(checked && !ui->timerLineEdit->text().isEmpty() && !ui->sendTextlineEdit->text().isEmpty()){
        _timerSend->start(ui->timerLineEdit->text().toUInt());
    }else{
        _timerSend->stop();
    }
}

void Widget::on_clearRecvBtn_clicked()
{
    ui->textRecv->clear();
    _sentByte=0;
    _recvByte=0;
    ui->send_label->setText(
                QString("Sent: %1").arg(_sentByte)
                );
    ui->recv_label->setText(QString("Received:")+QString().setNum(_recvByte));
}

void Widget::on_saveRecvBtn_clicked()
{
    // save
    // 这里用Qt的file 好啦
    if(ui->textRecv->toPlainText().isEmpty()) return;

     QString fileName = QFileDialog::getSaveFileName(this,tr("另存为"),
                                 "D:/1.txt",tr("All (*.*)"));

     QFile qfile(fileName);
     if (!qfile.open(QIODevice::WriteOnly))
            return;

     qfile.write(ui->textRecv->toPlainText().toUtf8());

     qfile.close();

}

void Widget::on_hidePannelBtn_clicked(bool checked)
{
    if(checked){
        ui->hidePannelBtn->setText("扩展面板");
        ui->boxMultiText->hide();
        return;
    }
    ui->hidePannelBtn->setText("隐藏面板");
    ui->boxMultiText->show();
}

void Widget::on_hideHistoryBtn_clicked(bool checked)
{
    if(checked){
        ui->hideHistoryBtn->setText("显示历史");
        ui->boxHistory->hide();
        return;
    }
    ui->hideHistoryBtn->setText("隐藏历史");
    ui->boxHistory->show();
}

void Widget::comboBox_click()
{
    // refresh 串口
    initserialCombo();
    ui->comStatusLabel->setText("Refresh ok!");
}

void Widget::on_loopsendBox_clicked(bool checked)
{
    if(checked){
        _timerloopSend->start(ui->sendmsSpin->text().toUInt());
    }else{
        _timerloopSend->stop();
    }
}

void Widget::loopSendMsg()
{
    QPushButton* nowBtn=findChild<QPushButton*>(QString("pushButton_line%1").arg(loopBtnIndex+1));
    loopBtnIndex=(loopBtnIndex+1)%8;
    emit nowBtn->clicked();

}

void Widget::lineBtnClickSlot(){
    // 右上角的按钮
    // pushButton_line1 lineEdit_line8    checkBox_line1
    QChar index=sender()->objectName().back();
    QLineEdit* correspondingEdit= findChild<QLineEdit*>(QString("lineEdit_line%1").arg(index));
    if(correspondingEdit->text().isEmpty()) return;
    QCheckBox* correspondingCheck=findChild<QCheckBox*>(QString("checkBox_line%1").arg(index));
    ui->hexSendCheck->setCheckState(correspondingCheck->checkState());
    ui->sendTextlineEdit->setText(correspondingEdit->text());
    on_sendDataBtn_clicked();
}

// 加event filter

void Widget::on_resetBtn_clicked()
{
    QMessageBox msg(QMessageBox::Question,"提示","重置列表将不可逆,确定是否重置?");
    msg.addButton(QString("否(N)"),QMessageBox::RejectRole);
    msg.addButton(QString("是(Y)"),QMessageBox::AcceptRole);
    YesNoEventFilter filter;
    msg.installEventFilter(&filter);
    int res=msg.exec();
    if(res==QDialog::Accepted){
        for(int i=1;i<9;i++){
            QLineEdit* edit=findChild<QLineEdit*>(QString("lineEdit_line%1").arg(i));
            edit->clear();
            QCheckBox* check=findChild<QCheckBox*>(QString("checkBox_line%1").arg(i));
            check->setCheckState(Qt::Unchecked);
        }
    }
}

void Widget::on_saveFileBtn_clicked()
{
    QString filename=QFileDialog::getSaveFileName(this,tr("保存列表"),
                                 "D:/save.txt",
                                 tr("txt文件 (*.txt)"));
    std::ofstream _file(filename.toStdString());
    for(int i=1;i<9;i++){
        QLineEdit* edit=findChild<QLineEdit*>(QString("lineEdit_line%1").arg(i));
        QCheckBox* check=findChild<QCheckBox*>(QString("checkBox_line%1").arg(i));
        _file<<i<< "|" << (int)check->checkState()<< "|" <<edit->text().toStdString() << std::endl;
    }
    _file.close();
}

void Widget::on_loadFileBtn_clicked()
{
    QString filename=QFileDialog::getOpenFileName();
    QFile _f(filename);
    if(_f.open(QIODevice::ReadOnly)){
        for(int i=1;i<9;i++){
            QLineEdit* edit=findChild<QLineEdit*>(QString("lineEdit_line%1").arg(i));
            QCheckBox* check=findChild<QCheckBox*>(QString("checkBox_line%1").arg(i));
            char buffer[1024];
            _f.readLine(buffer,1024);
            QString s1(buffer);
            auto slist=s1.split(QChar('|'));
//            qDebug()<<slist.size()<<slist[2];
            slist[2]=slist[2].remove('\r').remove('\n');
            if(!slist[2].isEmpty()) edit->setText(slist[2]);
            if(slist[1].toInt()){
                check->setCheckState(Qt::Checked);
            }else{
                check->setCheckState(Qt::Unchecked);
            }
        }
    }
}
