
TEMPLATE = app
TARGET = yangl_tests

SOURCES +=  \
    main.cpp \
    tst_action.cpp \
    tst_actionjson.cpp \
    tst_actionstorage.cpp \
    tst_clicall.cpp \
    tst_clicaller.cpp \
    tst_statechecker.cpp

HEADERS += \
    tst_action.h \
    tst_actionjson.h \
    tst_actionstorage.h \
    tst_clicall.h \
    tst_clicaller.h \
    tst_statechecker.h

include(tests_all_sources.pri)
