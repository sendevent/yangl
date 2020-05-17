QT       += core gui concurrent
TARGET = yangl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

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
    actions

SOURCES += \
    actions/action.cpp \
    actions/actionaccount.cpp \
    actions/actionconnect.cpp \
    actions/actiondisconnect.cpp \
    actions/actionsettings.cpp \
    actions/actionsfactory.cpp \
    actions/actionstatus.cpp \
    actions/actionstorage.cpp \
    cli/clibus.cpp \
    cli/clicall.cpp \
    main.cpp \
    nordvpnwraper.cpp \
    settings/appsettings.cpp \
    settings/settingsdialog.cpp \
    settings/settingsmanager.cpp \
    statechecker.cpp \
    trayicon.cpp

HEADERS += \
    actions/action.h \
    actions/actionaccount.h \
    actions/actionconnect.h \
    actions/actiondisconnect.h \
    actions/actionsettings.h \
    actions/actionsfactory.h \
    actions/actionstatus.h \
    actions/actionstorage.h \
    actions/actiontypes.h \
    cli/clibus.h \
    cli/clicall.h \
    nordvpnwraper.h \
    settings/appsettings.h \
    settings/settingsdialog.h \
    settings/settingsmanager.h \
    statechecker.h \
    trayicon.h

FORMS += \
    settings/settingsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc.qrc
