include(versiongen.pri)

QT += core gui concurrent qml quick quickwidgets location positioning
TARGET = yangl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    cli \
    settings \
    actions \
    geo

SOURCES += \
    aboutdialog.cpp \
    actions/action.cpp \
    actions/actionjson.cpp \
    actions/actionresultviewer.cpp \
    actions/actionstorage.cpp \
    actions/clicallresultview.cpp \
    cli/clicall.cpp \
    cli/clicaller.cpp \
    common.cpp \
    main.cpp \
    menuholder.cpp \
    nordvpninfo.cpp \
    nordvpnwraper.cpp \
    settings/actioneditor.cpp \
    settings/actionstab.cpp \
    settings/apppatheditor.cpp \
    settings/appsettings.cpp \
    settings/iconlineedit.cpp \
    settings/mapsettings.cpp \
    settings/settingsdialog.cpp \
    settings/settingsmanager.cpp \
    statechecker.cpp \
    trayicon.cpp \
    version.cpp \
    geo/mapserversmodel.cpp \
    geo/mapwidget.cpp \
    geo/serverschartview.cpp \
    geo/serversfiltermodel.cpp \
    geo/serverslistmanager.cpp

HEADERS += \
    aboutdialog.h \
    actions/action.h \
    actions/actionjson.h \
    actions/actionresultviewer.h \
    actions/actionstorage.h \
    actions/clicallresultview.h \
    cli/clicall.h \
    cli/clicaller.h \
    common.h \
    menuholder.h \
    nordvpninfo.h \
    nordvpnwraper.h \
    settings/actioneditor.h \
    settings/actionstab.h \
    settings/apppatheditor.h \
    settings/appsettings.h \
    settings/iconlineedit.h \
    settings/mapsettings.h \
    settings/settingsdialog.h \
    settings/settingsmanager.h \
    statechecker.h \
    trayicon.h \
    version.h \
    geo/mapserversmodel.h \
    geo/mapwidget.h \
    geo/serverschartview.h \
    geo/serversfiltermodel.h \
    geo/serverslistmanager.h

FORMS += \
    aboutdialog.ui \
    settings/actioneditor.ui \
    settings/actionstab.ui \
    settings/settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc.qrc
