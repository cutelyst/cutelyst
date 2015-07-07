# Cutelyst
A Web Framework built on top of Qt, using the simple approach of Catalyst (Perl) framework.

Qt's meta object system is what powers the core of Cutelyst, this allows for introspecting controllers methods signatures and generate matching actions that can be invokable later. Some features:

 * Multiple protocols support when using uWSGI engine
  * HTTP 1.1
  * FastCGI
  * uWSGI
 * REST with ActionREST
 * Plugin based views
  * Grantlee
  * Clearsilver
  * JSON
 * Dispatcher
  * Chained
  * Path
 * Plugins
  * Session
  * Authentication (with PBKDF2)
  * Authorization with RoleACL
 * Upload parser
 * JSON body as QJsonDocument when uploaded data is in JSON type
 * C++11
 * Chunked reponses
 * Request profiling/stats

## LICENSE

The library is under the LGPLv2 and public header files, documentation and
examples are under MIT license.

## Requirements

 * uWSGI - to receive and parse protocols requests (>= 1.8 recommended)
 * CMake - for the build system (>= 3.0)
 * Qt - the core library of this framework (>= 5.4)
 
