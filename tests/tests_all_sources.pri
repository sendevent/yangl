include (tests_common.pri)

INCLUDEPATH += $$APP_PRO_ROOT\
    $$APP_PRO_ROOT/cli \
    $$APP_PRO_ROOT/settings \
    $$APP_PRO_ROOT/actions

SOURCES += \
    $$APP_PRO_ROOT/actions/action.cpp \
    $$APP_PRO_ROOT/actions/actionjson.cpp \
    $$APP_PRO_ROOT/actions/actionstorage.cpp \
    $$APP_PRO_ROOT/cli/clicaller.cpp \
    $$APP_PRO_ROOT/cli/clicall.cpp \
    $$APP_PRO_ROOT/menuholder.cpp \
    $$APP_PRO_ROOT/nordvpnwraper.cpp \
    $$APP_PRO_ROOT/settings/actioneditor.cpp \
    $$APP_PRO_ROOT/settings/actionstab.cpp \
    $$APP_PRO_ROOT/settings/apppatheditor.cpp \
    $$APP_PRO_ROOT/settings/appsettings.cpp \
    $$APP_PRO_ROOT/settings/settingsdialog.cpp \
    $$APP_PRO_ROOT/settings/settingsmanager.cpp \
    $$APP_PRO_ROOT/statechecker.cpp \
    $$APP_PRO_ROOT/trayicon.cpp

HEADERS += $$APP_PRO_ROOT\
    $$APP_PRO_ROOT/actions/action.h \
    $$APP_PRO_ROOT/actions/actionjson.h \
    $$APP_PRO_ROOT/actions/actionstorage.h \
    $$APP_PRO_ROOT/actions/actiontypes.h \
    $$APP_PRO_ROOT/cli/clicaller.h \
    $$APP_PRO_ROOT/cli/clicall.h \
    $$APP_PRO_ROOT/menuholder.h \
    $$APP_PRO_ROOT/nordvpnwraper.h \
    $$APP_PRO_ROOT/settings/actioneditor.h \
    $$APP_PRO_ROOT/settings/actionstab.h \
    $$APP_PRO_ROOT/settings/apppatheditor.h \
    $$APP_PRO_ROOT/settings/appsettings.h \
    $$APP_PRO_ROOT/settings/settingsdialog.h \
    $$APP_PRO_ROOT/settings/settingsmanager.h \
    $$APP_PRO_ROOT/statechecker.h \
    $$APP_PRO_ROOT/trayicon.h

FORMS += \
    $$APP_PRO_ROOT/settings/actioneditor.ui \
    $$APP_PRO_ROOT/settings/actionstab.ui \
    $$APP_PRO_ROOT/settings/settingsdialog.ui

RESOURCES += \
    $$APP_PRO_ROOT/rsc.qrc
