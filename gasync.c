/* SPDX-FileCopyrightText: 2020 Jason Francis <jason@cycles.network>
 * SPDX-License-Identifier: CC0-1.0 */

#include "gasync.h"

#include <errno.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <ucontext.h>
#include <unistd.h>

typedef struct _GAsyncContext {
  ucontext_t outer;
  ucontext_t inner;
  size_t stacksize;
  void  *stack;
  GObject      *last_source;
  GAsyncResult *last_result;
  GCancellable *cancellable;
} GAsyncContext;

void g_await_callback(GObject *source, GAsyncResult *result, gpointer data) {
  GAsyncContext *context = data;
  context->last_source = source;
  context->last_result = result;
  swapcontext(&context->outer, &context->inner);
}

void g_await_escape(GAsyncContext *context) {
  swapcontext(&context->inner, &context->outer);
}

GCancellable *g_async_context_get_cancellable(GAsyncContext *context) {
  return context->cancellable;
}

GObject *g_async_context_get_last_source(GAsyncContext *context) {
  return context->last_source;
}

GAsyncResult *g_async_context_get_last_result(GAsyncContext *context) {
  return context->last_result;
}

static int async_context_idle_free(void *data) {
  GAsyncContext *context = data;
  munmap(context->stack, context->stacksize);
  g_clear_object(&context->cancellable);
  g_slice_free(GAsyncContext, context);
  return G_SOURCE_REMOVE;
}

G_GNUC_NORETURN
static void async_exec(GAsyncContext *context,
                       GAsyncCoroutineFunc function,
                       gpointer user_data) {
  function(context, user_data);
  g_idle_add(async_context_idle_free, context);
  setcontext(&context->outer);
  g_error("Tried to escape into already completed async context");
  g_abort();
}

void g_async_run(GAsyncCoroutineFunc function,
                 gpointer user_data,
                 GCancellable *cancellable) {
  GAsyncContext *context = g_slice_new0(GAsyncContext);

  context->cancellable = cancellable ? g_object_ref(cancellable) : NULL;

  struct rlimit stacksize;
  getrlimit(RLIMIT_STACK, &stacksize);
  size_t pagesize = sysconf(_SC_PAGESIZE);

  context->stacksize = stacksize.rlim_cur;
  context->stack = mmap(NULL, context->stacksize, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_STACK, -1, 0);
  mprotect(context->stack, pagesize, PROT_NONE);

  context->inner.uc_stack.ss_flags = 0;
  context->inner.uc_stack.ss_size = context->stacksize - pagesize;
  context->inner.uc_stack.ss_sp = context->stack + pagesize;
  context->inner.uc_link = &context->outer;

  getcontext(&context->inner);
  makecontext(&context->inner, (void(*)(void)) async_exec,
              3, context, function, user_data);
  swapcontext(&context->outer, &context->inner);
}
