/* SPDX-FileCopyrightText: 2020 Jason Francis <jason@cycles.network>
 * SPDX-License-Identifier: CC0-1.0 */

#ifndef __G_ASYNC_H__
#define __G_ASYNC_H__

#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _GAsyncContext GAsyncContext;

typedef void (*GAsyncCoroutineFunc)(GAsyncContext *context, gpointer user_data);

void g_await_callback(GObject *source, GAsyncResult *result, gpointer data);
void g_await_escape(GAsyncContext *context);
GCancellable *g_async_context_get_cancellable(GAsyncContext *context);
GObject *g_async_context_get_last_source(GAsyncContext *context);
GAsyncResult *g_async_context_get_last_result(GAsyncContext *context);

void g_async_run(GAsyncCoroutineFunc function,
                 gpointer user_data,
                 GCancellable *cancellable);

#define _G_UNPACK_ARGS(...) __VA_ARGS__
#define _G_CALL_MACRO(m, args) m args

#define G_AWAIT(_func, _context, _error, ...) \
  ( \
    _func##_async(__VA_ARGS__, \
                  g_async_context_get_cancellable(_context), \
                  g_await_callback, \
                  (_context)), \
    g_await_escape(_context), \
    _func##_finish((void *) g_async_context_get_last_source(_context), \
                   g_async_context_get_last_result(_context), \
                   (_error)))

#define G_AWAIT_OUT(_func, _context, _error, _outparams, ...) \
  ( \
    _func##_async(__VA_ARGS__, \
                  g_async_context_get_cancellable(_context), \
                  g_await_callback, \
                  (_context)), \
    g_await_escape(_context), \
    _func##_finish((void *) g_async_context_get_last_source(_context), \
                   g_async_context_get_last_result(_context), \
                   _G_CALL_MACRO(_G_UNPACK_ARGS, _outparams), \
                   (_error)))

#define G_AWAIT_CONSTRUCTOR(_func, _context, _error, ...) \
  ( \
    _func(__VA_ARGS__, \
          g_async_context_get_cancellable(_context), \
          g_await_callback, \
          (_context)), \
    g_await_escape(_context), \
    _func##_finish(g_async_context_get_last_result(_context), \
                   (_error)))

G_END_DECLS

#endif
