#ifndef CQUERYMULTIMAP_H
#define CQUERYMULTIMAP_H

#include <QMetaType>
#include <QMultiMap>
#include <QString>

typedef QMap<QString, QString> CQueryMultiMap;

Q_DECLARE_METATYPE(CQueryMultiMap)

#endif // CQUERYMULTIMAP_H
