QT += testlib core gui widgets svg concurrent
CONFIG += qt console warn_on depend_includepath testcase c++14
mac:CONFIG -= app_bundle

TEMPLATE = app

APP_PRO_ROOT=$$PWD/../app

INCLUDEPATH += $$APP_PRO_ROOT

#CONFIG(debug,debug|release): CONFIG+=autorun_app_tests

autorun_app_tests {
DESTDIR = ./
unix:QMAKE_POST_LINK=./$$TARGET
win32:QMAKE_POST_LINK=$${TARGET}.exe
}
