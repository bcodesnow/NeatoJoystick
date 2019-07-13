#include "joysticktransmitter.h"
neatoTcpProt_t ntp_str;

JoystickTransmitter::JoystickTransmitter(QObject *parent) : QObject(parent)
{
    m_transmitTimer = new QTimer();
    m_transmitTimer->setInterval(m_cmdInterval);
    connect(m_transmitTimer, SIGNAL(timeout()), this, SLOT(transmitMotorCmd()));
    m_transmitTimer->setSingleShot(false);
    m_transmitTimer->start();
}
