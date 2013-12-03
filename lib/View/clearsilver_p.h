#ifndef CLEARSILVER_P_H
#define CLEARSILVER_P_H

#include "clearsilver.h"

#include <ClearSilver/ClearSilver.h>

class ClearSilverPrivate
{
public:
    HDF *hdfForStash(Cutelyst *ctx, const QVariantHash &stash);
    bool render(Cutelyst *ctx, const QString &filename, const QVariantHash &stash, QByteArray &output);
    void renderError(Cutelyst *ctx, const QString &error);

    QString includePath;
    QString extension;
    QString wrapper;
};

#endif // CLEARSILVER_P_H
