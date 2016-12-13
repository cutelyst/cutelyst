# Cutelyst - The Qt Web Framework <img title="Cutelyst" src="http://i.imgur.com/us1pKAP.png" width="60px" alt="Cutelyst logo"/>

[![Build Status](https://travis-ci.org/cutelyst/cutelyst.svg?branch=master)](https://travis-ci.org/cutelyst/cutelyst)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/github/cutelyst/cutelyst?branch=master&svg=true)](https://ci.appveyor.com/project/dantti/cutelyst/branch/master)

A Web Framework built on top of Qt, using the simple and elegant approach of Catalyst (Perl) framework.

Qt's meta object system is what powers the core of Cutelyst, it allows for introspecting controller's methods signatures and generate matching actions that can be invoked later. Main features:

 * Cross-platform
 * Pluggable Engines
  * uWSGI - Multiple protocols support (HTTP 1.0, FastCGI, uWSGI)
  * Cutelyst-WSGI - A cross-platform and fast WSGI engine (HTTP 1.1 with Pipelining and Keep-Alive)
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
 
## BENCHMARKS

http://www.techempower.com/benchmarks/#section=data-r13

## LICENSE

The library is under the LGPLv2+ and public header files, documentation and
examples are under MIT license.

## REQUIREMENTS

 * CMake - for the build system (>= 3.1)
 * Qt - the core library of this framework (>= 5.6)

## OPTIONAL

  * uWSGI - to receive and parse protocols requests (>= 1.9 recommended)
