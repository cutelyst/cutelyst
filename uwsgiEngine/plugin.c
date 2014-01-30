#include <uwsgi.h>


int uwsgi_cplusplus_init(void);
int uwsgi_cplusplus_request(struct wsgi_request *);
void uwsgi_cplusplus_after_request(struct wsgi_request *);
void uwsgi_cutelyst_init_apps(void);

struct uwsgi_plugin libcutelyst_uwsgi_plugin = {

    .name = "cutelyst",
    .modifier1 = 0,
    .init = uwsgi_cplusplus_init,
    .request = uwsgi_cplusplus_request,
    .after_request = uwsgi_cplusplus_after_request,

};
