#!/bin/bash

CONF_MAJOR="1"
CONF_MINOR="0"
CONF_PATCH="0"

function assignVars()
{
    local  __resultvar=$1
    local  myresult=$2
    if [[ "$__resultvar" ]]; then
        eval $__resultvar="'$myresult'"
    else
        echo "$myresult"
    fi
}

function setMajor()
{
    assignVars $1 $CONF_MAJOR
}

function setMinor() {
    assignVars $1 $CONF_MINOR
}

function setPatch() {
    assignVars $1 $CONF_PATCH
}

function setRevision() {
    local branch="`git rev-parse --abbrev-ref HEAD`"
    local commit="`git rev-parse --short HEAD`"
    local dirty=$([ ! -z "`git diff-index --name-only HEAD`" ] && echo "*" || echo "")
    local info="[$commit@$branch$dirty]"

    assignVars $1 $info
}

setMajor MAJOR
setMinor MINOR
setPatch PATCH
setRevision REVISION

for i in "$@"
do
case $i in
    a)
    echo $MAJOR
    exit 0
    ;;
    b)
    echo $MINOR
    exit 0
    ;;
    c)
    echo $PATCH
    exit 0
    ;;
    d)
    echo $REVISION
    exit 0
    ;;
esac
done

VERSION=""
for j in $MAJOR $MINOR $PATCH $REVISION
do
    if [ ! -z "$j" ]
    then
        if [ ! -z "$VERSION" ]
        then
              VERSION="$VERSION."
        fi

        VERSION="$VERSION$j"
    fi
done

# 1.0.0.[xyz@master*] -> 1.0.0 [xyz@master*]
VERSION="${VERSION/.[/ [}"

echo $VERSION
