#include "controll.h"
#include "ui_controll.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QTableView>
#include <QFileDialog>


QSerialPort * serial;

Controll::Controll(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Controll)
{

    ui->setupUi(this);
    // Serial port on system
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->Serial->addItem(port.portName());
    }

    model = new QStandardItemModel(4,2,this);
    for(int row =0;row <4;row++)
    {

    }


}

Controll::~Controll()
{
    delete ui;
    serial->close();
}

void Controll::on_connect_button_clicked()
{
    serial = new QSerialPort(this);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setPortName(ui->Serial->currentText());
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    serial->open(QIODevice::ReadWrite);
    connect(serial,SIGNAL(readyRead()),this,SLOT(serialRecived()));




}
void Controll::serialRecived()
{

    QString data_read = serial->readLine();
    ui->Serial_window->insertPlainText(data_read);
    ui->Serial_window->moveCursor (QTextCursor::End);

    QRegExp rx("(\\;)"); //RegEx for ' ' or ',' or '.' or ':' or '\t'
    QStringList query = data_read.split(rx);
    qDebug() << query[0]; // outputs "000012"
    if(query.length()>4)
    {
        if(query[0] == "1")
        {
            ui->radio->setText(query[1]);
            ui->freqvensy->setText(query[3]);
            ui->Band->setText(query[4]);
            if(ui->bussconfig->rowCount() >= (query[2].toInt()))
            {
                QString ampname = ui->bussconfig->item((query[2].toInt()-1),2)->text();
                ui->current_amp->setText(ampname);
            }
        }
    }


}
/*** parsing a cvs fil to qwidget */

void Controll::on_Cvs_load_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open CVS file"),"","*.csv;;");

    QStringList wordList;

    QFile file(filename);

    if (file.open(QIODevice::ReadOnly))
    {
        int i =1;
        while(!file.atEnd())
        {
            //file opened successfully
            QString data;

            data = file.readLine();
            wordList = data.split(';');

            ui->bussconfig->insertRow(i);
            ui->bussconfig->setItem(i,0,new QTableWidgetItem(wordList[0]));
            ui->bussconfig->setItem(i,1,new QTableWidgetItem(wordList[1]));
            ui->bussconfig->setItem(i,2,new QTableWidgetItem(wordList[2]));
            ui->bussconfig->setItem(i,3,new QTableWidgetItem(wordList[3]));

            i++;
        }

    file.close();
    }


}
