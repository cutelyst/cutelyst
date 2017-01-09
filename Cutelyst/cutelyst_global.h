#ifndef CUTELYST_GLOBAL_H
#define CUTELYST_GLOBAL_H

#include <QtCore/QtGlobal>

// defined by cmake when building this library
#if defined(cutelyst_qt5_EXPORTS)
#  define CUTELYST_LIBRARY Q_DECL_EXPORT
#else
#  define CUTELYST_LIBRARY Q_DECL_IMPORT
#endif
#if defined(plugin_action_renderview_EXPORTS)
#  define CUTELYST_PLUGIN_ACTION_RENDERVIEW_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_ACTION_RENDERVIEW_EXPORT Q_DECL_IMPORT
#endif
#if defined(plugin_action_rest_EXPORTS)
#  define CUTELYST_PLUGIN_ACTION_REST_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_ACTION_REST_EXPORT Q_DECL_IMPORT
#endif
#if defined(plugin_action_roleacl_EXPORTS)
#  define CUTELYST_PLUGIN_ACTION_ROLEACL_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_ACTION_ROLEACL_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_authentication_EXPORTS)
#  define CUTELYST_PLUGIN_AUTHENTICATION_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_AUTHENTICATION_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_session_EXPORTS)
#  define CUTELYST_PLUGIN_SESSION_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_SESSION_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_staticsimple_EXPORTS)
#  define CUTELYST_PLUGIN_STATICSIMPLE_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_STATICSIMPLE_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_utils_pagination_EXPORTS)
#  define CUTELYST_PLUGIN_UTILS_PAGINATION_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_UTILS_PAGINATION_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_statusmessage_EXPORTS)
#  define CUTELYST_PLUGIN_STATUSMESSAGE_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_STATUSMESSAGE_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_utils_sql_EXPORTS)
#  define CUTELYST_PLUGIN_UTILS_SQL_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_UTILS_SQL_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_utils_validator_EXPORTS)
#  define CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_view_clearsilver_EXPORTS)
#  define CUTELYST_VIEW_CLEARSILVER_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_VIEW_CLEARSILVER_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_view_email_EXPORTS)
#  define CUTELYST_VIEW_EMAIL_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_VIEW_EMAIL_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_view_grantlee_EXPORTS)
#  define CUTELYST_VIEW_GRANTLEE_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_VIEW_GRANTLEE_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_qt5_plugin_view_json_EXPORTS)
#  define CUTELYST_VIEW_JSON_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_VIEW_JSON_EXPORT Q_DECL_IMPORT
#endif
#if defined(cutelyst_wsgi_qt5_EXPORTS)
#  define CUTELYST_WSGI_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_WSGI_EXPORT Q_DECL_IMPORT
#endif

#endif // CUTELYST_GLOBAL_H

