#ifndef CUTELYST_GLOBAL_H
#define CUTELYST_GLOBAL_H

#include <QtCore/QtGlobal>

// defined by cmake when building this library
#if defined(cutelyst_qt5_EXPORTS)
#  define CUTELYST_LIBRARY Q_DECL_EXPORT
#else
#  define CUTELYST_LIBRARY Q_DECL_IMPORT
#endif

#endif // CUTELYST_GLOBAL_H

