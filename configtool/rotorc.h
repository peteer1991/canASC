#ifndef ROTORC_H
#define ROTORC_H
#include <QMainWindow>
#include <QTimer>


namespace Ui {
class Rotorc;
}

class Rotorc : public QMainWindow
{
    Q_OBJECT

public:
    explicit Rotorc(QWidget *parent = 0);
    ~Rotorc();

private slots:
    void on_pushButton_2_clicked();
    void on_actionExit_triggered();
    void on_dial_sliderMoved(int position);
    void msgReceived(QString msg);
    void txpower(int tot ,int fwd,int swr);
    void on_can_button_clicked();
    void Rotorc_TX();
    void on_pushButton_8_clicked();
    void on_pushButton_7_clicked();
    void send_position();
    void on_Amp_press_clicked();
    void Radio_frq(QString freq);
    void Radio_mode(int ,int ,int);

protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *, QEvent * pEvent);
    int curent_map;

private:
    Ui::Rotorc *ui;
    int degree;
    void show_tx();
    void hide_tx();
    double extractDouble(QString s);

    QTimer timer10ms;
    int counter;

};

#endif // ROTORC_H
