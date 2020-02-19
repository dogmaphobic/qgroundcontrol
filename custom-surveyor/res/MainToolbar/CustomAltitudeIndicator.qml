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

import QtQuick          2.11
import QtQuick.Controls 1.4
import QtQuick.Layouts  1.11

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0

import CustomQuickInterface                 1.0

//-------------------------------------------------------------------------
//-- Altitude Indicator
Item {
    id:                     _root
    width:                  altRow.width
    anchors.top:            parent.top
    anchors.bottom:         parent.bottom
    RowLayout {
        id:             altRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth * 0.25
        QGCLabel {
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.mediumFontPointSize
            color:                  qgcPal.text
            text:                   "POINTS:"
        }
        QGCLabel {
            Layout.minimumWidth:    ScreenTools.defaultFontPixelWidth * 8
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.largeFontPointSize
            color:                  qgcPal.text
            text:                   CustomQuickInterface.points
        }
        Item { width: ScreenTools.defaultFontPixelWidth * 2; height: 1; }
        QGCLabel {
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.mediumFontPointSize
            color:                  qgcPal.text
            text:                   "REL:"
        }
        QGCLabel {
            Layout.minimumWidth:    ScreenTools.defaultFontPixelWidth * 8
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.largeFontPointSize
            color:                  qgcPal.text
            text:                   CustomQuickInterface.altitudeRelative.toFixed(2) + (activeVehicle ? activeVehicle.altitudeRelative.units : "")
        }
        Item { width: ScreenTools.defaultFontPixelWidth * 2; height: 1; }
        QGCLabel {
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.mediumFontPointSize
            color:                  qgcPal.text
            text:                   "AMSL:"
        }
        QGCLabel {
            Layout.minimumWidth:    ScreenTools.defaultFontPixelWidth * 8
            Layout.alignment:       Qt.AlignVCenter
            font.pointSize:         ScreenTools.largeFontPointSize
            color:                  qgcPal.text
            text:                   CustomQuickInterface.altitudeAMSL.toFixed(2) + (activeVehicle ? activeVehicle.altitudeAMSL.units : "")
        }
    }
}
