include (tests_common.pri)

INCLUDEPATH += $$APP_PRO_ROOT\
    $$APP_PRO_ROOT/cli \
    $$APP_PRO_ROOT/settings \
    $$APP_PRO_ROOT/actions \
    $$APP_PRO_ROOT/geo

FORMS += \
    $$APP_PRO_ROOT/settings/actioneditor.ui \
    $$APP_PRO_ROOT/settings/actionstab.ui \
    $$APP_PRO_ROOT/settings/settingsdialog.ui \
    $$APP_PRO_ROOT/aboutdialog.ui

RESOURCES += \
    $$APP_PRO_ROOT/rsc.qrc

HEADERS += $$APP_PRO_ROOT\
    $$APP_PRO_ROOT/actions/actionresultviewer.h \
    $$APP_PRO_ROOT/actions/clicallresultview.h \
    $$APP_PRO_ROOT/actions/action.h \
    $$APP_PRO_ROOT/actions/actionjson.h \
    $$APP_PRO_ROOT/actions/actionstorage.h \
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
    $$APP_PRO_ROOT/settings/mapsettings.h \
    $$APP_PRO_ROOT/settings/iconlineedit.h \
    $$APP_PRO_ROOT/statechecker.h \
    $$APP_PRO_ROOT/nordvpninfo.h \
    $$APP_PRO_ROOT/trayicon.h \
    $$APP_PRO_ROOT/aboutdialog.h \
    $$APP_PRO_ROOT/version.h \
    $$APP_PRO_ROOT/geo/mapserversmodel.h \
    $$APP_PRO_ROOT/geo/mapwidget.h \
    $$APP_PRO_ROOT/geo/serverschartview.h \
    $$APP_PRO_ROOT/geo/serversfiltermodel.h \
    $$APP_PRO_ROOT/geo/serverslistmanager.h

SOURCES += \
    $$APP_PRO_ROOT/actions/actionresultviewer.cpp \
    $$APP_PRO_ROOT/actions/clicallresultview.cpp \
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
    $$APP_PRO_ROOT/settings/mapsettings.cpp \
    $$APP_PRO_ROOT/settings/iconlineedit.cpp \
    $$APP_PRO_ROOT/statechecker.cpp \
    $$APP_PRO_ROOT/nordvpninfo.cpp \
    $$APP_PRO_ROOT/trayicon.cpp \
    $$APP_PRO_ROOT/aboutdialog.cpp \
    $$APP_PRO_ROOT/version.cpp \
    $$APP_PRO_ROOT/geo/mapserversmodel.cpp \
    $$APP_PRO_ROOT/geo/mapwidget.cpp \
    $$APP_PRO_ROOT/geo/serverschartview.cpp \
    $$APP_PRO_ROOT/geo/serversfiltermodel.cpp \
    $$APP_PRO_ROOT/geo/serverslistmanager.cpp
