/****************************************************************************
 *
 * (c) 2009-2019 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 *   @brief Custom QGCCorePlugin Declaration
 *   @author Gus Grubba <gus@auterion.com>
 */

#pragma once

#include "QGCCorePlugin.h"
#include "QGCOptions.h"
#include "QGCLoggingCategory.h"

class CustomPlugin;
class CustomSettings;

Q_DECLARE_LOGGING_CATEGORY(CustomLog)

//-----------------------------------------------------------------------------
//-- Our own, custom options
class CustomOptions : public QGCOptions
{
public:
    CustomOptions(CustomPlugin*, QObject* parent = nullptr);
    bool        wifiReliableForCalibration      () const final { return true; }
#if defined(Q_OS_LINUX)
    double      toolbarHeightMultiplier         () final { return 1.25; }
#endif
    QUrl        flyViewOverlay                  () const final { return QUrl::fromUserInput("qrc:/custom/CustomFlyView.qml"); }
    //-- We have our own toolbar
    QUrl        mainToolbarUrl                  () const final { return QUrl::fromUserInput("qrc:/custom/CustomMainToolBar.qml"); }
    QUrl        planToolbarUrl                  () const final { return QUrl::fromUserInput("qrc:/custom/CustomMainToolBar.qml"); }
    QUrl        flightDisplayViewWidgetstUrl    () const final { return QUrl(); }
    //-- Don't show instrument widget
    CustomInstrumentWidget* instrumentWidget    () final { return nullptr; }

    //-- We handle multiple vehicles in a custom way
    bool        enableMultiVehicleList          () const final { return false; }
    //-- We don't use the toolstrip
    bool        showToolstrip                   () const final { return false; }
};


//-----------------------------------------------------------------------------
class CustomPlugin : public QGCCorePlugin
{
    Q_OBJECT

public:
    CustomPlugin(QGCApplication* app, QGCToolbox *toolbox);
    ~CustomPlugin();

    // Overrides from QGCCorePlugin
    QVariantList&           settingsPages                   () final;
    QGCOptions*             options                         () final;
    QString                 brandImageIndoor                () const final;
    QString                 brandImageOutdoor               () const final;
    bool                    overrideSettingsGroupVisibility (QString name) final;
    QQmlApplicationEngine*  createRootWindow                (QObject* parent) final;
    bool                    adjustSettingMetaData           (const QString& settingsGroup, FactMetaData& metaData) final;
    void                    paletteOverride                 (QString colorName, QGCPalette::PaletteColorInfo_t& colorInfo) final;
    // Overrides from QGCTool
    void                    setToolbox                      (QGCToolbox* toolbox);

    const static QColor     _windowShadeEnabledLightColor;
    const static QColor     _windowShadeEnabledDarkColor;

private slots:
    void                    _advancedChanged                (bool advanced);

private:
    void
    addSettingsEntry(
        const QString& title,
        const char* qmlFile,
        const char* iconFile = nullptr);

private:
    CustomOptions*       _pOptions = nullptr;
    QVariantList         _customSettingsList; // Not to be mixed up with QGCCorePlugin implementation
};
