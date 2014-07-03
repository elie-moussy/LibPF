#ifndef LIBPF_GLOBAL_H
#define LIBPF_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBPF_LIBRARY)
#  define LIBPFSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBPFSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBPF_GLOBAL_H
