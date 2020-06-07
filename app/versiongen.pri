shellScript=$$system_quote("$$_PRO_FILE_PWD_/../version.sh")
message($$system("$$shellScript"))

versionTarget.target = ../app/version.h
versionTarget.depends = FORCE
versionTarget.commands = $$shellScript
PRE_TARGETDEPS += ../app/version.h
QMAKE_EXTRA_TARGETS += versionTarget
