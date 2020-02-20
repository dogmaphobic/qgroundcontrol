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

#include "gpxwriter.h"
#include "Vehicle.h"
#include "QGCApplication.h"
#include "AppSettings.h"
#include "SettingsManager.h"

//----------------------------------------------------------------------------------------
GPXWayPoint::GPXWayPoint(QGeoCoordinate coordinate, int id, QObject *parent)
    : QObject(parent)
    , _id(id)
    , _coordinate(coordinate)
    , _timeStamp(QDateTime::currentDateTimeUtc())
{
}

//----------------------------------------------------------------------------------------
void
GPXWayPoint::setGPSInfo(VehicleGPSFactGroup* gps)
{
    _gpsCount = gps->count()->rawValue().toInt();
    _gpsHdop  = gps->hdop()->rawValue().toDouble();
    _gpsVdop  = gps->vdop()->rawValue().toDouble();
}

//----------------------------------------------------------------------------------------
GPXWriter::GPXWriter(QObject *parent)
    : QObject(parent)
    , _stream(&_output)
    , _start(QDateTime::currentDateTimeUtc())
{
    _stream.setAutoFormatting(true);
    _stream.writeStartDocument();
    _stream.writeStartElement("gpx");
    _stream.writeAttribute("xmlns=",                "http://www.topografix.com/GPX/1/1");
    _stream.writeAttribute("version",               "1.1");
    _stream.writeAttribute("xmlns:xsi",             "http://www.w3.org/2001/XMLSchema-instance");
    _stream.writeAttribute("xsi:schemaLocation",    "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd");
    _stream.writeAttribute("creator",               "QGCSurveyor");
    _stream.writeStartElement("trk");
}

//----------------------------------------------------------------------------------------
void
GPXWriter::addWaypoint(GPXWayPoint *wp)
{
    if(wp) {
        QString lat, lon, elev;
        lat.sprintf("%f", wp->coordinate().latitude());
        lon.sprintf("%f", wp->coordinate().longitude());
        elev.sprintf("%f", wp->coordinate().altitude());
        _stream.writeStartElement("wpt");
        _stream.writeAttribute("lat", lat);
        _stream.writeAttribute("lon", lon);
        _stream.writeTextElement("name", QString::number(wp->id()));
        _stream.writeTextElement("elev", elev);
        _stream.writeTextElement("time", wp->_timeStamp.toString(Qt::ISODateWithMs));
        if(!wp->description().isEmpty()) {
            _stream.writeTextElement("desc", wp->description());
        }
        if(wp->_gpsCount) {
            QString h, v;
            h.sprintf("%f", wp->_gpsHdop);
            v.sprintf("%f", wp->_gpsVdop);
            _stream.writeTextElement("sat", QString::number(wp->_gpsCount));
            _stream.writeTextElement("hdop", h);
            _stream.writeTextElement("vdop", v);
        }
        _stream.writeEndElement();
    }
}

//----------------------------------------------------------------------------------------
void
GPXWriter::writeAndClose()
{
    _stream.writeEndElement();
    _stream.writeEndElement();
    _stream.writeEndDocument();
    QString path = qgcApp()->toolbox()->settingsManager()->appSettings()->missionSavePath();
    QString timeStamp = _start.toString("yyyyMMdd-hhmmss");
    QString filename = QString("%1/%2_wp.gpx").arg(path).arg(timeStamp);
    QFile trackFile(filename);
    if (trackFile.open(QFile::WriteOnly)) {
        trackFile.write(_output.toUtf8());
        trackFile.close();
    }
}
