#!/bin/bash

CONF_MAJOR="1"
CONF_MINOR="0"
CONF_PATCH="0"
CONF_BUILDTIME=`date +%s`
CONF_BRANCH=""
CONF_HASH=""
CONF_DIRTY=""

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

function setDirtyMark()
{

echo $dirtyMark
    assignVars $1 $dirtyMark
}

function setRevision() {
    assignVars CONF_BRANCH $(git rev-parse --abbrev-ref HEAD)
    assignVars CONF_HASH $(git rev-parse --short HEAD)

    local dirtyFiles=$(git diff-index --name-only HEAD)
    if [[ ! -z "$dirtyFiles" ]]; then
        assignVars CONF_DIRTY "*"
    fi

    local info="[$CONF_HASH@$CONF_BRANCH$CONF_DIRTY]"

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
    e)
    echo $CONF_BUILDTIME
    exit 0
    ;;
esac
done

VERSION=$(printf '%s.%s.%s %s' "$MAJOR" "$MINOR" "$PATCH" "$REVISION") # 1.0.0 [xyz@master*]
VERSION_SNAP=$(printf '%s.%s.%s-%s-%s%s' "$MAJOR" "$MINOR" "$PATCH" "$CONF_BRANCH" "$CONF_HASH" "$CONF_DIRTY") # 1.0.0-master-xyz*]
VERSION_SNAP="${VERSION_SNAP/\*/\+}" # 1.0.0.[xyz@master*] -> 1.0.0-master-xyz*] -> # 1.0.0-master-xyz+]

# for dev local branch:
VERSION_SNAP="${VERSION_SNAP/_/-}" # 1.0.0.[xyz@dengof_snap] -> 1.0.0.[xyz@dengof-snap]
VERSION_SNAP="${VERSION_SNAP/\#/N}" # 1.0.0.[xyz@issue_#123] -> 1.0.0.[xyz@issue-N123]

metaFile=yangl.metainfo.xml
#sed -E -e "0,/(<release.*\/>)/ s//\1\n<release date=\"`date -d @$CONF_BUILDTIME +%Y-%m-%d`\" version=\"$VERSION_SNAP\" \/>/" ./$metaFile.in > ./$metaFile
sed -E -e "0,/(replaceme)/ s//\n<release date=\"`date -d @$CONF_BUILDTIME +%Y-%m-%d`\" version=\"$VERSION_SNAP\" \/>/" ./$metaFile.in > ./$metaFile

echo $VERSION
