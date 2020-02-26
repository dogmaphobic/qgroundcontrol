/****************************************************************************
 *
 * (c) 2009-2019 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 * @file
 *   @brief Custom QtQuick Interface
 *   @author Gus Grubba <gus@auterion.com>
 */

#include "QGCApplication.h"
#include "AppSettings.h"
#include "SettingsManager.h"
#include "MAVLinkLogManager.h"
#include "QGCMapEngine.h"
#include "QGCApplication.h"
#include "PositionManager.h"
#include "TrajectoryPoints.h"

#include "CustomPlugin.h"
#include "CustomQuickInterface.h"
#include "gpxwriter.h"

#include <QSettings>
#include <QPluginLoader>

#define DISTANCE_THRESHOLD 5
#define ALTITUDE_THRESHOLD 0.25

static const char* kTrackingHeader =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
"<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" version=\"1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\" creator=\"QGCSurveyor\">\n" \
"  <trk>\n" \
"    <name><![CDATA[Track]]></name>\n" \
"    <desc><![CDATA[##TIME_STAMP##]]></desc>\n" \
"    <trkseg>\n";

static const char* kTrackingBody =
"      <trkpt lat=\"##LAT##\" lon=\"##LONG##\">\n" \
"        <ele>##ELEV##</ele>\n" \
"        <time>##TIME_STAMP##</time>\n" \
"      </trkpt>\n";

static const char* kTrackingGPSData =
"        <sat>##SAT##</sat>\n" \
"        <hdop>##HDOP##</hdop>\n" \
"        <vdop>##VDOP##</vdop>\n";

static const char* kTrackingFooter =
"    </trkseg>\n" \
"  </trk>\n" \
"</gpx>\n";

//-----------------------------------------------------------------------------
CustomQuickInterface::CustomQuickInterface(QObject* parent)
    : QObject(parent)
    , _lastAltitude(qQNaN())
{
    qCDebug(CustomLog) << "CustomQuickInterface Created";
}

//-----------------------------------------------------------------------------
CustomQuickInterface::~CustomQuickInterface()
{
    qCDebug(CustomLog) << "CustomQuickInterface Destroyed";
    _trackFile.close();
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::init()
{
    MultiVehicleManager *manager = qgcApp()->toolbox()->multiVehicleManager();
    connect(manager, &MultiVehicleManager::activeVehicleChanged, this, &CustomQuickInterface::_setActiveVehicle);
    connect(&_trackTimer, &QTimer::timeout, this, &CustomQuickInterface::_trackingTime);
    GPSRTKFactGroup* gpsFG = dynamic_cast<GPSRTKFactGroup*>(qgcApp()->gpsRtkFactGroup());
    connect(gpsFG->currentAccuracy(), &Fact::rawValueChanged, this,  &CustomQuickInterface::_rtkAccuracyChanged);
    _trackTimer.setSingleShot(false);
    _trackTimer.stop();
    _startStopSound.setSource(QUrl::fromUserInput("qrc:/custom/wav/boop.wav"));
    _startStopSound.setLoopCount(1);
    _activeSound.setSource(QUrl::fromUserInput("qrc:/custom/wav/beep.wav"));
    _activeSound.setLoopCount(1);
    _captureSound.setSource(QUrl::fromUserInput("qrc:/custom/wav/camera.wav"));
    _captureSound.setLoopCount(1);
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_setActiveVehicle(Vehicle* vehicle)
{
    if(_vehicle) {
        _vehicle = nullptr;
        _trackTimer.stop();
    }
    _vehicle = vehicle;
    if(_vehicle) {
        Fact* altitudeRelative = _vehicle->altitudeRelative();
        Fact* altitudeAMSL     = _vehicle->altitudeAMSL();
        connect(altitudeRelative, &Fact::rawValueChanged, this, &CustomQuickInterface::_altitudeRelativeChanged);
        connect(altitudeAMSL,     &Fact::rawValueChanged, this, &CustomQuickInterface::_altitudeAMSLChanged);
        connect(_vehicle, &Vehicle::coordinateChanged, this, &CustomQuickInterface::_vehicleCoordinateChanged);
        _trackTimer.start(1000);
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_altitudeRelativeChanged(QVariant value)
{
    double a = value.toDouble();
    if(isfinite(a)) {
        _altitudeRelative = (_altitudeRelative * 0.95) + (value.toDouble() * 0.05);
        emit altitudeRelativeChanged();
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_altitudeAMSLChanged(QVariant value)
{
    double a = value.toDouble();
    if(isfinite(a)) {
        _altitudeAMSL = (_altitudeAMSL * 0.95) + (value.toDouble() * 0.05);
        emit altitudeAMSLChanged();
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_rtkAccuracyChanged(QVariant value)
{
    GPSRTKFactGroup* gpsFG = dynamic_cast<GPSRTKFactGroup*>(qgcApp()->gpsRtkFactGroup());
    if(gpsFG->active()->rawValue().toBool()) {
        RTKSettings* rtkSettings = qgcApp()->toolbox()->settingsManager()->rtkSettings();
        if(!_ready && value.toDouble() < rtkSettings->surveyInAccuracyLimit()->rawValue().toDouble()) {
            _ready = true;
            emit readyChanged();
            _activeSound.play();
        }
        if(_ready && value.toDouble() > rtkSettings->surveyInAccuracyLimit()->rawValue().toDouble()) {
            _ready = false;
            emit readyChanged();
            _startStopSound.setLoopCount(2);
            _startStopSound.play();
            _startStopSound.setLoopCount(1);
        }
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_trackingTime()
{

}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::reset()
{
    if(_tracking && _vehicle) {
        _vehicle->trajectoryPoints()->stop();
        _closeFile();
        _tracking = false;
        emit trackingChanged();
        _startStopSound.play();
    }
    if(_waypointWriter) {
        if(_waypoints.count()) {
            for(int i = 0; i < _waypoints.count(); i++) {
                _waypointWriter->addWaypoint(qobject_cast<GPXWayPoint*>(_waypoints.get(i)));
            }
            _waypointWriter->writeAndClose();
            _waypoints.clearAndDeleteContents();
        }
        delete _waypointWriter;
        _waypointWriter = nullptr;
    }
    _points = 0;
    emit pointsChanged();
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::setTracking(bool set)
{
    if(_tracking != set && _vehicle) {
        _tracking = set;
        emit trackingChanged();
        if(set) {
            _vehicle->trajectoryPoints()->resume();
            QString path = qgcApp()->toolbox()->settingsManager()->appSettings()->missionSavePath();
            QString timeStamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd-hhmmss");
            QString filename = QString("%1/%2.gpx").arg(path).arg(timeStamp);
            _trackFile.setFileName(filename);
            if (!_trackFile.open(QFile::WriteOnly)) {
                qWarning() << "Error creating" << filename;
            } else {
                QString header(kTrackingHeader);
                header.replace("##TIME_STAMP##", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
                _trackFile.write(header.toUtf8());
                _startStopSound.play();
                _vehicle->setArmed(true);
            }
        } else {
            _vehicle->trajectoryPoints()->pause();
            _vehicle->setArmed(false);
            _closeFile();
            _startStopSound.play();
        }
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_vehicleCoordinateChanged(QGeoCoordinate coordinate)
{
    if(_tracking) {
        // The goal of this algorithm is to limit the number of trajectory points whic represent the vehicle path.
        // Fewer points means higher performance of map display.
        if (_lastPoint.isValid()) {
            double distance = _lastPoint.distanceTo(coordinate);
            if (distance > DISTANCE_THRESHOLD) {
                // Vehicle has moved far enough from previous point for an update
                if (qIsNaN(_lastAltitude) || qAbs(_altitudeAMSL - _lastAltitude) > ALTITUDE_THRESHOLD) {
                    // Vehicle has changed elevation far enough
                    _lastAltitude = _altitudeAMSL;
                    _lastPoint = coordinate;
                    _addTracking(coordinate, _altitudeAMSL);
                }
            }
        } else {
            // Add the very first trajectory point to the list
            _lastPoint = coordinate;
            _addTracking(coordinate, _altitudeAMSL);
        }
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_addTracking(QGeoCoordinate coordinate, double altitude, bool wp)
{
    if(_tracking && _trackFile.isOpen()) {
        QString body(kTrackingBody);
        QString pad;
        pad.sprintf("%f", coordinate.latitude());
        body.replace("##LAT##", pad);
        pad.sprintf("%f", coordinate.longitude());
        body.replace("##LONG##", pad);
        pad.sprintf("%f", altitude);
        body.replace("##ELEV##", pad);
        body.replace("##TIME_STAMP##", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        VehicleGPSFactGroup* gps = dynamic_cast<VehicleGPSFactGroup*>(_vehicle->gpsFactGroup());
        if(gps && gps->lock()->rawValue().toInt() >= GPS_FIX_TYPE_3D_FIX) {
            QString bodyExta(kTrackingGPSData);
            pad.sprintf("%d", gps->count()->rawValue().toInt());
            body.replace("##SAT##", pad);
            pad.sprintf("%f", gps->hdop()->rawValue().toDouble());
            body.replace("##HDOP##", pad);
            pad.sprintf("%f", gps->vdop()->rawValue().toDouble());
            body.replace("##VDOP##", pad);
            _trackFile.write(body.toUtf8());
        }
        if(wp) {
            body.replace("trkpt", "wpt");
        }
        _trackFile.write(body.toUtf8());
        _trackFile.flush();
        if(!wp) {
            _captureSound.play();
        }
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::addWaypoint()
{
    if(_vehicle) {
        if(!_waypointWriter) {
            _waypointWriter = new GPXWriter(this);
        }
        GPXWayPoint* wp = new GPXWayPoint(QGeoCoordinate(_vehicle->coordinate().latitude(), _vehicle->coordinate().longitude(), _altitudeAMSL), _points, _altitudeRelative, this);
        wp->setGPSInfo(dynamic_cast<VehicleGPSFactGroup*>(_vehicle->gpsFactGroup()));
        _waypoints.append(wp);
        _addTracking(_vehicle->coordinate(), _altitudeAMSL, true);
        _points++;
        _lastPoint = _vehicle->coordinate();
        emit pointsChanged();
        _captureSound.play();
    }
}

//----------------------------------------------------------------------------------------
void
CustomQuickInterface::_closeFile()
{
    if(_trackFile.isOpen()) {
        _trackFile.write(kTrackingFooter);
        _trackFile.close();
    }
}
