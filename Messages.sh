#!/bin/sh

if [ ! -d i18n ]; then
    mkdir i18n
fi

if [ ! -x "$QT5LUPDATE" ]; then
    QT5LUPDATE=`which lupdate-qt5 2> /dev/null`; fi

if [ ! -x "$QT5LUPDATE" ]; then
    QT5LUPDATE=`which lupdate5 2> /dev/null`; fi

if [ ! -x "$QT5LUPDATE" ]; then
    QT5LUPDATE=`which lupdate 2> /dev/null`; fi

if [ ! -x "$QT5LUPDATE" ]; then
    echo "lupdate can not be found or is not executable."; echo "Use export QT5LUPDATE=/path/to/lupdate"; exit 1; fi

for DIR in cmd server Cutelyst Cutelyst/Plugins/Memcached Cutelyst/Plugins/CSRFProtection
do
    if [ ! -d ${DIR}/i18n ]; then
        mkdir ${DIR}/i18n
    fi
done

for LANG in en de
do
$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG cmd -ts cmd/i18n/cutelystcmd.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG server -ts server/i18n/cutelystserver.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG -no-recursive Cutelyst -ts Cutelyst/i18n/cutelystcore.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/Memcached -ts Cutelyst/Plugins/Memcached/i18n/plugin_memcached.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/CSRFProtection -ts Cutelyst/Plugins/CSRFProtection/i18n/plugin_csrfprotection.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/View/Cutelee -ts i18n/plugin_view_cutelee.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/View/Grantlee -ts i18n/plugin_view_grantlee.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/Utils/Validator -ts i18n/plugin_utils_validator.$LANG.ts
done
