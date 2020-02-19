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
import QtQuick.Controls 2.4
import QtQuick.Layouts  1.11
import QtQuick.Window   2.11

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.Palette               1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Controllers           1.0

import Custom.Widgets                       1.0

Item {
    id:                                     toolBar
    anchors.fill:                           parent
    property string sectionTitle:           qsTr("Survey")
    property bool   inFlyView:              rootBackground.visible
    property color  menuSeparatorColor:     qgcPal.globalTheme === QGCPalette.Light ? Qt.rgba(0,0,0,0.25) : Qt.rgba(1,1,1,0.25)
    //-------------------------------------------------------------------------
    //-- Setup can be invoked from c++ side
    Connections {
        target: setupWindow
        onVisibleChanged: {
            if(setupWindow.visible) {
                vehicleSetup.checked = true
                sectionTitle = vehicleSetup.text
            }
        }
    }
    //-------------------------------------------------------------------------
    //-- Initial State
    Component.onCompleted: {
        flyButton.checked = true
        sectionTitle = flyButton.text
    }
    onInFlyViewChanged: {
        if(inFlyView) {
            flyButton.checked = true
            sectionTitle = flyButton.text
        }
    }
    Row {
        id:                                 iconRow
        height:                             parent.height
        anchors.left:                       parent.left
        spacing:                            ScreenTools.defaultFontPixelWidth * 2

        CustomIconButton {
            height:                         parent.height
            onPressed: {
                if(drawer.visible) {
                    drawer.close()
                } else {
                    drawer.open()
                }
                // Easter egg mechanism
                _pressCount++
                eggTimer.restart()
                if (_pressCount == 5) {
                    QGroundControl.corePlugin.showAdvancedUI = !QGroundControl.corePlugin.showAdvancedUI
                }
            }
            property int _pressCount: 0
            Timer {
                id:             eggTimer
                interval:       1000
                onTriggered:    parent._pressCount = 0
            }
        }
        Rectangle {
            width:                          1
            height:                         parent.height
            color:                          qgcPal.globalTheme === QGCPalette.Light ? Qt.rgba(0,0,0,0.15) : Qt.rgba(1,1,1,0.15)
        }
    }
    //-------------------------------------------------------------------------
    // Indicators
    Loader {
        source:                             "/custom/CustomMainToolBarIndicators.qml"
        anchors.left:                       iconRow.right
        anchors.leftMargin:                 ScreenTools.defaultFontPixelWidth * 2
        anchors.right:                      parent.right
        anchors.top:                        parent.top
        anchors.bottom:                     parent.bottom
    }
    //-------------------------------------------------------------------------
    // Parameter download progress bar
    Rectangle {
        anchors.bottom:                     parent.bottom
        height:                             ScreenTools.defaultFontPixelheight * 0.25
        width:                              activeVehicle ? activeVehicle.parameterManager.loadProgress * parent.width : 0
        color:                              qgcPal.colorGreen
    }
    //-------------------------------------------------------------------------
    // Bottom single pixel divider
    Rectangle {
        anchors.left:                       parent.left
        anchors.right:                      parent.right
        anchors.bottom:                     parent.bottom
        height:                             1
        color:                              menuSeparatorColor
    }
    //-------------------------------------------------------------------------
    //-- Navigation Drawer (Left to Right, on command or using touch gestures)
    Drawer {
        id:                                 drawer
        y:                                  header.height
        width:                              navButtonWidth
        height:                             mainWindow.height - header.height
        closePolicy:                        Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            color:                          qgcPal.window
        }
        ButtonGroup {
            id:                             buttonGroup
            buttons:                        buttons.children
        }
        ColumnLayout {
            id:                             buttons
            spacing:                        0
            anchors.top:                    parent.top
            anchors.left:                   parent.left
            anchors.right:                  parent.right
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                width:                      parent.width
                height:                     1
                color:                      menuSeparatorColor
            }
            CustomToolBarButton {
                id:                         flyButton
                spacing:                    1
                text:                       qsTr("Survey")
                icon.source:                "/qmlimages/PaperPlane.svg"
                Layout.fillWidth:           true
                onClicked: {
                    checked = true
                    drawer.close()
                    sectionTitle = text
                    mainWindow.showFlyView()
                }
            }
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                width:                      parent.width
                height:                     1
                color:                      menuSeparatorColor
            }
            CustomToolBarButton {
                text:                       qsTr("Analyze")
                icon.source:                "/qmlimages/Analyze.svg"
                Layout.fillWidth:           true
                onClicked: {
                    checked = true
                    drawer.close()
                    sectionTitle = text
                    mainWindow.showAnalyzeView()
                }
            }
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                width:                      parent.width
                height:                     1
                color:                      menuSeparatorColor
            }
            CustomToolBarButton {
                id:                         vehicleSetup
                text:                       qsTr("GPS Setup")
                icon.source:                "/qmlimages/Gears.svg"
                Layout.fillWidth:           true
                onClicked: {
                    checked = true
                    drawer.close()
                    sectionTitle = text
                    mainWindow.showSetupView()
                }
            }
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                width:                      parent.width
                height:                     1
                color:                      menuSeparatorColor
            }
        }
        ColumnLayout {
            id:                             lowerButtons
            anchors.bottom:                 parent.bottom
            anchors.left:                   parent.left
            anchors.right:                  parent.right
            spacing:                        0
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                Layout.fillWidth:           true
                height:                     1
                color:                      menuSeparatorColor
            }
            CustomToolBarButton {
                id:                         exitButton
                text:                       qsTr("Exit")
                icon.source:                "/res/land.svg"
                Layout.fillWidth:           true
                onClicked: {
                    drawer.close()
                    mainWindow.close()
                }
                visible: mainWindow.visibility === Window.FullScreen
            }
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                Layout.fillWidth:           true
                height:                     1
                color:                      menuSeparatorColor
                visible: exitButton.visible
            }
            Rectangle {
                Layout.alignment:           Qt.AlignVCenter
                Layout.fillWidth:           true
                height:                     1
                color:                      menuSeparatorColor
            }
            CustomToolBarButton {
                id:                         settingsButton
                text:                       qsTr("Settings")
                icon.source:                "/qmlimages/Gears.svg"
                Layout.fillWidth:           true
                onClicked: {
                    checked = true
                    buttonGroup.checkState = Qt.Unchecked
                    drawer.close()
                    sectionTitle = text
                    mainWindow.showSettingsView()
                }
            }
            Connections {
                target:                     buttonGroup
                onClicked:                  settingsButton.checked = false
            }
        }
    }
}
