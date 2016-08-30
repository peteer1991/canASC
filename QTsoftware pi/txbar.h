#ifndef TXBAR_H
#define TXBAR_H

#include <QDialog>


namespace Ui {
class TXbar;
}

class TXbar : public QDialog
{
    Q_OBJECT

public:
    explicit TXbar(QWidget *parent = 0);
    ~TXbar();
    void update_power(int t,int f , int r);

private:
    Ui::TXbar *ui;

};

void Rotorc_TX();
#endif // TXBAR_H
