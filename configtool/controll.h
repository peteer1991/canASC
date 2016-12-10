#ifndef CONTROLL_H
#define CONTROLL_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

namespace Ui {
class Controll;
}

class Controll : public QMainWindow
{
    Q_OBJECT

public:
    explicit Controll(QWidget *parent = 0);
    ~Controll();

private slots:
    void on_pushButton_clicked();

    void on_connect_button_clicked();
    void serialRecived();

    void on_Cvs_load_clicked();

private:
    Ui::Controll *ui;
    QStandardItemModel *model;
};

#endif // CONTROLL_H
