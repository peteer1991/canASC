#include "txbar.h"
#include "ui_txbar.h"

TXbar::TXbar(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TXbar)
{
    ui->setupUi(this);


}

TXbar::~TXbar()
{
    delete ui;
}
//Updatse value for fwd on screen
void TXbar::update_power(int t,int f,int r)
{
    double fwd_pross=(f*100)/t;
    double ref_pross=(r*100)/t;
    ui->FWD->setValue((int)fwd_pross);
    ui->REF->setValue((int)ref_pross);
    ui->fwdp->setText(QString::number(f)+" W");
    ui->refp->setText(QString::number(r)+" W");
    // calculate SWR ratio
    if(f >0 && r >0 ){
        double Vratio = (r / f);
        double SWR = ((1 + Vratio)/ (1 - Vratio));
        ui->SWR->setText("1:"+QString::number(SWR));
    }


}
