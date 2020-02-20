message("Adding Custom Plugin")

# Branding

DEFINES += CUSTOMHEADER=\"\\\"CustomPlugin.h\\\"\"
DEFINES += CUSTOMCLASS=CustomPlugin

TARGET   = QGCSurveyor
DEFINES += QGC_APPLICATION_NAME=\"\\\"QGCSurveyor\\\"\"

DEFINES += QGC_ORG_NAME=\"\\\"qgroundcontrol.org\\\"\"
DEFINES += QGC_ORG_DOMAIN=\"\\\"org.qgroundcontrol\\\"\"

QGC_CUSTOM_BUILD_FOLDER = custom

QGC_APP_NAME        = "QGCSurveyor"
QGC_BINARY_NAME     = "QGCSurveyor"
QGC_ORG_NAME        = "QGCSurveyor"
QGC_ORG_DOMAIN      = "org.qgroundcontrol"
QGC_APP_DESCRIPTION = "QGC Surveyor"
QGC_APP_COPYRIGHT   = "Copyright (C) 2019 QGroundControl Development Team. All rights reserved."

# Build a single flight stack by disabling APM support
MAVLINK_CONF = common
CONFIG  += QGC_DISABLE_APM_MAVLINK
CONFIG  += QGC_DISABLE_APM_PLUGIN QGC_DISABLE_APM_PLUGIN_FACTORY

# We implement our own PX4 plugin factory
CONFIG  += QGC_DISABLE_PX4_PLUGIN_FACTORY

#disable all the things we don't use
DEFINES += QGC_DISABLE_BLUETOOTH
DEFINES += QGC_DISABLE_NFC
DEFINES += QGC_DISABLE_QTNFC
DEFINES += QGC_DISABLE_UVC
DEFINES += QGC_DISABLE_PAIRING
CONFIG  += DISABLE_VIDEOSTREAMING

QT += \
    multimedia \

# Our own, custom resources
RESOURCES += \
    $$QGCROOT/$$QGC_CUSTOM_BUILD_FOLDER/custom.qrc

QML_IMPORT_PATH += \
    $$QGCROOT/$$QGC_CUSTOM_BUILD_FOLDER/res

# Our own, custom sources
SOURCES += \
    $$PWD/src/CustomPlugin.cc \
    $$PWD/src/CustomQuickInterface.cc \
    $$PWD/src/gpxwriter.cc \

HEADERS += \
    $$PWD/src/CustomPlugin.h \
    $$PWD/src/CustomQuickInterface.h \
    $$PWD/src/gpxwriter.h \

INCLUDEPATH += \
    $$PWD/src \

#-------------------------------------------------------------------------------------
# Custom Firmware/AutoPilot Plugin

INCLUDEPATH += \
    $$QGCROOT/custom/src/FirmwarePlugin \
    $$QGCROOT/custom/src/AutoPilotPlugin

HEADERS+= \
    $$QGCROOT/custom/src/AutoPilotPlugin/CustomAutoPilotPlugin.h \
    $$QGCROOT/custom/src/FirmwarePlugin/CustomFirmwarePlugin.h \
    $$QGCROOT/custom/src/FirmwarePlugin/CustomFirmwarePluginFactory.h \

SOURCES += \
    $$QGCROOT/custom/src/AutoPilotPlugin/CustomAutoPilotPlugin.cc \
    $$QGCROOT/custom/src/FirmwarePlugin/CustomFirmwarePlugin.cc \
    $$QGCROOT/custom/src/FirmwarePlugin/CustomFirmwarePluginFactory.cc \

