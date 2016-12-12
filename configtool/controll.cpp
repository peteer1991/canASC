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

    if(query.length()>4)
    {

        if(query[0] == "1")
        {
            ui->radio->setText(query[1]);
            //ui->freqvensy->setText(query[3]);
            if(ui->bussconfig->rowCount() >= (query[2].toInt()) && query[2].toInt() >0)
            {
                QString ampname = ui->bussconfig->item((query[2].toInt()-1),2)->text();
                ui->current_amp->setText(ampname);
            }
            else
            {
               ui->current_amp->setText("No amplifier is selected");

            }
        this->Radio_frq(query[3]);
        }

         qDebug() << query; // outputs "000012"
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
void Controll::Radio_frq(QString freq)
{

    QString toReturn = freq;
    toReturn =toReturn.remove( QRegExp( "[^0-9.]" ) );

    int freqv =toReturn.toInt();


    float band;
    int cm =0;
    if(freqv >0)
   {
       band= (float)(30000000/(float)freqv);
       if(band < 1)
       {

            band= band*100;
            cm=1;
            ui->Band->setText(QString::number(band, 'f', 2)+" Cm");
            toReturn.insert(3,".");
            toReturn.insert(7,".");
       }
       else
       {
           ui->Band->setText(QString::number(band, 'f', 2)+" M");
           cm =0;

           if(band >30)
           {
               toReturn.insert(1,".");
               toReturn.insert(5,".");
           }
           else if(band > 3)
           {
               toReturn.insert(2,".");
               toReturn.insert(6,".");
           }
           else
           {
               toReturn.insert(3,".");
               toReturn.insert(7,".");

           }
       }
   }

    ui->freqvensy->setText(toReturn);

}
