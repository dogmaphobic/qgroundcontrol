/****************************************************************************
 *
 * (c) 2009-2019 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 * @file
 *   @author Gus Grubba <gus@auterion.com>
 */

import QtQuick                  2.11
import QtQuick.Controls         2.4
import QtQuick.Layouts          1.11
import QtQuick.Dialogs          1.3
import QtPositioning            5.2
import QtLocation               5.12

import QGroundControl                       1.0
import QGroundControl.Controllers           1.0
import QGroundControl.Controls              1.0
import QGroundControl.FactSystem            1.0
import QGroundControl.FlightMap             1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.Palette               1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Vehicle               1.0
import QGroundControl.QGCPositionManager    1.0

import CustomQuickInterface                 1.0
import Custom.Widgets                       1.0

Item {
    anchors.fill:                           parent

    readonly property real _heading:            activeVehicle   ? activeVehicle.heading.rawValue : 0
    readonly property real _indicatorsHeight:   ScreenTools.defaultFontPixelHeight
    readonly property bool _rtkRelay:           QGroundControl.gpsRtk.active.value && QGroundControl.settingsManager.rtkSettings.forwardRTCM.value
    readonly property bool _ready:              activeVehicle && activeVehicle.gps.lock.value > 2
    readonly property real _separator:          ScreenTools.defaultFontPixelHeight * 0.5

    readonly property Fact currentLatitude:     _rtkRelay ? QGroundControl.gpsRtk.currentLatitude  : null
    readonly property Fact currentLongitude:    _rtkRelay ? QGroundControl.gpsRtk.currentLongitude : null
    readonly property Fact currentAltitude:     _rtkRelay ? QGroundControl.gpsRtk.currentAltitude  : null

    function setMapCenter() {
        mainWindow.flightDisplayMap.center = QtPositioning.coordinate(currentLatitude.value, currentLongitude.value, currentAltitude.value)
    }

    Connections {
        target: currentLatitude
        onValueChanged: setMapCenter
    }

    Connections {
        target: currentLongitude
        onValueChanged: setMapCenter
    }

    Connections {
        target: currentAltitude
        onValueChanged: setMapCenter
    }

    Connections {
        target: mainWindow
        onFlightDisplayMapChanged: {
            if(mainWindow.flightDisplayMap) {
                mainWindow.flightDisplayMap.addMapItemView(waypointView)
            }
        }
    }

    MapItemView {
        id:         waypointView
        model:      CustomQuickInterface.waypoints
        delegate:   MapQuickItem {
            anchorPoint.x:  sourceItem.width  / 2
            anchorPoint.y:  sourceItem.height / 2
            coordinate:     object.coordinate
            sourceItem: Rectangle {
                width:      _radius * 2
                height:     _radius * 2
                radius:     _radius
                color:      Qt.rgba(0.2,0,0,0.1)
                border.color: Qt.rgba(0,0,0,0.75)
                border.width: 1
                readonly property real _radius: ScreenTools.defaultFontPixelHeight * 0.75
                QGCLabel {
                    anchors.centerIn:   parent
                    color:              "white"
                    text:               object.id
                }
                MouseArea {
                    anchors.fill:   parent
                    onClicked: {
                        waypointEditor.wpObject = object
                        waypointEditor.open()
                    }
                }
            }
        }
    }

    //---------------------------------------------------------------
    //-- Waypoint Editor
    Popup {
        id:                 waypointEditor
        property var wpObject: null
        width:              wpCol.width  + (ScreenTools.defaultFontPixelWidth  * 4)
        height:             wpCol.height + (ScreenTools.defaultFontPixelHeight * 4)
        modal:              true
        focus:              true
        parent:             Overlay.overlay
        closePolicy:        Popup.CloseOnEscape | Popup.CloseOnPressOutside
        x:                  Math.round((mainWindow.width  - width)  * 0.5)
        y:                  Math.round((mainWindow.height - height) * 0.5)
        background: Rectangle {
            anchors.fill:   parent
            color:          Qt.rgba(0,0,0,0.75)
            radius:         ScreenTools.defaultFontPixelWidth
        }
        onVisibleChanged: {
            if(!visible && waypointEditor.wpObject) {
                waypointEditor.wpObject.description = waypointDescription.text
            }
        }
        Column {
            id:                 wpCol
            spacing:            ScreenTools.defaultFontPixelHeight * 0.5
            anchors.centerIn:   parent
            QGCLabel {
                text:       qsTr("Waypoint " + (waypointEditor.wpObject ? waypointEditor.wpObject.id : "N/A"))
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Item { width: 1; height: 1; }
            GridLayout {
                anchors.margins:    ScreenTools.defaultFontPixelHeight
                columnSpacing:      ScreenTools.defaultFontPixelWidth
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 2
                QGCLabel {
                    text:       qsTr("LAT:")
                }
                QGCLabel {
                    text:       waypointEditor.wpObject ? waypointEditor.wpObject.coordinate.latitude.toFixed(6) : ""
                }
                QGCLabel {
                    text:       qsTr("LON:")
                }
                QGCLabel {
                    text:       waypointEditor.wpObject ? waypointEditor.wpObject.coordinate.longitude.toFixed(6) : ""
                }
                QGCLabel {
                    text:       qsTr("ALT:")
                }
                QGCLabel {
                    text:       waypointEditor.wpObject ? waypointEditor.wpObject.coordinate.altitude.toFixed(2) : ""
                }
            }
            Item { width: 1; height: 1; }
            QGCLabel {
                text:               qsTr("Description")
            }
            CustomTextArea {
                id:                 waypointDescription
                width:              ScreenTools.defaultFontPixelWidth  * 30
                height:             ScreenTools.defaultFontPixelHeight * 4
                font.pointSize:     ScreenTools.defaultFontPointSize
                text:               waypointEditor.wpObject ? waypointEditor.wpObject.description : ""
            }
        }
    }

    //-------------------------------------------------------------------------
    //-- Heading Indicator
    Rectangle {
        id:             compassBar
        height:         ScreenTools.defaultFontPixelHeight * 1.5
        width:          ScreenTools.defaultFontPixelWidth  * 50
        color:          "#DEDEDE"
        radius:         2
        clip:           true
        visible:        !_rtkRelay
        anchors.top:    parent.top
        anchors.topMargin:          ScreenTools.defaultFontPixelHeight * 2
        anchors.horizontalCenter:   parent.horizontalCenter
        Repeater {
            model: 720
            QGCLabel {
                function _normalize(degrees) {
                    var a = degrees % 360
                    if (a < 0) a += 360
                    return a
                }
                property int _startAngle: modelData + 180 + _heading
                property int _angle: _normalize(_startAngle)
                anchors.verticalCenter: parent.verticalCenter
                x:              visible ? ((modelData * (compassBar.width / 360)) - (width * 0.5)) : 0
                visible:        _angle % 45 == 0
                color:          "#75505565"
                font.pointSize: ScreenTools.smallFontPointSize
            text: {
                    switch(_angle) {
                    case 0:     return "N"
                    case 45:    return "NE"
                    case 90:    return "E"
                    case 135:   return "SE"
                    case 180:   return "S"
                    case 225:   return "SW"
                    case 270:   return "W"
                    case 315:   return "NW"
                    }
                    return ""
                }
            }
        }
    }
    Rectangle {
        id:                         headingIndicator
        height:                     ScreenTools.defaultFontPixelHeight
        width:                      ScreenTools.defaultFontPixelWidth * 4
        color:                      qgcPal.windowShadeDark
        visible:                    !_rtkRelay
        anchors.bottom:             compassBar.top
        anchors.bottomMargin:       ScreenTools.defaultFontPixelHeight * -0.1
        anchors.horizontalCenter:   parent.horizontalCenter
        QGCLabel {
            text:                   _heading
            color:                  qgcPal.text
            font.pointSize:         ScreenTools.smallFontPointSize
            anchors.centerIn:       parent
        }
    }
    Image {
        height:                     _indicatorsHeight
        width:                      height
        source:                     "/custom/img/compass_pointer.svg"
        visible:                    !_rtkRelay
        fillMode:                   Image.PreserveAspectFit
        sourceSize.height:          height
        anchors.top:                compassBar.bottom
        anchors.topMargin:          ScreenTools.defaultFontPixelHeight * -0.5
        anchors.horizontalCenter:   parent.horizontalCenter
    }

    //-------------------------------------------------------------------------
    //-- Actions
    Rectangle {
        height:                         trackCol.height + (ScreenTools.defaultFontPixelHeight * 2)
        width:                          trackCol.width  + (ScreenTools.defaultFontPixelWidth  * 2)
        color:                          Qt.rgba(0,0,0,0.5)
        radius:                         6
        clip:                           true
        visible:                        !_rtkRelay
        anchors.verticalCenter:         parent.verticalCenter
        anchors.right:                  parent.right
        anchors.rightMargin:            ScreenTools.defaultFontPixelWidth
        Column {
            id:                         trackCol
            spacing:                    ScreenTools.defaultFontPixelHeight
            anchors.centerIn:           parent
            //-- Tracking
            QGCLabel {
                text:                   "Track"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle {
                color:                  Qt.rgba(0,0,0,0)
                width:                  height
                height:                 ScreenTools.defaultFontPixelHeight * 4
                radius:                 width * 0.5
                border.color:           qgcPal.buttonText
                border.width:           1
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle {
                    width:              parent.width * 0.75
                    height:             width
                    radius:             width * 0.5
                    color:              _ready ? qgcPal.colorGreen : qgcPal.text
                    visible:            !pauseTracking.visible
                    opacity:            _ready ? 1 : 0.5
                    anchors.centerIn:   parent
                }
                Rectangle {
                    id:                 pauseTracking
                    width:              parent.width * 0.5
                    height:             width
                    color:              qgcPal.colorRed
                    visible:            CustomQuickInterface.tracking
                    anchors.centerIn:   parent
                }
                MouseArea {
                    anchors.fill:   parent
                    enabled:        activeVehicle
                    onClicked: {
                        if(_ready || CustomQuickInterface.tracking) {
                            CustomQuickInterface.tracking = !CustomQuickInterface.tracking
                        }
                    }
                }
            }
            Item { width: 1; height: _separator; }
            //-- Waypoint
            QGCLabel {
                text:                   "Waypoint"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle {
                color:                  Qt.rgba(0,0,0,0)
                width:                  height
                height:                 ScreenTools.defaultFontPixelHeight * 4
                radius:                 width * 0.5
                border.color:           qgcPal.buttonText
                border.width:           1
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle {
                    width:              parent.width * 0.75
                    height:             width
                    radius:             width * 0.5
                    color:              _ready ? qgcPal.colorGreen : qgcPal.text
                    opacity:            _ready ? 1 : 0.5
                    anchors.centerIn:   parent
                }
                MouseArea {
                    anchors.fill:   parent
                    enabled:        activeVehicle
                    onClicked: {
                        if(_ready) {
                            CustomQuickInterface.addWaypoint()
                        }
                    }
                }
            }
            Item { width: 1; height: _separator; }
            //-- Waypoint
            QGCLabel {
                text:                   "Reset"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Rectangle {
                color:                  Qt.rgba(0,0,0,0)
                width:                  height
                height:                 ScreenTools.defaultFontPixelHeight * 4
                radius:                 width * 0.5
                border.color:           qgcPal.buttonText
                border.width:           1
                anchors.horizontalCenter: parent.horizontalCenter
                Rectangle {
                    width:              parent.width * 0.75
                    height:             width
                    radius:             width * 0.5
                    color:              qgcPal.colorRed
                    anchors.centerIn:   parent
                }
                MessageDialog {
                    id:                 resetDialog
                    visible:            false
                    icon:               StandardIcon.Warning
                    standardButtons:    StandardButton.Yes | StandardButton.No
                    title:              qsTr("Reset Waypoints")
                    text:               qsTr("Save and reset all waypoints?")
                    onYes: {
                        CustomQuickInterface.reset()
                        resetDialog.close()
                    }
                    onNo: {
                        resetDialog.close()
                    }
                }
                MouseArea {
                    anchors.fill:   parent
                    enabled:        activeVehicle
                    onClicked: {
                        if(activeVehicle) {
                            resetDialog.open()
                        }
                    }
                }
            }
        }
    }
}
