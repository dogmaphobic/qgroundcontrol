/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include "RTCMMavlink.h"

#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "QGCApplication.h"
#include "SettingsManager.h"

#include <cstdio>

QGC_LOGGING_CATEGORY(RTCMLog, "RTCMLog")

// Add to protocol

static MAV_TYPE      MAV_TYPE_RTCM    = static_cast<MAV_TYPE>(40);
static MAV_COMPONENT MAV_COMP_ID_RTCM = static_cast<MAV_COMPONENT>(253);

RTCMMavlink::RTCMMavlink(QGCToolbox& toolbox)
    : _toolbox(toolbox)
{
    _bandwidthTimer.start();
    _heartbeatTimer.start();
    RTKSettings* rtkSettings = qgcApp()->toolbox()->settingsManager()->rtkSettings();
    // Are we forwarding?
    if(rtkSettings->forwardRTCM()->rawValue().toBool()) {
        if(rtkSettings->forwardRTCMURI()->rawValue().toString().startsWith("UDP")){
            QString add = rtkSettings->forwardRTCMAddress()->rawValue().toString();
            qCDebug(RTCMLog) << "Forwarding RTCM to UDP:" << add;
            if(!add.isEmpty()) {
                _targetAddress = QHostAddress(add);
                _targetPort = static_cast<uint16_t>(rtkSettings->forwardRTCMPort()->rawValue().toUInt());
                _udpSocket = new QUdpSocket(this);
                _udpSocket->setProxy(QNetworkProxy::NoProxy);
                _udpSocket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 256 * 1024);
                _mavlinkChannel = _mavlinkChannel = qgcApp()->toolbox()->linkManager()->_reserveMavlinkChannel();
            }
        } else {
            QString uart = rtkSettings->forwardRTCMURI()->rawValue().toString();
            if(!uart.isEmpty()) {
                qCDebug(RTCMLog) << "Forwarding RTCM to Serial:" << uart;
                _serialPort = new QSerialPort(uart, this);
                if (_serialPort->open(QIODevice::WriteOnly)) {
                    _serialPort->setBaudRate     (QSerialPort::Baud57600);
                    _serialPort->setDataBits     (QSerialPort::Data8);
                    _serialPort->setFlowControl  (QSerialPort::NoFlowControl);
                    _serialPort->setStopBits     (QSerialPort::OneStop);
                    _serialPort->setParity       (QSerialPort::NoParity);
                } else {
                    qWarning() << "Error opening RTCM port:" << uart;
                }
            }
        }
    }
    // Are we listening?
    if(rtkSettings->listenRTCM()->rawValue().toBool()) {
        qCDebug(RTCMLog) << "Listening for RTCM messages";
        MAVLinkProtocol* mavlinkProtocol = qgcApp()->toolbox()->mavlinkProtocol();
        connect(mavlinkProtocol, &MAVLinkProtocol::messageReceived, this, &RTCMMavlink::_receiveMessage);
    }

}

RTCMMavlink::~RTCMMavlink()
{
    if(_udpSocket) {
        _udpSocket->deleteLater();
    }
    if (_serialPort) {
        _serialPort->close();
        _serialPort->deleteLater();
    }
}

void RTCMMavlink::RTCMDataUpdate(QByteArray message)
{
    /* statistics */
    _bandwidthByteCounter += message.size();
    qint64 elapsed = _bandwidthTimer.elapsed();
    if (elapsed > 1000) {
        QString txt;
        txt.sprintf("RTCM bandwidth: %.2f kB/s\n", static_cast<double>(_bandwidthByteCounter / elapsed * 1000.f / 1024.f));
        qCDebug(RTCMLog) << txt;
        _bandwidthTimer.restart();
        _bandwidthByteCounter = 0;
    }

    const int maxMessageLength = MAVLINK_MSG_GPS_RTCM_DATA_FIELD_DATA_LEN;
    mavlink_gps_rtcm_data_t mavlinkRtcmData;
    memset(&mavlinkRtcmData, 0, sizeof(mavlink_gps_rtcm_data_t));

    if (message.size() < maxMessageLength) {
        mavlinkRtcmData.len     = static_cast<uint8_t>(message.size());
        mavlinkRtcmData.flags   = static_cast<uint8_t>((_sequenceId & 0x1F) << 3);
        memcpy(&mavlinkRtcmData.data, message.data(), static_cast<size_t>(message.size()));
        sendMessageToVehicle(mavlinkRtcmData);
    } else {
        // We need to fragment
        // Fragment id indicates the fragment within a set
        uint8_t fragmentId = 0;
        int start = 0;
        while (start < message.size()) {
            int length = std::min(message.size() - start, maxMessageLength);
            mavlinkRtcmData.flags = 1;                                                  // LSB set indicates message is fragmented
            mavlinkRtcmData.flags |= static_cast<uint8_t>(fragmentId++ << 1);           // Next 2 bits are fragment id
            mavlinkRtcmData.flags |= static_cast<uint8_t>((_sequenceId & 0x1F) << 3);   // Next 5 bits are sequence id
            mavlinkRtcmData.len = static_cast<uint8_t>(length);
            memcpy(&mavlinkRtcmData.data, message.data() + start, static_cast<size_t>(length));
            sendMessageToVehicle(mavlinkRtcmData);
            start += length;
        }
    }
    ++_sequenceId;
}

void RTCMMavlink::sendMessageToVehicle(const mavlink_gps_rtcm_data_t& msg)
{
    QmlObjectListModel& vehicles = *_toolbox.multiVehicleManager()->vehicles();
    MAVLinkProtocol* mavlinkProtocol = _toolbox.mavlinkProtocol();
    // Send to all connected vehicles
    for (int i = 0; i < vehicles.count(); i++) {
        Vehicle* vehicle = qobject_cast<Vehicle*>(vehicles[i]);
        mavlink_message_t message;
        mavlink_msg_gps_rtcm_data_encode_chan(
            static_cast<uint8_t>(mavlinkProtocol->getSystemId()),
            static_cast<uint8_t>(mavlinkProtocol->getComponentId()),
            vehicle->priorityLink()->mavlinkChannel(),
            &message,
            &msg);
        vehicle->sendMessageOnLink(vehicle->priorityLink(), message);
    }
    // If forwarding RTCM, do it
    if (_udpSocket || _serialPort) {
        qint64 elapsed = _heartbeatTimer.elapsed();
        //-- Send heartbeat
        if (elapsed > 1000) {
            _heartbeatTimer.start();
            mavlink_message_t   msg;
            mavlink_msg_heartbeat_pack_chan(
                static_cast<uint8_t>(mavlinkProtocol->getSystemId()),
                MAV_COMP_ID_RTCM,
                static_cast<uint8_t>(_mavlinkChannel),
                &msg,
                MAV_TYPE_RTCM,          // MAV_TYPE
                MAV_AUTOPILOT_GENERIC,  // MAV_AUTOPILOT
                0,                      // MAV_MODE
                0,                      // custom mode
                0);                     // MAV_STATE
            mavlink_message_t message;
            uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
            int len = mavlink_msg_to_send_buffer(buffer, &message);
            if(_udpSocket) {
                _udpSocket->writeDatagram(reinterpret_cast<char*>(buffer), len, _targetAddress, _targetPort);
            } else {
                _serialPort->write(reinterpret_cast<char*>(buffer), len);
            }
        }
        //-- Send RTCM Payload
        mavlink_message_t message;
        mavlink_msg_gps_rtcm_data_encode_chan(
            static_cast<uint8_t>(mavlinkProtocol->getSystemId()),
            MAV_COMP_ID_RTCM,
            static_cast<uint8_t>(_mavlinkChannel),
            &message,
            &msg);
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        int len = mavlink_msg_to_send_buffer(buffer, &message);
        if(_udpSocket) {
            _udpSocket->writeDatagram(reinterpret_cast<char*>(buffer), len, _targetAddress, _targetPort);
        } else {
            _serialPort->write(reinterpret_cast<char*>(buffer), len);
        }
        qCDebug(RTCMLog) << "Sent MAVLINK_MSG_ID_GPS_RTCM_DATA";
    }
}

//-----------------------------------------------------------------------------
void
RTCMMavlink::_receiveMessage(LinkInterface*, mavlink_message_t message)
{
    //-- Note that there is no authentication. Anything sending us these messages will
    //   be accepted. But then again, that's true for any MAVLink traffic.
    if (message.msgid == MAVLINK_MSG_ID_GPS_RTCM_DATA && message.compid == MAV_COMP_ID_RTCM) {
        qCDebug(RTCMLog) << "Received MAVLINK_MSG_ID_GPS_RTCM_DATA";
        mavlink_gps_rtcm_data_t m;
        mavlink_msg_gps_rtcm_data_decode(&message, &m);
        sendMessageToVehicle(m);
    }
}
