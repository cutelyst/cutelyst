/*
 * SPDX-FileCopyrightText: (C) 2020-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/async.h>
#include <coroutine>
#include <functional>
#include <memory>

#include <QObject>

namespace Cutelyst {

/**
 * @brief The CoroContext class
 *
 * This is a VOID coroutine context aimed at making coroutine
 * usage in Cutelyst safe.
 *
 * When entering the coroutine body one must call `co_yield context;`
 * This replaces the need for ASync if only coroutines are used.
 *
 * In the case of client disconnect Cutelyst::Context destroy() signal
 * is emitted and destroys this coroutine, making sure no use after
 * free is done.
 *
 */
class CoroContext
{
public:
    struct promise_type {
        std::coroutine_handle<promise_type> handle;
        std::vector<QMetaObject::Connection> connections;

        void clean()
        {
            for (auto &conn : connections) {
                QObject::disconnect(conn);
            }
            connections.clear();
        }

        void return_void() noexcept {}

        CoroContext get_return_object()
        {
            handle = std::coroutine_handle<promise_type>::from_promise(*this);
            return {};
        }

        std::suspend_never initial_suspend() const noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void unhandled_exception() {}

        bool await_ready() const noexcept { return false; }

        std::suspend_never yield_value(QObject *obj)
        {
            auto conn = QObject::connect(obj, &QObject::destroyed, [this] {
                clean();

                if (handle) {
                    handle.destroy();
                }
            });
            connections.emplace_back(std::move(conn));
            return {};
        }

        std::suspend_never yield_value(Cutelyst::Context *obj)
        {
            // Automatically delay replies
            // async cannot be used in coroutine body
            // else we get a double free when the coroutine
            // body ends and Cutelyst::Engine deletes the Context*
            // resulting in destroyed signal being emitted and
            // and coroutine dtor already on the stack to be called
            ASync a(obj);

            auto conn = QObject::connect(obj, &QObject::destroyed, [this, a] {
                clean();

                if (handle) {
                    handle.destroy();
                }
            });
            connections.emplace_back(std::move(conn));
            return {};
        }
        void await_suspend(std::coroutine_handle<> h) noexcept {}
        void await_resume() const noexcept {}

        ~promise_type() { clean(); }
    };
};

} // namespace Cutelyst
