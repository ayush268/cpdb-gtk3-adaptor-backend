#include "config.h"

#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <cpdb-libs-frontend.h>
#include <errno.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include <glib/gstdio.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

#include <gtk/gtk.h>
#include <gtk/gtkprintbackend.h>
#include <gtk/gtkunixprint.h>
#include <gtk/gtkprinter-private.h>

#include "gtkprintbackendcpdb.h"

#include <gtkprintutils.h>

typedef struct _GtkPrintBackendCpdbClass GtkPrintBackendCpdbClass;

#define GTK_PRINT_BACKEND_CPDB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PRINT_BACKEND_CPDB, GtkPrintBackendCpdbClass))
#define GTK_IS_PRINT_BACKEND_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PRINT_BACKEND_CPDB))
#define GTK_PRINT_BACKEND_CPDB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PRINT_BACKEND_CPDB, GtkPrintBackendCpdbClass))

#define _STREAM_MAX_CHUNK_SIZE 8192

static GType print_backent_cpdb_type = 0;

struct _GtkPrintBackendCpdbClass
{
  GtkPrintBackendClass parent_class;
};

struct _GtkPrintBackendCpdb
{
  GtkPrintBackend parent_instance;
  FrontendObj *f;
}

static GObjectClass *backend_parent_class;

static void                 gtk_print_backend_cpdb_class_init      (GtkPrintBackendCpdbClass *class);
static void                 gtk_print_backend_cpdb_init            (GtkPrintBackendCpdb *impl);
//static void                 gtk_print_backend_cpdb_finalize        (GObject *object);
//static void                 gtk_print_backend_cpdb_dispose         (GObject *object);
//static void                 cpdb_request_printer_list              (GtkPrintBackend *print_backend);
static void                 cpdb_printer_request_details           (GtkPrinter *printer);
static GtkPrintCapabilities cpdb_printer_get_capabilities          (GtkPrinter *printer);
static GtkPrinterOptionSet *cpdb_printer_get_options               (GtkPrinter *printer,
                                                                    GtkPrintSettings *settings,
                                                                    GtkPageSetup *page_setup,
                                                                    GtkPrintCapabilities capabilities);
static void                 cpdb_printer_get_settings_from_options (GtkPrinter *printer,
                                                                    GtkPrinterOptionSet *options,
                                                                    GtkPrintSettings *settings);
static void                 cpdb_printer_prepare_for_print         (GtkPrinter *printer,
                                                                    GtkPrintJob *print_job,
                                                                    GtkPrintSettings *settings,
                                                                    GtkPageSetup *page_setup);
static cairo_surface_t *    cpdb_printer_create_cairo_surface      (GtkPrinter *printer,
                                                                    GtkPrintSettings *settings,
                                                                    gdouble width,
                                                                    gdouble height,
                                                                    GIOChannel *cache_io);
static void                 gtk_print_backend_cpdb_print_stream    (GtkPrintBackend *print_backend,
                                                                    GtkPrintJob *job,
                                                                    GIOChannel *data_io,
                                                                    GtkPrintJobCompleteFunc callback,
                                                                    gpointer user_data,
                                                                    GDestroyNotify dnotify);
//static GList *              cpdb_printer_list_papers               (GtkPrinter *printer);
//static GtkPageSetup *       cpdb_printer_get_default_page_size     (GtkPrinter *printer);

// TODO Declare any more functions which are needed

static int add_printer_callback(PrinterObj *p)
{
  print_basic_options(p);
}

static int remove_printer_callback(PrinterObj *p)
{
  g_message("CPDB: Removed Printer %s : %s!\n",p->name, p->backend_name);
}

static void gtk_print_backend_cpdb_register_type (GTypeModule *module)
{
  const GTypeInfo print_backend_cpdb_info =
  {
    sizeof (GtkPrintBackendCpdbClass),
    NULL, // base_init
    NULL, // base_finalize
    (GClassInitFunc) gtk_print_backend_cpdb_class_init,
    NULL, // class_finalize
    NULL, // class_data
    sizeof (GtkPrintBackendCpdb),
    0,    // n_preallocs
    (GInstanceInitFunc) gtk_print_backend_cpdb_init,
  };

  print_backend_cpdb_type = g_type_module_register_type (module,
                                                         GTK_TYPE_PRINT_BACKEND,
                                                         "GtkPrintBackendCpdb",
                                                         &print_backend_cpdb_info,
                                                         0);

}

G_MODULE_EXPORT void pb_module_init (GTypeModule *module)
{
  gtk_print_backend_cpdb_register_type (module);
  //gtk_printer_cpdb_register_type (module);
  //TODO check if anything else is needed here
}

G_MODULE_EXPORT void pb_module_exit (void)
{

}

G_MODULE_EXPORT GtkPrintBackend *pb_module_create (void)
{
  return gtk_print_backend_cpdb_new ();
}

Gtype gtk_print_backend_cpdb_get_type (void)
{
  return print_backend_cpdb_type;
}

GtkPrintBackend *gtk_print_backend_cpdb_new (void)
{
  return g_boject_new (GTK_TYPE_PRINT_BACKEND_CPDB, NULL);
}

static void gtk_print_backend_cpdb_class_init (GtkPrintBackendCpdbClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  GtkPrintBackendClass *backend_class = GTK_PRINT_BACKEND_CLASS (class);

  backend_parent_class = g_type_class_peek_parent (class);
  
  //gobject_class->finalize = gtk_print_backend_cpdb_finalize;
  //gobject_class->dispose = gtk_print_backend_cpdb_dispose;

  //backend_class->request_print_list = cpdb_request_printer_list;
  backend_class->printer_request_details = cpdb_printer_request_details;
  backend_class->printer_get_capabilities = cpdb_printer_get_capabilities;
  backend_class->printer_get_options = cpdb_printer_get_options;
  backend_class->printer_get_settings_from_options = cpdb_printer_get_settings_from_options;
  backend_class->printer_prepare_for_print = cpdb_printer_prepare_for_print;
  backend_class->print_stream = gtk_print_backend_cpdb_print_stream;
  backend_class->printer_create_cairo_surface = cpdb_printer_create_cairo_surface;
  //backend_class->printer_list_papers = cpdb_printer_list_papers;
  //backend_class->printer_get_default_page_size = cpdb_printer_get_default_page_size;

  //TODO If any new functions are added, then add them here also
}

static cairo_status_t _cairo_write(void *closure,
                                   const unsigned char *data,
                                   unsigned int length)
{
  GIOChannel *io = (GIOChannel *)closure;
  gzise bytes_written = 0;
  GError *error = NULL;

  while(length > 0)
  {
    g_io_channel_write_chars (io, (gchar *)data, length, &bytes_written, &error);

    if(error != NULL)
    {
      GTK_NOTE (PRINTING, g_print ("CPDB Backend: Error writing to temp file, %s\n", error->message));
      g_error_free (error);
      return CAIRO_STATUS_WRITE_ERROR;
    }

    data += bytes_written;
    length -= bytes_written;
  }

  return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t *cpdb_printer_create_cairo_surface(GtkPrinter *printer,
                                                          GtkPrintSettings *settings,
                                                          gdouble width,
                                                          gdouble height,
                                                          GIOChannel *cache_io)
{
  cairo_surface_t *surface;
  surface = cairo_pdf_surface_create_for_stream (_cairo_write, cache_io, width, height);

  cairo_surface_set_fallback_resolution (surface,
                                         2.0 * gtk_print_settings_get_printer_lpi (settings),
                                         2.0 * gtk_print_settings_get_printer_lpi (settings));

  return surface;
}

typedef struct {
  GtkPrintBackend *backend;
  GtkPrintJobCompleteFunc callback;
  GtkPrintJob *job;
  gpointer user_data;
  GDestroyNotify dnotify;

  // TODO do we need anything else here?
} CpdbPrintStreamData;

static void cpdb_print_cb_locked (GtkPrintBackendCpdb *print_backend,
                                  GError *error,
                                  gpointer user_data)
{
  CpdbPrintStreamData *ps = (CpdbPrintStreamData *) user_data;
  
  if (ps->callback)
    ps->callback (ps->job, ps->user_data, error);

  if (ps->dnotify)
    ps->dnotify (ps->user_data);

  gtk_print_job_set_status (ps->job,
                            error ? GTK_PRINT_STATUS_FINISHED_ABORTED
                                  : GTK_PRINT_STATUS_FINISHED);

  if (ps->job)
    g_object_unref (ps->job);

  g_free (ps);
}

static void cpdb_print_cb (GtkPrintBackendCpdb *print_backend,
                           GError *error,
                           gpointer user_data);
{
  gdk_threads_enter ();

  cpdb_print_cb_locked (print_backend, error, user_data);

  gdk_threads_leave ();
}

//static void cpdb_request_printer_list (GtkPrintBackend *backend)
//{
//  GtkPrintBackendCpdb *backend;
//  
//  event_callback add_cb = (event_callback)add_printer_callback;
//  event_callback rem_cb = (event_callback)remove_printer_callback;
//
//  backend = get_new_FrontendObj(NULL, add_cb, rem_cb);
//
//  connect_to_dbus(backend);
//
//  cpdb_get_printer_list (backend);
//}

static gboolean file_write (GIOChannel *source,
                            GIOCondition con,
                            gpointer user_data)
{
  //TODO Complete this
}

static void gtk_print_backend_file_print_stream (GtkPrintBackend *print_backend,
                                                 GtkPrintJob *job,
                                                 GIOChannel *data_io,
                                                 GtkPrintJobCompleteFunc callback,
                                                 gpointer user_data,
                                                 GDestroyNotify dnotify)
{
  //TODO Complete this
}


