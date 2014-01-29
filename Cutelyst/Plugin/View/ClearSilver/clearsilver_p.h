#ifndef CLEARSILVER_P_H
#define CLEARSILVER_P_H

#include "clearsilver.h"

#include <ClearSilver/ClearSilver.h>

namespace Cutelyst {

class ClearSilverPrivate
{
public:
    HDF *hdfForStash(Context *ctx, const QVariantHash &stash);
    void serializeHash(HDF *hdf, const QVariantHash &hash, const QString &prefix = QString());
    void serializeMap(HDF *hdf, const QVariantMap &map, const QString &prefix = QString());
    void serializeVariant(HDF *hdf, const QVariant &value, const QString &key);
    bool render(Context *ctx, const QString &filename, const QVariantHash &stash, QByteArray &output);
    void renderError(Context *ctx, const QString &error);

    QString includePath;
    QString extension;
    QString wrapper;
};

}

#endif // CLEARSILVER_P_H
