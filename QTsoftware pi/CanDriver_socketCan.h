#ifndef CANDRIVER_LINUXSOCKETCAN_H
#define CANDRIVER_LINUXSOCKETCAN_H

#include <QThread>

#include "QtCanOpen.h"

class PollWorker : public QObject {
    Q_OBJECT

public:
    explicit PollWorker (int sockfd);

public slots:
    void poll ();
    void stop ();

signals:
    void frameReceived (quint16 cobId, quint8 length, QByteArray data, bool rtr);

private:
    int  m_sockfd;
    bool m_exit;
};

class CanDriver_socketCan : public CanDriver {
    Q_OBJECT
    Q_CLASSINFO ("driverName", "SocketCAN")
#ifndef QT_CANOPEN_STATIC_DRIVERS
    Q_PLUGIN_METADATA (IID "QtCanOpen.Drivers")
    Q_INTERFACES (CanDriver)
#endif

public:
    Q_INVOKABLE explicit CanDriver_socketCan (QObject * parent = NULL);
    virtual ~CanDriver_socketCan ();

public slots: // CanDriver interface
    virtual void init (QVariantMap   options);
    virtual void send (CanMessage  * message);

private slots:
    void onFrameReceived (quint16 cobId, quint8 length, QByteArray data, bool rtr);

private:
    int          m_sockfd;
    bool         m_valid;
    QThread    * m_thread;
    PollWorker * m_poller;
};

#endif // CANDRIVER_LINUXSOCKETCAN_H
