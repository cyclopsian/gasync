/* SPDX-FileCopyrightText: 2020 Jason Francis <jason@cycles.network>
 * SPDX-License-Identifier: CC0-1.0 */

#include "gasync.h"

static void async_read_file(const char *path,
    GAsyncContext *context, GError **out_error) {
  GError *error = NULL;
  g_autoptr(GFile) file = g_file_new_for_path(path);
  g_autoptr(GFileInputStream) stream
    = G_AWAIT(g_file_read, context, &error, file, 100);
  if (error) {
    g_propagate_error(out_error, error);
    return;
  }

  g_autoptr(GOutputStream) sout = g_memory_output_stream_new_resizable();
  G_AWAIT(g_output_stream_splice, context, &error,
      sout, G_INPUT_STREAM(stream),
      G_OUTPUT_STREAM_SPLICE_NONE, G_PRIORITY_DEFAULT);
  gpointer data = g_memory_output_stream_get_data(G_MEMORY_OUTPUT_STREAM(sout));
  gsize size = g_memory_output_stream_get_size(G_MEMORY_OUTPUT_STREAM(sout));
  write(STDOUT_FILENO, data, size);
}

static GMainLoop *loop;
static int ret = 0;

static void read_file(GAsyncContext *context, gpointer user_data) {
  GError *error = NULL;
  async_read_file(user_data, context, &error);
  if (error) {
    g_critical("%s\n", error->message);
    ret = 1;
  }
  g_main_loop_quit(loop);
}

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;
  g_async_run(read_file, argv[1], NULL);
  loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  return ret;
}
