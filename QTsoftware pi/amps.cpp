#include "amps.h"
#include "ui_amps.h"

amps::amps(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::amps)
{
    ui->setupUi(this);
}

amps::~amps()
{
    delete ui;
}
