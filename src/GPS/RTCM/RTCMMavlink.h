/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QUdpSocket>
#include <QSerialPort>
#include <QHostAddress>

#include "QGCToolbox.h"
#include "MAVLinkProtocol.h"
#include "QGCLoggingCategory.h"

Q_DECLARE_LOGGING_CATEGORY(RTCMLog)

/**
 ** class RTCMMavlink
 * Receives RTCM updates and sends them via MAVLINK to the device
 */
class RTCMMavlink : public QObject
{
    Q_OBJECT
public:
    RTCMMavlink(QGCToolbox& toolbox);
    ~RTCMMavlink();
    //TODO: API to select device(s)?

public slots:
    void RTCMDataUpdate     (QByteArray message);

private slots:
    void _receiveMessage    (LinkInterface* link, mavlink_message_t message);

private:
    void sendMessageToVehicle(const mavlink_gps_rtcm_data_t& msg);

    QGCToolbox&     _toolbox;
    QElapsedTimer   _bandwidthTimer;
    int             _bandwidthByteCounter = 0;
    uint8_t         _sequenceId = 0;
    QUdpSocket*     _udpSocket = nullptr;
    QHostAddress    _targetAddress;
    uint16_t        _targetPort = 0;
    QSerialPort*    _serialPort = nullptr;
    int             _mavlinkChannel = 0;
    QElapsedTimer   _heartbeatTimer;
};
