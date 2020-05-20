include(tests_all_sources.pri)

TEMPLATE = app
TARGET = yangl_tests

SOURCES +=  \
    main.cpp \
    tst_action.cpp \
    tst_actionjson.cpp \
    tst_clicall.cpp \
    tst_clicaller.cpp

HEADERS += \
    tst_action.h \
    tst_actionjson.h \
    tst_clicall.h \
    tst_clicaller.h
