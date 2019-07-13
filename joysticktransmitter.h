#ifndef JOYSTICKTRANSMITTER_H
#define JOYSTICKTRANSMITTER_H

#include <QObject>
#include <QPointF>
#include <QDebug>
#include <QTimer>
#include <QtMath>

#include "neatotcpprot.h"
#include "../TCPSender/tcpsender.h"

class JoystickTransmitter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF polarPos READ getPolarPos WRITE setPolarPos NOTIFY polarPosChanged)
public:
    char buf1[128];
    char buf2[128];
    uint32_t len1;
    uint32_t len2;
    explicit JoystickTransmitter(QObject *parent = nullptr);

    void setPolarPos(QPointF point) {
        m_pos = point;
        //qDebug()<<"Last polar point:"<<point.x()<<point.y();
        emit polarPosChanged();
    }

    QPointF getPolarPos() {
        return m_pos;
    }
    void set_ref_to_tcpSender ( TCPSender* ref)
    {
        m_ref_to_tcpSender = ref;
        m_isTcpSenderActive = 1;
    }


    void sendMoveMsgLR (uint8_t type, uint16_t speed, int16_t distance)
    {


        if (type == PROT_LEFT_WHEEL_MOVE)
        {
            len1 = sprintf (buf1, "setmotor lwheeldist %d speed %d\n", speed, distance);
            m_ref_to_tcpSender->add_job(&buf1[0], &len1, SEND_MODE_WITHOUT_REPLY);
        }
        else
        {
            len2 = sprintf (buf2, "setmotor rwheeldist %d speed %d\n", speed, distance);
            m_ref_to_tcpSender->add_job(&buf2[0], &len2, SEND_MODE_WITHOUT_REPLY);
        }

        uint8_t chkSum = 0;
        chkSum+= ntp_str.sendBuf[0] = type;
        chkSum+= ntp_str.sendBuf[1] = ( ( uint16_t ) speed ) >> 8;
        chkSum+= ntp_str.sendBuf[2] = ( ( uint16_t ) speed ) & 0xFF;
        chkSum+= ntp_str.sendBuf[3] = ( ( int16_t ) distance ) >> 8;
        chkSum+= ntp_str.sendBuf[4] = ( ( int16_t ) distance ) & 0xFF;
        ntp_str.sendBuf[5] = PROT_FINISHED_0;
        ntp_str.sendBuf[6] = PROT_FINISHED_1;
        ntp_str.sendBuf[7] = chkSum;
        ntp_str.protLen = 8;

//        qDebug()<<"Buffer:";
//        for (int i=0;i<ntp_str.protLen;i++) {
//            qDebug()<<ntp_str.sendBuf[i];
//        }

//        uint8_t *mongoptr;
//        mongoptr = ntp_str.sendBuf;
        qDebug()<<"check sum sender:"<<chkSum;
        mongoReceiver(ntp_str.sendBuf, ntp_str.protLen);
             //   &ntp_str.sendBuf;

    }

    void mongoReceiver(uint8_t* data, uint16_t length)
    {
        Q_UNUSED(length)
        uint8_t chkSumRcv = 0;
        uint8_t type;
        uint16_t speed;
        int16_t distance;
        chkSumRcv += type = data[0];
        chkSumRcv += speed = ( (uint16_t)  data[1] ) << 8;
        chkSumRcv += speed |= ( (uint16_t) data[2] );
        chkSumRcv += distance = ( (int16_t) data[3]) << 8;
        chkSumRcv += distance |= ( (uint16_t) data[4] );
        qDebug()<<"check sum receiver:"<<chkSumRcv;






    }






signals:
    void polarPosChanged();

public slots:
    void transmitMotorCmd() {
        int16_t wheelDistL = 0;
        int16_t wheelDistR = 0;
        // calculate speed
        double pol_r = m_pos.x();
        double pol_phi = m_pos.y();

        uint16_t maxWheelSpeed = 300; // mm/s

        if (pol_r > 1) pol_r = 1;
        //if (pol_r < 0.1) pol_r = 0; // sector 0: no movement

        uint16_t wheelSpeed = static_cast<uint16_t>( pol_r*maxWheelSpeed );
        // adjust wheel distance depending on speed
        int16_t stepDistance = wheelSpeed * m_cmdInterval/1000;// timerStep/1000

        /* pol_phi:
         * R -> +- 0
         * up -> - pi/2
         * down -> + pi/2
         * L -> +- pi
         *
         */

        // set distance depending on angle
        double lrRatio;
        double angleOffset = M_PI*0.02;
        if (pol_r > 0)
        {
            // sector UP
            if (pol_phi <= -M_PI_2+angleOffset && pol_phi >= -M_PI_2-angleOffset)
            {
                wheelDistL = stepDistance;
                wheelDistR = stepDistance;
                qDebug()<<"SECTOR: UP";
            }
            // sector DOWN
            else if (pol_phi <= M_PI_2+angleOffset && pol_phi >= M_PI_2-angleOffset)
            {
                wheelDistL = -stepDistance;
                wheelDistR = -stepDistance;
                qDebug()<<"SECTOR: DOWN";
            }
            // sector LEFT
            else if (pol_phi >= M_PI-angleOffset || pol_phi <= -M_PI+angleOffset)
            {
                wheelDistL = -stepDistance;
                wheelDistR = stepDistance;
                qDebug()<<"SECTOR: LEFT";
            }
            // sector RIGHT
            else if (pol_phi >= 0-angleOffset && pol_phi <= 0+angleOffset)
            {
                wheelDistL = stepDistance;
                wheelDistR = -stepDistance;
                qDebug()<<"SECTOR: RIGHT";
            }

            else if (pol_phi <= 0) {
                if (pol_phi > -M_PI_2)
                {
                    // sector UP_RIGHT
                    lrRatio = (pol_phi/-M_PI_2)*2-1;//(pol_phi/-M_PI_2-0.5)*2;
                    wheelDistL = stepDistance;
                    wheelDistR = static_cast<int16_t>( stepDistance * lrRatio );
                }
                else {
                    // sector UP_LEFT
                    lrRatio = ((pol_phi+M_PI_2)/M_PI_2+0.5)*2;
                    wheelDistL = static_cast<int16_t>( stepDistance * lrRatio );
                    wheelDistR = stepDistance;
                }
            }
            else {
                if (pol_phi < M_PI_2)
                {
                    // sector DOWN_RIGHT
                    lrRatio = (pol_phi/M_PI_2)*2-1;//(pol_phi/-M_PI_2-0.5)*2;
                    wheelDistL = static_cast<int16_t>( -stepDistance * lrRatio );
                    wheelDistR = -stepDistance;
                }
                else {
                    // sector DOWN_LEFT
                    lrRatio = (1-(pol_phi-M_PI_2)/M_PI_2-0.5)*2;
                    wheelDistL = -stepDistance;
                    wheelDistR = static_cast<int16_t>( -stepDistance * lrRatio );
                }
            }
        }
        sendMoveMsgLR(PROT_LEFT_WHEEL_MOVE,wheelSpeed,wheelDistL);
        sendMoveMsgLR(PROT_RIGHT_WHEEL_MOVE,wheelSpeed,wheelDistR);
        // qDebug()<<"transmit cmd: SPEED"<<wheelSpeed<<"WHEEL L"<<wheelDistL<<"WHEEL R"<<wheelDistR;
    }

private:
    TCPSender* m_ref_to_tcpSender;
    quint8 m_isTcpSenderActive;
    QPointF m_pos;
    QTimer *m_transmitTimer;
    uint16_t m_cmdInterval = 100; // ms)

    // int stepDistance = 100; // mm
};

#endif // JOYSTICKTRANSMITTER_H
