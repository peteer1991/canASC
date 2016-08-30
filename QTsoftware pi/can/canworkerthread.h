// Copyright 2012 CrossControl

#ifndef CANWORKERTHREAD_H
#define CANWORKERTHREAD_H

#include <QThread>

#include "canwrapper.h"
#include <linux/can.h>
#include <linux/can/raw.h>

class CanWorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit CanWorkerThread(QObject *parent = 0);

    void Init(CanWrapper *wrapper); // Initialize

    void run();         // start thread

    void shutDown();    // Make thread shut down
    QString FQ1;
    QString FQ2;



signals:
    void msgReceived(QString msg);
    void txpower(int tot ,int fwd,int swr);
    void show_tx();
    void hide_tx();
    void radio_frq(QString freq);
    void Radio_mode(int,int,int);

public slots:

private:
    CanWrapper *m_can;  // Pointer to can wrapper class
    bool m_running;     // Set to false to stop thread
    quint32 freq;



};

#endif // CANWORKERTHREAD_H
