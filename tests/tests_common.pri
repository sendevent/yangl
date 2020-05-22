QT += testlib core gui widgets concurrent
CONFIG += qt console warn_on depend_includepath testcase C++14
mac:CONFIG -= app_bundle

TEMPLATE = app

APP_PRO_ROOT=$$PWD/../app

INCLUDEPATH += $$APP_PRO_ROOT

CONFIG(debug,debug|release): CONFIG+=autorun_app_tests
autorun_app_tests {
unix {
    PRE_TARGETDEPS=$$OUT_PWD/../app/yangl
    QMAKE_POST_LINK=$$OUT_PWD/$$TARGET
}
    win32:QMAKE_POST_LINK=$${TARGET}.exe
}
