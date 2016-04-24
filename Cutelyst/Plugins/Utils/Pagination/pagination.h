#ifndef PAGINATION_H
#define PAGINATION_H

#include <Cutelyst/context.h>

namespace Cutelyst {

namespace Utils {

namespace Pagination CUTELYST_LIBRARY {

QVariantMap page(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks = 10);

}

}

}

#endif // PAGINATION_H
