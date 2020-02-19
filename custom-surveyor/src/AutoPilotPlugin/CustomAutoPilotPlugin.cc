/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 * @file
 *   @brief Custom Autopilot Plugin
 *   @author Gus Grubba <gus@auterion.com>
 */

#include "CustomAutoPilotPlugin.h"

#include "QGCApplication.h"
#include "QGCCorePlugin.h"

//-----------------------------------------------------------------------------
CustomAutoPilotPlugin::CustomAutoPilotPlugin(Vehicle* vehicle, QObject* parent)
    : PX4AutoPilotPlugin(vehicle, parent)
{
    connect(qgcApp()->toolbox()->corePlugin(), &QGCCorePlugin::showAdvancedUIChanged, this, &CustomAutoPilotPlugin::_advancedChanged);
}

//-----------------------------------------------------------------------------
void
CustomAutoPilotPlugin::_advancedChanged(bool)
{
    _components.clear();
    emit vehicleComponentsChanged();
}

//-----------------------------------------------------------------------------
const QVariantList&
CustomAutoPilotPlugin::vehicleComponents()
{
    if (_components.count() == 0 && !_incorrectParameterVersion) {
        if (_vehicle) {
            if (_vehicle->parameterManager()->parametersReady()) {
                _airframeComponent = new AirframeComponent(_vehicle, this);
                _airframeComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue(reinterpret_cast<VehicleComponent*>(_airframeComponent)));

                _sensorsComponent = new SensorsComponent(_vehicle, this);
                _sensorsComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue(reinterpret_cast<VehicleComponent*>(_sensorsComponent)));

                //-- Is there an ESP8266 Connected?
                if(_vehicle->parameterManager()->parameterExists(MAV_COMP_ID_UDP_BRIDGE, "SW_VER")) {
                    _esp8266Component = new ESP8266Component(_vehicle, this);
                    _esp8266Component->setupTriggerSignals();
                    _components.append(QVariant::fromValue(reinterpret_cast<VehicleComponent*>(_esp8266Component)));
                }
            } else {
                qWarning() << "Call to vehicleCompenents prior to parametersReady";
            }

            if(_vehicle->parameterManager()->parameterExists(_vehicle->id(), "SLNK_RADIO_CHAN")) {
                _syslinkComponent = new SyslinkComponent(_vehicle, this);
                _syslinkComponent->setupTriggerSignals();
                _components.append(QVariant::fromValue(reinterpret_cast<VehicleComponent*>(_syslinkComponent)));
            }
        } else {
            qWarning() << "Internal error";
        }
    }
    return _components;
}
