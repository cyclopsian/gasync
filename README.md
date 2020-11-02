# GAsync

Implements async/await in C for GLib GIO using stackful coroutines. Designed
to be used with the GLib main loop and asynchronous GIO methods. These are the
ones typically suffixed with `_async` and `_finish` that take
`GAsyncReadyCallback` and `GAsyncResult`.

### Public API

```c
typedef void (*GAsyncCoroutineFunc)(GAsyncContext *context, gpointer user_data);

void g_async_run(GAsyncCoroutineFunc function,
                 gpointer user_data,
                 GCancellable *cancellable);

#define G_AWAIT(func, context, error, ...)
#define G_AWAIT_OUT(func, context, error, outparams, ...)
#define G_AWAIT_CONSTRUCTOR(func, context, error, ...)
```

### Usage

Spawn a new coroutine with `g_async_run`:

```c
  g_autoptr(GCancellable) cancellable = g_cancellable_new();
  g_async_run(my_coroutine, mydata, cancellable);
```

Then, call the `G_AWAIT` macros inside the coroutine:

```c
static void my_coroutine(GAsyncContext *context, gpointer user_data)
{
  GError *error = NULL;
  MyData *mydata = user_data;
  g_autoptr(GFileInputStream) stream = NULL;
  gboolean result;
  g_autofree char *contents = NULL;
  gsize length;
  char *etag;
  g_autoptr(GDBusConnection) bus = NULL;

  /* G_AWAIT is the basic form.
   * The macro calls both g_file_read_async and g_file_read_finish. */
  stream = G_AWAIT(g_file_read, context, &error, mydata->file, 100);

  /* The caller must handle errors. */
  if (error != NULL) {
    g_critical("Can't read file!");
    return;
  }

  /* G_AWAIT_OUT calls a method with output parameters in finish.
   * Output parameters come first passed as a "tuple" of sorts. */
  result = G_AWAIT_OUT(g_file_load_contents, context, &error,
                       (&contents, &length, &etag), mydata->file);

  /* G_AWAIT_CONSTRUCTOR calls a method without an object in finish.
   * The macro calls both g_bus_get and g_bus_get_finish. */
  bus = G_AWAIT_CONSTRUCTOR(g_bus_get, context, &error, G_BUS_TYPE_SESSION);
}
```

See `gasync-example.c` for an example using a main loop.
