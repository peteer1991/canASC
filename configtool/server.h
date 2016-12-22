#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(quint16 port, bool debug = false, QObject *parent = Q_NULLPTR);
    ~Server();

    void Send_heartbeat(QString message);

Q_SIGNALS:
    void closed();


private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();


private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    bool m_debug;
    QString Brodcast;

};

#endif //ECHOSERVER_H
