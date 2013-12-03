#include "cutelystview.h"

CutelystView::CutelystView(QObject *parent) :
    QObject(parent)
{
}

bool CutelystView::process(Cutelyst *c)
{
    Q_UNUSED(c)
    qFatal("directly inherits from Catalyst::View. You need to\n"
           " inherit from a subclass like Cutelyst::View::ClearSilver instead.\n");
    return false;
}
