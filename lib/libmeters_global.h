#ifndef LIBMETERS_GLOBAL_H
#define LIBMETERS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBMETERS_LIBRARY)
#  define LIBMETERSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBMETERSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBMETERS_GLOBAL_H
