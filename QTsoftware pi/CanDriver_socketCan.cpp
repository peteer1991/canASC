
#include "CanDriver_socketCan.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>

#define CAN_SOCKET     socket
#define CAN_CLOSE      close
#define CAN_RECV       read
#define CAN_SEND       write
#define CAN_BIND       bind
#define CAN_IOCTL      ioctl
#define CAN_SETSOCKOPT setsockopt

#define DRIVER_DEBUG "CanDriver SocketCAN :"

#ifndef CAN_MAX_DLEN
#define CAN_MAX_DLEN 8
#endif

CanDriver_socketCan::CanDriver_socketCan (QObject * parent)
    : CanDriver (parent)
    , m_sockfd  (-1)
    , m_valid   (false)
    , m_thread  (NULL)
    , m_poller  (NULL)
{
    m_thread = new QThread (this);
}

CanDriver_socketCan::~CanDriver_socketCan () {
    if (m_valid) {
        m_valid = false;
        m_poller->stop ();
        m_thread->quit ();
        m_thread->wait ();
        CAN_CLOSE (m_sockfd);
    }
}

void CanDriver_socketCan::init (QVariantMap options) {
    ifreq ifr;
    sockaddr_can addr;
    int err = 0;
    QByteArray busName = options.value ("busName", "0").toByteArray ();
    bool isNum;
    busName.left (1).toInt (&isNum);
    if (isNum) {
        busName.prepend ("can");
    }
    INFO << DRIVER_DEBUG << "Creating a CAN socket file descriptor...";
    m_sockfd = CAN_SOCKET (PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_sockfd >= 0) {
        INFO << DRIVER_DEBUG << "Setting iFreq parameters for CAN socket...";
        qstrncpy (ifr.ifr_name, busName.constData (), IFNAMSIZ);
        err = CAN_IOCTL (m_sockfd, SIOCGIFINDEX, &ifr);
        if (!err) {
            INFO << DRIVER_DEBUG << "Setting CAN socket option 'CAN_RAW_LOOPBACK' to true...";
            int loopback = true;
            if (CAN_SETSOCKOPT (m_sockfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof (loopback))) {
                WARN << DRIVER_DEBUG << "Set CAN socket option 'CAN_RAW_LOOPBACK' failed !";
            }
            INFO << DRIVER_DEBUG << "Setting CAN socket option 'CAN_RAW_RECV_OWN_MSGS' to false...";
            int recv_own_msgs = false;
            if (CAN_SETSOCKOPT (m_sockfd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof (recv_own_msgs))) {
                WARN << DRIVER_DEBUG << "Set CAN socket option 'CAN_RAW_RECV_OWN_MSGS' failed !";
            }
            INFO << DRIVER_DEBUG << "Setting CAN socket option 'SO_RCVTIMEO' to 1 sec...";
            struct timeval timeout;
            timeout.tv_sec  = 1;
            timeout.tv_usec = 0;
            if (CAN_SETSOCKOPT (m_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof (timeout))) {
                WARN << DRIVER_DEBUG << "Set CAN socket option 'CAN_RAW_RECV_OWN_MSGS' failed !";
            }
            INFO << DRIVER_DEBUG << "Connecting CAN socket...";
            addr.can_family  = AF_CAN;
            addr.can_ifindex = ifr.ifr_ifindex;
            err = CAN_BIND (m_sockfd, (sockaddr *) &addr, sizeof (addr));
            if (!err) {
                INFO << DRIVER_DEBUG << "CAN socket connected and configured : OK.";
                m_valid = true;
                m_poller = new PollWorker (m_sockfd);
                m_poller->moveToThread (m_thread);
                connect (m_thread, &QThread::started,
                         m_poller, &PollWorker::poll,
                         Qt::UniqueConnection);
                connect (m_poller, &PollWorker::frameReceived,
                         this,     &CanDriver_socketCan::onFrameReceived,
                         Qt::UniqueConnection);
                m_thread->start (QThread::HighPriority);
                INFO << DRIVER_DEBUG << "Start polling...";
            }
            else {
                WARN << DRIVER_DEBUG << "Connecting CAN socket failed !";
            }
        }
        else {
            WARN << DRIVER_DEBUG << "Getting IF index for" << ifr.ifr_name << "failed";
        }
    }
    else {
        WARN << DRIVER_DEBUG << "Socket creation failed !";
    }
}

void CanDriver_socketCan::send (CanMessage * message) {
    INFO << DRIVER_DEBUG << "CAN socket, sending message" << message << "...";
    if (m_valid) {
        if (message != NULL) {
            can_frame frame;
            frame.can_id = message->getCobId ();
            if (frame.can_id >= 0x800) {
                frame.can_id |= CAN_EFF_FLAG;
            }
            frame.can_dlc = message->getLength ();
            if (message->getRtr ()) {
                frame.can_id |= CAN_RTR_FLAG;
            }
            else {
                memcpy (frame.data, message->getData ().constData (), 8);
            }
            int bytes = CAN_SEND (m_sockfd, &frame, sizeof (frame));
            if (bytes >= 0) {
                INFO << DRIVER_DEBUG << "CAN socket" << bytes << "bytes sent.";
            }
            else {
                WARN << DRIVER_DEBUG << "CAN socket send failed !";
            }
        }
        else {
            WARN << DRIVER_DEBUG << "Can't send NULL message to CAN socket !";
        }
    }
    else {
        WARN << DRIVER_DEBUG << "Can't send message because state is not valid !";
    }
}

void CanDriver_socketCan::onFrameReceived(quint16 cobId, quint8 length, QByteArray data, bool rtr) {
    emit received (new CanMessage (cobId, length, data, rtr));
}

PollWorker::PollWorker (int sockfd)
    : m_sockfd (sockfd)
    , m_exit   (false)
{ }

void PollWorker::poll () {
    static can_frame  frame;
    static size_t     size = sizeof (frame);
    forever {
        if (!m_exit) {
            if (CAN_RECV (m_sockfd, &frame, size) > 0) {
                emit frameReceived (quint16 (frame.can_id & CAN_EFF_MASK),
                                    quint8 (frame.can_dlc),
                                    QByteArray::fromRawData ((char *) frame.data, CAN_MAX_DLEN),
                                    bool (frame.can_id & CAN_RTR_FLAG));
            }
        }
        else {
            WARN << DRIVER_DEBUG << "Exit poller";
            break;
        }
    }
}

void PollWorker::stop () {
    m_exit = true;
}
