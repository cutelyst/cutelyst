#ifndef UPLOAD_P_H
#define UPLOAD_P_H

#include "upload.h"

#include <QMultiHash>

namespace Cutelyst {

class UploadPrivate
{
public:
    UploadPrivate(QIODevice *dev);

    Headers headers;
    QString filename;
    QIODevice *device;
    qint64 startOffset = 0;
    qint64 endOffset = 0;
    qint64 pos = 0;
};

}

#endif // UPLOAD_P_H
