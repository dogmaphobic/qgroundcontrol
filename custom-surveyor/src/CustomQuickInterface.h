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

#include "Vehicle.h"

#include <QObject>
#include <QTimer>
#include <QColor>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QSoundEffect>

//-----------------------------------------------------------------------------
// QtQuick Interface (UI)
class CustomQuickInterface : public QObject
{
    Q_OBJECT
public:
    CustomQuickInterface(QObject* parent = nullptr);
    ~CustomQuickInterface();

    Q_PROPERTY(double               altitudeRelative            READ altitudeRelative                                   NOTIFY altitudeRelativeChanged)
    Q_PROPERTY(double               altitudeAMSL                READ altitudeAMSL                                       NOTIFY altitudeAMSLChanged)
    Q_PROPERTY(bool                 ready                       READ ready                                              NOTIFY readyChanged)
    Q_PROPERTY(bool                 tracking                    READ tracking               WRITE  setTracking          NOTIFY trackingChanged)
    Q_PROPERTY(int                  points                      READ points                                             NOTIFY pointsChanged)

    Q_INVOKABLE void addWaypoint    ();
    Q_INVOKABLE void reset          ();

    void        init                ();

    double      altitudeRelative    () { return _altitudeRelative; }
    double      altitudeAMSL        () { return _altitudeAMSL; }
    bool        tracking            () { return _tracking; }
    bool        ready               () { return _ready; }
    int         points              () { return _points; }

    void        setTracking         (bool set);

signals:
    void altitudeRelativeChanged    ();
    void altitudeAMSLChanged        ();
    void trackingChanged            ();
    void readyChanged               ();
    void pointsChanged              ();

private slots:
    void _setActiveVehicle          (Vehicle* vehicle);
    void _altitudeRelativeChanged   (QVariant value);
    void _altitudeAMSLChanged       (QVariant value);
    void _trackingTime              ();
    void _vehicleCoordinateChanged  (QGeoCoordinate coordinate);
    void _rtkAccuracyChanged        (QVariant value);

private:
    void _closeFile                 ();
    void _addWaypoint               (bool wp, QGeoCoordinate coordinate, double altitude);

private:
    Vehicle*        _vehicle            = nullptr;
    double          _altitudeRelative   = 0;
    double          _altitudeAMSL       = 0;
    bool            _tracking           = false;
    bool            _ready              = false;
    int             _points             = 0;
    QTimer          _trackTimer;
    QGeoCoordinate  _lastPoint;
    double          _lastAltitude;
    QSoundEffect    _startStopSound;
    QSoundEffect    _activeSound;
    QSoundEffect    _captureSound;
    QFile           _trackFile;
};
