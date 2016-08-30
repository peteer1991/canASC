#ifndef AMPS_H
#define AMPS_H

#include <QDialog>

namespace Ui {
class amps;
}

class amps : public QDialog
{
    Q_OBJECT

public:
    explicit amps(QWidget *parent = 0);
    ~amps();

private:
    Ui::amps *ui;
};

#endif // AMPS_H
