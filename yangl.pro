TEMPLATE = subdirs

CONFIG += c++14

SUBDIRS += \
    app

!CONFIG(no_tests) {
message("adding tests")
SUBDIRS += test_fake_status tests
tests.depends += test_fake_status
app.depends += tests
}
