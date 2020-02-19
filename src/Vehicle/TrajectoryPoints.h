/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QmlObjectListModel.h"

#include <QGeoCoordinate>

class Vehicle;

class TrajectoryPoints : public QObject
{
    Q_OBJECT

public:
    TrajectoryPoints(Vehicle* vehicle, QObject* parent = nullptr);

    Q_INVOKABLE QVariantList list(void) const { return _points; }

    /// Resets any existing trajectory and starts collecting a new one
    void start  (void);
    /// Stops collecting coordinate changes
    void stop   (void);
    /// Pause collecting coordinate changes.
    void pause  ();
    /// Resumes collecting coordinate changes. If not running, it will start from scratch.
    void resume ();

public slots:
    void clear  (void);

signals:
    void pointAdded     (QGeoCoordinate coordinate);
    void updateLastPoint(QGeoCoordinate coordinate);
    void pointsCleared  (void);

private slots:
    void _vehicleCoordinateChanged(QGeoCoordinate coordinate);

private:
    Vehicle*        _vehicle;
    QVariantList    _points;
    QGeoCoordinate  _lastPoint;
    double          _lastAzimuth;
    bool            _paused  = false;
    bool            _running = false;

    static constexpr double _distanceTolerance = 2.0;
    static constexpr double _azimuthTolerance  = 1.5;
};
