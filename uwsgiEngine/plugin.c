#include <uwsgi.h>

#include "plugin.h"

int uwsgi_cutelyst_init(void);
int uwsgi_cutelyst_request(struct wsgi_request *);
void uwsgi_cutelyst_init_apps(void);

struct uwsgi_option uwsgi_cutelyst_options[] = {

    {"cutelyst-app", required_argument, 'A', "loads the Cutelyst Application", uwsgi_opt_set_str, &options.app, 0},
    {"cutelyst-config", required_argument, 'C', "sets the application config", uwsgi_opt_set_str, &options.config, 0},
    {0, 0, 0, 0, 0, 0, 0},

};

struct uwsgi_plugin libcutelyst_uwsgi_plugin = {

    .name = "cutelyst",
    .modifier1 = 0,
    .init = uwsgi_cutelyst_init,
    .request = uwsgi_cutelyst_request,
    .init_apps = uwsgi_cutelyst_init_apps,
    .options = uwsgi_cutelyst_options,

};
