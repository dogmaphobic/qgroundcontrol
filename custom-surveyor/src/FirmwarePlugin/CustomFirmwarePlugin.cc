/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 * @file
 *   @brief Custom Firmware Plugin (PX4)
 *   @author Gus Grubba <gus@auterion.com>
 *
 */

#include "CustomFirmwarePlugin.h"
#include "CustomAutoPilotPlugin.h"

//-----------------------------------------------------------------------------
CustomFirmwarePlugin::CustomFirmwarePlugin()
{
    for (int i = 0; i < _flightModeInfoList.count(); i++) {
        FlightModeInfo_t& info = _flightModeInfoList[i];
        //-- Narrow the options to only these two
        if (info.name != _altCtlFlightMode &&
            info.name != _posCtlFlightMode) {
            info.canBeSet = false;
        }
    }
}

//-----------------------------------------------------------------------------
AutoPilotPlugin* CustomFirmwarePlugin::autopilotPlugin(Vehicle* vehicle)
{
    return new CustomAutoPilotPlugin(vehicle, vehicle);
}


//-----------------------------------------------------------------------------
const QVariantList&
CustomFirmwarePlugin::toolBarIndicators(const Vehicle*)
{
    if(_toolBarIndicatorList.size() == 0) {
        _toolBarIndicatorList.append(QVariant::fromValue(QUrl::fromUserInput("qrc:/custom/CustomAltitudeIndicator.qml")));
        _toolBarIndicatorList.append(QVariant::fromValue(QUrl::fromUserInput("qrc:/custom/CustomGPSIndicator.qml")));
  }
    return _toolBarIndicatorList;
}
