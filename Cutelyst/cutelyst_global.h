#ifndef CUTELYST_GLOBAL_H
#define CUTELYST_GLOBAL_H

#include <QtCore/QtGlobal>

#if defined(CUTELYST_LIBRARY)
#  define CUTELYST_LIBRARY Q_DECL_EXPORT
#else
#  define CUTELYST_LIBRARY Q_DECL_IMPORT
#endif

#endif // CUTELYST_GLOBAL_H

