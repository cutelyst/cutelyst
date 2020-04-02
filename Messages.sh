#!/bin/sh

if [ ! -d i18n ]; then
    mkdir i18n
fi

if [ ! $QT5LUPDATE ]; then
    QT5LUPDATE=/usr/bin/lupdate; fi

if [ ! -x "$QT5LUPDATE" ]; then
    echo "${QT5LUPDATE} can not be found or is not executable."; echo "Use export QT5LUPDATE=/path/to/lupdate"; exit 1; fi

for LANG in en de
do
$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG cmd -ts i18n/cutelystcmd.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG wsgi -ts i18n/cutelystwsgi.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG -no-recursive Cutelyst -ts i18n/cutelystcore.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/Memcached -ts i18n/plugin_memcached.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/CSRFProtection -ts i18n/plugin_csrfprotection.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/View/Cutelee -ts i18n/plugin_view_cutelee.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/View/Grantlee -ts i18n/plugin_view_grantlee.$LANG.ts

$QT5LUPDATE -no-obsolete -locations none -source-language en -target-language $LANG Cutelyst/Plugins/Utils/Validator -ts i18n/plugin_utils_validator.$LANG.ts
done
