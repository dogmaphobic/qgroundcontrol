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

#pragma once

#include <QObject>
#include <QXmlStreamWriter>
#include <QGeoCoordinate>
#include <QDateTime>

class VehicleGPSFactGroup;

//----------------------------------------------------------------------------------------
class GPXWayPoint : public QObject
{
    Q_OBJECT
    friend class GPXWriter;
public:
    explicit GPXWayPoint(QGeoCoordinate coordinate, int id, QObject *parent = nullptr);

    Q_PROPERTY(int              id              READ id                                        CONSTANT)
    Q_PROPERTY(QGeoCoordinate   coordinate      READ coordinate         WRITE setCoordinate    NOTIFY coordinateChanged)
    Q_PROPERTY(QString          description     READ description        WRITE setDescription   NOTIFY descriptionChanged)

    int             id              () { return _id; }
    QString         description     () { return _description; }
    void            setDescription  (QString des) { _description = des; emit descriptionChanged(); }

    QGeoCoordinate  coordinate      () { return _coordinate; }
    void            setCoordinate   (QGeoCoordinate cor) { _coordinate = cor; emit coordinateChanged(); }

    void            setGPSInfo      (VehicleGPSFactGroup* gps);

signals:
    void descriptionChanged     ();
    void coordinateChanged      ();

private:
    int             _id = 0;
    QString         _description;
    QGeoCoordinate  _coordinate;
    QDateTime       _timeStamp;
    int             _gpsCount       = 0;
    double          _gpsHdop        = 0.0;
    double          _gpsVdop        = 0.0;
};

//----------------------------------------------------------------------------------------
class GPXWriter : public QObject
{
    Q_OBJECT
public:
    explicit GPXWriter(QObject *parent = nullptr);

    void addWaypoint  (GPXWayPoint* wp);
    void writeAndClose();

private:
    QString             _output;
    QXmlStreamWriter    _stream;
    QDateTime           _start;
};

