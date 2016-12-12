#include "rotorc.h"
#include <QtCore>
#include <QtGui>
#include <QtWidgets/QMessageBox>
#include <QtSerialPort/QtSerialPort>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTouchEvent>



#include <QDebug>
#include <assert.h>
#include <stdio.h>
#include <QMessageBox>
#include <QString>

#define PI 3.14159265

Rotorc::Rotorc(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Rotorc)
{

    ui->setupUi(this);


    counter = 0;


    timer10ms.setInterval(10);
    //connect(&timer10ms, SIGNAL(timeout()), this, SLOT(on_sendButton_clicked()));



    qApp->installEventFilter(this);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents);
}

Rotorc::~Rotorc()
{
    delete ui;
}
bool Rotorc::eventFilter(QObject *, QEvent * pEvent)
{
    switch(pEvent->type())
    {
        case QEvent::TouchBegin:
       {
            qDebug("BEGIN");
            QList<QTouchEvent::TouchPoint> touchBeginPoints =dynamic_cast<QTouchEvent *>(pEvent)->touchPoints();
            foreach (const QTouchEvent::TouchPoint &touchBeginPoint, touchBeginPoints)
            {
                float touchBeginX = touchBeginPoint.pos().x();
                float touchBeginY = touchBeginPoint.pos().y();
                if(touchBeginX > 37 && touchBeginX < 480 && touchBeginY >25 )
                {
                    double deltaY = (touchBeginY - 240);
                    double deltaX = (touchBeginX - 240);
                    double  result = (atan2 (deltaY,deltaX) * 180 / PI)+90;
                    if(result <0)
                    {
                        result = 360+result;
                    }
                    //qDebug() <<position;
                    ui->Degree->display((int)result);
                    this->degree =(int)result;
                    this->send_position();

                    QWidget::update();
                    break;
                   // qDebug() << "touchBeginPoint := " << result << ",  " << touchBeginY;
                }

            }
        }
         break;

    default:
        break;
    };
return false;
}


void Rotorc::on_pushButton_2_clicked()
{
   /*
    QMessageBox* msgBox = new QMessageBox();
    msgBox->setWindowTitle("Rotors");
    msgBox->setText("There is no rotor controller attached to the system");
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();
    ui->Degree->display(100);
*/
    this->Rotorc_TX();

}
void Rotorc::paintEvent(QPaintEvent *e)
{
    QImage myImage;
    if(this->curent_map ==1)
    {
        myImage.load(":/images/map_vhf.png");
    }
    else
    {
        myImage.load(":/images/map.png");
    }

    QRectF rectangle(30, 30, 450, 450);
    int startAngle = ((this->degree)-20-90) * -16;
    //int startAngle2 = 33 * -16;
    int spanAngle = 40 * -16;


    QPainter painter(this);
    painter.drawImage(30, 30, myImage);
    painter.setBrush(QColor(0, 0, 255, 127));
    painter.drawPie(rectangle, startAngle, spanAngle);
    // not in use att this moment
    //painter.setBrush(QColor(255, 0, 0, 127));
    //painter.drawPie(rectangle, startAngle2, spanAngle);

}


void Rotorc::Rotorc_TX()
{

    tx_bar.setModal(true);
    // hide the qt bar
    // txbar.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    tx_bar.setStyleSheet("background-color: black;color :#ffffff;font-weight: bold;");
    tx_bar.exec();


}



void Rotorc::on_actionExit_triggered()
{
    QCoreApplication::quit();
}


void Rotorc::on_dial_sliderMoved(int position)
{
    //qDebug() <<position;
    ui->Degree->display(position);
    this->degree =position;
    QWidget::update();
    this->send_position();


}
void Rotorc::msgReceived(QString msg)
{
    // Append message to log
    ui->logEdit->appendPlainText(msg);


    // Increase message counter
    m_receivedMessages++;
    ui->label_2->setText(QString::number(m_receivedMessages));
}
// Change CAN net


void Rotorc::txpower(const int tot ,const int fwd,const int swr)
{
 //    ui->label->setText(QString::number(fwd));
 //   ui->label_3->setText(QString::number(swr));
    tx_bar.update_power(300,fwd,swr);

}
/* set the valu of the recived freq*/

void Rotorc::Radio_frq(QString freq)
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
            ui->band->setText(QString::number(band, 'f', 2)+" Cm");
            toReturn.insert(3,".");
            toReturn.insert(7,".");
       }
       else
       {
           ui->band->setText(QString::number(band, 'f', 2)+" M");
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

    ui->Fq->setText(toReturn);

}
void Rotorc::Radio_mode(int ptt,int amp_id,int mode)
{

    switch(mode)
    {
        case 0:
            ui->mode->setText("LSB");
            break;
        case 1:
            ui->mode->setText("USB");
            break;
        case 2:
            ui->mode->setText("CW");
            break;
        case 3:
            ui->mode->setText("CW-R");
            break;
        case 4:
            ui->mode->setText("AM");
            break;
        case 5:
            ui->mode->setText("CW-N");
            break;
        case 6:
            ui->mode->setText("WFM");
            break;
        case 7:
            ui->mode->setText("FM");
            break;
        case 8:
            ui->mode->setText("NFM");
            break;

        case 9:
            ui->mode->setText("DIG");
            break;

        case 10:
            ui->mode->setText("PKT");
            break;
        default:

            break;

    }



}


double  Rotorc::extractDouble(QString s)
{
    QString num;
    foreach(QChar c, s) {
        if (c.isDigit() || c == '.' || c == '-') {
            num.append(c);
        }
    }
    bool ok;
    double ret = num.toDouble(&ok);
    if (ok) {
        return ret;
    } else {
        throw "Cannot extract double value";
    }
}


void Rotorc::show_tx()
{
    tx_bar.setModal(true);
    // hide the qt bar
    // txbar.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    tx_bar.setStyleSheet("background-color: black;color :#ffffff;font-weight: bold;");
    tx_bar.exec();


}
void Rotorc::hide_tx()
{
    tx_bar.hide();
}

void Rotorc::on_can_button_clicked()
{
    int ret;
    int errorCode;

    // Close old connection (if there is one) and shut down worker threads
    can->Close();

    // Tell thread to shut down, force if problem
    m_workerThread->shutDown();

    if(!m_workerThread->wait(3000))
    {
        m_workerThread->terminate();
    }

    // Init new connection
    QString str;
    str  = "can0";

    ret = can->Init(str.toUtf8(), errorCode);
    if(!ret)
    {
        QMessageBox msgBox;
        msgBox.setText("Could not initialize CAN net. Error code: " + QString::number(errorCode));
        msgBox.exec();

        return;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("Connected to Can-bus can0");
        msgBox.exec();
    }

    // Enable error messages to be reported
    can->EnableErrorMessages();

    // initialize worker thread
    m_workerThread->Init(can);

    // Start thread
    m_workerThread->start();
}

void Rotorc::on_pushButton_8_clicked()
{
    this->curent_map =0;
    QWidget::update();
}

void Rotorc::on_pushButton_7_clicked()
{
    this->curent_map =1;
    QWidget::update();
}
void Rotorc::send_position()
{
      struct can_frame msg;
      int i;
      int ret;
      int errorCode;

      msg.can_id = 0x01; // ID selected by random
      msg.can_dlc = 8;

      // Clear data
      for(i = 0; i < 8; i++)
      {
          msg.data[i] = 0;
      }

      // First byte contains message number
      msg.data[0] = 1;
      msg.data[1] = 2;
       if((this->degree-255 ) >0 )
       {
            msg.data[3] = this->degree -255;
            msg.data[2] = 255;
       }
       else
        {
            msg.data[2] =0;
            msg.data[3] = this->degree;
       }

      ret = can->SendMsg(msg, false, false, errorCode);


      if(!ret)
      {
/*          msgBox.setText("Could not send CAN message, aborting. Error code: " + QString::number(errorCode));
          msgBox.exec();
*/
          // Quit loop at error
          return;
      }

}


void Rotorc::on_Amp_press_clicked()
{


    amp_v.setModal(true);
    // hide the qt bar
    // txbar.setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //amps.setStyleSheet("background-color: black;color :#ffffff;font-weight: bold;");
    amp_v.showFullScreen();
    //amp_v.exec();
}
