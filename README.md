# Cutelyst - The Qt Web Framework <img title="Cutelyst" src="http://i.imgur.com/us1pKAP.png" width="60px" alt="Cutelyst logo"/>

[![Windows Build status](https://ci.appveyor.com/api/projects/status/github/cutelyst/cutelyst?branch=master&svg=true)](https://ci.appveyor.com/project/dantti/cutelyst/branch/master)
[![Code Quality: Cpp](https://img.shields.io/lgtm/grade/cpp/g/cutelyst/cutelyst.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/cutelyst/cutelyst/context:cpp)
[![Total Alerts](https://img.shields.io/lgtm/alerts/g/cutelyst/cutelyst.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/cutelyst/cutelyst/alerts)

A Web Framework built on top of Qt, using the simple and elegant approach of Catalyst (Perl) framework.

Qt's meta object system is what powers the core of Cutelyst, it allows for introspecting controller's methods signatures and generate matching actions that can be invoked later.

## BENCHMARKS

Don't trust us on being fast, check out the most comprehensive web framework benchmarks by TechEmpower 
http://www.techempower.com/benchmarks/

## FEATURES:

 * Cross-platform
 * Stable API/ABI - v3 tagged from master, v2 tags, v1 on v1.x.x branch (unmaintained)
 * Pluggable Engines
   * Cutelyst::Server - A cross-platform and fast server engine
     * HTTP/1.1 - Pipelining and Keep-Alive
     * HTTP/2 - Upgrade to H2, ALPN negotiation on HTTPS and direct H2C
     * FastCGI - Pipelining and Keep-Alive
   * [uWSGI](http://projects.unbit.it/uwsgi) - Multiple protocols support (HTTP 1.0, FastCGI, uWSGI)
 * WebSockets
 * REST with ActionREST
 * Plugin based views
   * [Cutelee](https://github.com/cutelyst/cutelee) (A Qt implementation of Django's template engine)
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
   * Validator (to validate user input)
   * CSRF protection
   * Memcached
   * UserAgent
 * Asynchronous processing (just don't use local QEventLoops or it will eventually crash)
   * Async SQL with [ASql](https://github.com/cutelyst/asql)
 * Upload parser
 * JSON body as QJsonDocument when uploaded data is in JSON format
 * C++17
 * Chunked reponses (via QIODevice write API)
 * Request profiling/stats
 * Unit tested
 * QtCreator integration

## DOCUMENTATION

Get started with our [Tutorial](https://github.com/cutelyst/cutelyst/wiki/Tutorial_01_Intro) or check the [API](http://api.cutelyst.org).

## COMMUNITY

The Cutelyst project IRC channel is [#cutelyst](http://webchat.freenode.net/?channels=%23cutelyst) on freenode.

Or you can use the [Mailing List](https://groups.google.com/forum/#!forum/cutelyst)

## REQUIREMENTS

 * CMake - for the build system (>= 3.9)
 * Qt - the core library of this framework (>= 5.12)

## OPTIONAL

  * uWSGI - to receive and parse protocols requests (>= 1.9 recommended)

## LICENSE

The source code is available is under the 3-Clause BSD.
