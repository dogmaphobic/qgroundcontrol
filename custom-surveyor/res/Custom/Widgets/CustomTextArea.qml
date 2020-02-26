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

import QtQuick                      2.3
import QtQuick.Controls             1.2
import QtQuick.Controls.Styles      1.4

import QGroundControl.Controls      1.0
import QGroundControl.Palette       1.0
import QGroundControl.ScreenTools   1.0

TextArea {
    font.pointSize:         ScreenTools.defaultFontPointSize
    style: TextAreaStyle {
        textColor:          qgcPal.windowShade
        backgroundColor:    qgcPal.text
    }
}
