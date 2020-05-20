TEMPLATE = subdirs

SUBDIRS += \
    app \
    test_fake_status \
    tests

tests.depends=test_fake_status
