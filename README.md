# Cutelyst - The Qt Web Framework

[![Build Status](https://travis-ci.org/cutelyst/cutelyst.svg?branch=master)](https://travis-ci.org/cutelyst/cutelyst)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/github/cutelyst/cutelyst?branch=master&svg=true)](https://ci.appveyor.com/project/dantti/cutelyst/branch/master)

A Web Framework built on top of Qt, using the simple and elegant approach of Catalyst (Perl) framework.

Qt's meta object system is what powers the core of Cutelyst, this allows for introspecting controller's methods signatures and generate matching actions that can be invoked later. Some features:

 * Multiple protocols support when using uWSGI engine
  * HTTP 1.1
  * FastCGI
  * uWSGI
 * REST with ActionREST
 * Plugin based views
  * [Grantlee](http://www.grantlee.org) (A Qt implementation of Django's template engine)
  * [Clearsilver](http://www.clearsilver.net)
  * JSON
  * [Email](https://github.com/cutelyst/simple-mail)
 * Dispatcher
  * Chained
  * Path
 * Plugins
  * Session
  * Authentication (with PBKDF2)
  * Authorization with RoleACL
  * StatusMessage
 * Upload parser
 * JSON body as QJsonDocument when uploaded data is in JSON format
 * C++11
 * Chunked reponses (via QIODevice write API)
 * Request profiling/stats
 * Asynchronous processing (optional and dependent on application code)
 * Unit tested
 * QtCreator integration

## LICENSE

The library is under the LGPLv2+ and public header files, documentation and
examples are under MIT license.

## REQUIREMENTS

 * uWSGI - to receive and parse protocols requests (>= 2.0 recommended)
 * CMake - for the build system (>= 3.1)
 * Qt - the core library of this framework (>= 5.6)
 
