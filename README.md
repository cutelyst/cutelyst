# Cutelyst - The Qt Web Framework <img title="Cutelyst" src="http://i.imgur.com/us1pKAP.png" width="60px" alt="Cutelyst logo"/>

A Web Framework built on top of Qt, using the simple and elegant approach of Catalyst (Perl) framework.

Qt's meta object system is what powers the core of Cutelyst, it allows for introspecting controller's methods signatures and generate matching actions that can be invoked later.

## BENCHMARKS

Don't trust us on being fast, check out the most comprehensive web framework benchmarks by TechEmpower
http://www.techempower.com/benchmarks/

## FEATURES:

 * Cross-platform
 * Stable API/ABI - v3 tagged from v3.x.x, v2 tags, v1 on v1.x.x branch (unmaintained)
 * Pluggable Engines
   * Cutelyst::Server - A cross-platform and fast server engine
     * HTTP/1.1 - Pipelining and Keep-Alive
     * HTTP/2 - Upgrade to H2, ALPN negotiation on HTTPS and direct H2C
     * FastCGI - Pipelining and Keep-Alive
 * WebSockets
 * REST with ActionREST
 * Plugin based views
   * [Cutelee](https://github.com/cutelyst/cutelee) (A Qt implementation of Django's template engine)
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
 * C++20
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

 * CMake - for the build system (>= 3.16)
 * Qt - the core library of this framework (>= 6.2)

## LICENSE

The source code is available is under the 3-Clause BSD.
