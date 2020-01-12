/**
 *  Volnoti - Lightweight Volume Notification
 *  Copyright (C) 2011  David Brazdil <db538@cam.ac.uk>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <dbus/dbus-glib.h>

#include "common.h"
#include "gopt.h"
#include "notification.h"

#define IMAGE_PATH   PREFIX

typedef struct {
        GObject parent;

        gint volume;
        gboolean muted;
        gboolean brightness;
        gint nobar;
        const gchar* muteicon;
        const gchar* officon;
        const gchar* lowicon;
        const gchar* medicon;
        const gchar* highicon;
        const gchar* brighticon;
        const gchar* singleicon;

        GtkWindow *notification;

        GdkPixbuf *icon_high;
        GdkPixbuf *icon_medium;
        GdkPixbuf *icon_low;
        GdkPixbuf *icon_off;
        GdkPixbuf *icon_muted;
        GdkPixbuf *icon_bright;

        GdkPixbuf *image_progressbar_empty;
        GdkPixbuf *image_progressbar_full;
        GdkPixbuf *image_progressbar;
        gint width_progressbar;
        gint height_progressbar;

        gint time_left;
        gint timeout;
        gboolean debug;
        Settings settings;
} VolumeObject;

typedef struct {
        GObjectClass parent;
} VolumeObjectClass;

GType volume_object_get_type(void);

gboolean volume_object_notify(VolumeObject* obj,
                              gint value_in,
                              gint nobar_in,
                              gint brightness_in,
                              const gchar* muteicon_in,
                              const gchar* officon_in,
                              const gchar* lowicon_in,
                              const gchar* medicon_in,
                              const gchar* highicon_in,
                              const gchar* singleicon_in,
                              const gchar* brighticon_in,
                              GError** error);

#define VOLUME_TYPE_OBJECT \
        (volume_object_get_type())
#define VOLUME_OBJECT(object) \
        (G_TYPE_CHECK_INSTANCE_CAST ((object), \
                                     VOLUME_TYPE_OBJECT, VolumeObject))
#define VOLUME_OBJECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                  VOLUME_TYPE_OBJECT, VolumeObjectClass))
#define VOLUME_IS_OBJECT(object) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((object), \
                                     VOLUME_TYPE_OBJECT))
#define VOLUME_IS_OBJECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                  VOLUME_TYPE_OBJECT))
#define VOLUME_OBJECT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                    VOLUME_TYPE_OBJECT, VolumeObjectClass))

G_DEFINE_TYPE(VolumeObject, volume_object, G_TYPE_OBJECT)

#include "value-daemon-stub.h"

char * i_high = IMAGE_PATH "volume_high_dark.svg";
char * i_medium = IMAGE_PATH "volume_medium_dark.svg";
char * i_low = IMAGE_PATH "volume_low_dark.svg";
char * i_off = IMAGE_PATH "volume_off_dark.svg";
char * i_muted = IMAGE_PATH "volume_muted_dark.svg";

char * i_brightness = IMAGE_PATH "display-brightness-dark.svg";

char * pb_empty = IMAGE_PATH "progressbar_empty_dark.svg";
char * pb_full = IMAGE_PATH "progressbar_full_dark.svg";

/**
 * volume object init
 * @param obj
 */
static void volume_object_init(VolumeObject* obj) {
        g_assert(obj != NULL);
        obj->notification = NULL;
}

/**
 * volume object class init
 *
 * @param klass
 */
static void volume_object_class_init(VolumeObjectClass* klass) {
        g_assert(klass != NULL);

        dbus_g_object_type_install_info(VOLUME_TYPE_OBJECT,
                                        &dbus_glib_volume_object_object_info);
}

/**
 * time handler
 *
 * @param  obj
 *
 * @return
 */
static gboolean time_handler(VolumeObject *obj)
{
        g_assert(obj != NULL);

        obj->time_left--;

        if (obj->time_left <= 0) {
                print_debug("Destroying notification...", obj->debug);
                destroy_notification(obj->notification);
                obj->notification = NULL;
                print_debug_ok(obj->debug);
                return FALSE;
        }

        return TRUE;
}

/**
 * volume_object_notify
 *
 * @param  obj        [description]
 * @param  value      [description]
 * @param  nobarvalue [description]
 * @param  muteicon   [description]
 * @param  officon    [description]
 * @param  lowicon    [description]
 * @param  medicon    [description]
 * @param  highicon   [description]
 * @param  singleicon [description]
 * @param  error      [description]
 * @return            [description]
 */
gboolean volume_object_notify(VolumeObject* obj,
                              gint value,
                              gint nobarvalue,
                              gint brightness,
                              const gchar* muteicon,
                              const gchar* officon,
                              const gchar* lowicon,
                              const gchar* medicon,
                              const gchar* highicon,
                              const gchar* singleicon,
                              const gchar* brighticon,
                              GError** error) {
        g_assert(obj != NULL);

        if (value < 0) {
                obj->muted = TRUE;
                obj->volume = 0;
        } else {
                obj->muted = FALSE;
                obj->volume = (value > 100) ? 100 : value;
        }

        obj->brightness = brightness;

        if (nobarvalue == 1) {
                print_debug("Disablingg progress bar...", obj->debug);
                obj->image_progressbar = gdk_pixbuf_new_from_file(IMAGE_PATH "empty.png", NULL);
                obj->image_progressbar_empty = gdk_pixbuf_new_from_file(IMAGE_PATH "empty.png", NULL);
                obj->image_progressbar_full = gdk_pixbuf_new_from_file(IMAGE_PATH "empty.png", NULL);
                print_debug_ok(obj->debug);
        }

        if (obj->notification == NULL) {
                print_debug("Creating new notification...", obj->debug);
                obj->notification = create_notification(obj->settings);
                gtk_widget_realize(GTK_WIDGET(obj->notification));
                g_timeout_add(1000, (GSourceFunc) time_handler, (gpointer) obj);
                print_debug_ok(obj->debug);
        }

        if(brighticon && brighticon[0] != '\0') {
                obj->icon_bright = gdk_pixbuf_new_from_file(brighticon, NULL);
        }
        if (singleicon && singleicon[0] != '\0') {
                obj->icon_muted = gdk_pixbuf_new_from_file(singleicon, NULL);
                obj->icon_off = gdk_pixbuf_new_from_file(singleicon, NULL);
                obj->icon_low = gdk_pixbuf_new_from_file(singleicon, NULL);
                obj->icon_medium = gdk_pixbuf_new_from_file(singleicon, NULL);
                obj->icon_high = gdk_pixbuf_new_from_file(singleicon, NULL);
        }else {

                if (muteicon && muteicon[0] != '\0') {
                        obj->icon_muted = gdk_pixbuf_new_from_file(muteicon, NULL);
                }

                if (officon && officon[0] != '\0') {
                        obj->icon_off = gdk_pixbuf_new_from_file(officon, NULL);
                }

                if (lowicon && lowicon[0] != '\0') {
                        obj->icon_low = gdk_pixbuf_new_from_file(lowicon, NULL);
                }

                if (medicon && medicon[0] != '\0') {
                        obj->icon_medium = gdk_pixbuf_new_from_file(medicon, NULL);
                }

                if (highicon && highicon[0] != '\0') {
                        obj->icon_high = gdk_pixbuf_new_from_file(highicon, NULL);
                }

        }

        // choose icon
        if (obj->brightness)
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_bright);
        else if (obj->muted)
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_muted);
        else if (obj->volume >= 75)
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_high);
        else if (obj->volume >= 50)
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_medium);
        else if (obj->volume >= 25)
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_low);
        else
                set_notification_icon(GTK_WINDOW(obj->notification), obj->icon_off);

        // prepare and set progress bar


        if (obj->nobar != 0) {
                print_debug("Composing progress bar...", obj->debug);  
                gint width_full = obj->width_progressbar * obj->volume / 100;
                gdk_pixbuf_copy_area(obj->image_progressbar_full, 0, 0, width_full, obj->height_progressbar,
                                obj->image_progressbar, 0, 0);
                gdk_pixbuf_copy_area(obj->image_progressbar_empty, width_full, 0, obj->width_progressbar - width_full, obj->height_progressbar,
                                obj->image_progressbar, width_full, 0);
                set_progressbar_image(GTK_WINDOW(obj->notification), obj->image_progressbar);
                print_debug_ok(obj->debug);
        } else {
                obj->image_progressbar = NULL;
        }

        obj->time_left = obj->timeout;
        gtk_widget_show_all(GTK_WIDGET(obj->notification));

        // reset icons
        obj->icon_muted = gdk_pixbuf_new_from_file(i_muted, NULL);
        obj->icon_high = gdk_pixbuf_new_from_file(i_high, NULL);
        obj->icon_medium = gdk_pixbuf_new_from_file(i_medium, NULL);
        obj->icon_low = gdk_pixbuf_new_from_file(i_low, NULL);
        obj->icon_off = gdk_pixbuf_new_from_file(i_off, NULL);
        obj->icon_bright = gdk_pixbuf_new_from_file(i_brightness, NULL);
        obj->image_progressbar_empty = gdk_pixbuf_new_from_file(pb_empty, NULL);
        obj->image_progressbar_full = gdk_pixbuf_new_from_file(pb_full, NULL);

        return TRUE;
}

/**
 * Print usage
 *
 * @param filename
 * @param failure
 */
static void print_usage(const char* filename, int failure) {
        Settings settings = get_default_settings();
        g_print("Usage: %s [arguments]\n"
                " -h\t\t--help\t\t\thelp\n"
                " -v\t\t--verbose\t\tverbose\n"
                " -n\t\t--no-daemon\t\tdo not daemonize\n"
                "\n"
                "Configuration:\n"
                " -t <int>\t--timeout <int>\t\tnotification timeout in seconds\n"
                " -a <float>\t--alpha <float>\t\ttransparency level (0.0 - 1.0, default %.2f)\n"
                " -b <int>\t--border <int>\t\tborder size in pixels\n"
                " -p <int>,<int>\t--pos <int>,<int>\t\thorizontal and vertical position\n"
                " -r <int>\t--corner-radius <int>\tradius of the round corners in pixels (default %d)\n"
                " -T <string>\t--theme <string>\ttheme name\n"
                , filename, settings.alpha, settings.corner_radius);
        if (failure)
                exit(EXIT_FAILURE);
        else
                exit(EXIT_SUCCESS);
}

char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
/**
 * Main
 *
 * @param  argc
 * @param  argv
 * @return
 */
int main(int argc, char* argv[]) {
        Settings settings = get_default_settings();
        int timeout = 3;

        DBusGConnection *bus = NULL;
        DBusGProxy *bus_proxy = NULL;
        VolumeObject *status = NULL;
        GMainLoop *main_loop = NULL;
        GError *error = NULL;
        guint result;

        void *options = gopt_sort(&argc, (const char**) argv, gopt_start(
                                          gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
                                          gopt_option('n', 0, gopt_shorts('n'), gopt_longs("no-daemon")),
                                          gopt_option('z', 0, gopt_shorts('z'), gopt_longs("horizontal")),
                                          gopt_option('t', GOPT_ARG, gopt_shorts('t'), gopt_longs("timeout")),
                                          gopt_option('a', GOPT_ARG, gopt_shorts('a'), gopt_longs("alpha")),
                                          gopt_option('b', GOPT_ARG, gopt_shorts('b'), gopt_longs("border")),
                                          gopt_option('p', GOPT_ARG, gopt_shorts('p'), gopt_longs("pos")),
                                          gopt_option('r', GOPT_ARG, gopt_shorts('r'), gopt_longs("corner-radius")),
                                          gopt_option('T', GOPT_ARG, gopt_shorts('T'), gopt_longs("theme")),
                                          gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));

        int help = gopt(options, 'h');
        int debug = gopt(options, 'v');
        int no_daemon = gopt(options, 'n');
        int horizontal = gopt(options, 'z');
        const char* themename;
        settings.horizontal = horizontal;

        if (gopt(options, 't')) {
                if (sscanf(gopt_arg_i(options, 't', 0), "%d", &timeout) != 1)
                        print_usage(argv[0], TRUE);
        }

        if (gopt(options, 'a')) {
                if (sscanf(gopt_arg_i(options, 'a', 0), "%f", &settings.alpha) != 1 || settings.alpha < 0.0f || settings.alpha > 1.0f)
                        print_usage(argv[0], TRUE);
        }

        if (gopt(options, 'b')) {
                if(sscanf(gopt_arg_i(options, 'b', 0), "%d", &settings.border) != 1 || settings.border <= 0) {
                        print_usage(argv[0], TRUE);
                }
        }

        if (gopt(options, 'p')) {
                if(sscanf(gopt_arg_i(options, 'p', 0), "%d,%d", &settings.pos_x, &settings.pos_y) != 2 || settings.pos_x < 0 || settings.pos_y < 0) {
                        print_usage(argv[0], TRUE);
                }
        }

        if (gopt(options, 'r')) {
                if (sscanf(gopt_arg_i(options, 'r', 0), "%d", &settings.corner_radius) != 1)
                        print_usage(argv[0], TRUE);
        }

        // theme
        if(gopt_arg( options, 'T', &themename )) {
                GKeyFile* gkf; /* Notice we declared a pointer */
                gint locTimeOut = 0;

                gchar* bg_color;
                gchar* hi; // high icon
                gchar* lo; // low icon
                gchar* me; // medium icon
                gchar* off; // off icon
                gchar* mu; // mute icon
                gchar* em; // empty progressbar
                gchar* fu; // full progressbar
                gchar* br; // brightnes
                gchar* hz; // horizontal

                gchar* theme_dir = getenv("HOME");

                gint corner_radius;
                gint border;
                gdouble alpha;
                gint pos_x;
                gint pos_y;


                strcat(theme_dir, "/.config/tvolnoti/themes/");
                strcat(theme_dir, themename);
                strcat(theme_dir,"/");

                DIR* dir = opendir(theme_dir);

                if (dir) {
                        closedir(dir);
                } else {
                        fprintf (stderr, "Could not find theme directory %s\n", theme_dir);
                        return EXIT_FAILURE;
                }

                gkf = g_key_file_new();

                gchar* conffile = concat(theme_dir, "theme.conf");

                if (!g_key_file_load_from_file(gkf, conffile, G_KEY_FILE_NONE, NULL)) {
                        fprintf (stderr, "Could not read config file %s\n", conffile);
                        return EXIT_FAILURE;
                }

                free(conffile);

                if(locTimeOut = g_key_file_get_integer(gkf, "General", "timeout", NULL)) {
                        timeout = locTimeOut;
                }

                if(bg_color = g_key_file_get_string(gkf, "Style", "bg_color", NULL)) {
                        settings.color_string = bg_color;
                }

                if(corner_radius = g_key_file_get_integer(gkf, "Style", "corner_radius", NULL)) {
                        settings.corner_radius = corner_radius;
                }

                if(border = g_key_file_get_integer(gkf, "Style", "border", NULL)) {
                        settings.border = border;
                }

                if(pos_x = g_key_file_get_integer(gkf, "Style", "posx", NULL)) {
                        settings.pos_x = pos_x;
                }

                if(pos_y = g_key_file_get_integer(gkf, "Style", "posy", NULL)) {
                        settings.pos_y = pos_y;
                }

                if(alpha = (float)g_key_file_get_double(gkf, "Style", "alpha", NULL)) {
                        settings.alpha = alpha;
                }

                if(hz = g_key_file_get_string(gkf, "Style", "horizontal", NULL)) {
                        if (strcmp(hz, "TRUE") == 0) {
                                settings.horizontal = TRUE;
                        } else if (strcmp(hz, "FALSE") == 0) {
                                settings.horizontal = FALSE;
                        }
                }

                // icons
                if(hi = g_key_file_get_string(gkf, "Icons", "high", NULL)) {
                        i_high = concat(theme_dir, hi);
                } else {
                        i_high = IMAGE_PATH "volume_high_dark.svg";
                }

                if(me = g_key_file_get_string(gkf, "Icons", "medium", NULL)) {
                        i_medium = concat(theme_dir, me);
                } else {
                        i_medium = IMAGE_PATH "volume_medium_dark.svg";
                }

                if(lo = g_key_file_get_string(gkf, "Icons", "low", NULL)) {
                        i_low = concat(theme_dir, lo);
                }

                if(off = g_key_file_get_string(gkf, "Icons", "off", NULL)) {
                        i_off = concat(theme_dir, off);
                }

                if(mu = g_key_file_get_string(gkf, "Icons", "muted", NULL)) {
                        i_muted = concat(theme_dir, mu);
                }

                if(br = g_key_file_get_string(gkf, "Icons", "brightness", NULL)) {
                        i_brightness = concat(theme_dir, br);
                }

                if(em = g_key_file_get_string(gkf, "ProgressBar", "progressbar_empty", NULL)) {
                        pb_empty = concat(theme_dir, em);
                }

                if(fu = g_key_file_get_string(gkf, "ProgressBar", "progressbar_full", NULL)) {
                        pb_full = concat(theme_dir, fu);
                }

                /*
                 * Do what you want to do...
                 * Don't forget to free before you leave.
                 */

                g_key_file_free (gkf);
        }

        gopt_free(options);

        if (help)
                print_usage(argv[0], FALSE);

        // initialize GTK
        g_log_set_always_fatal(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL);
        gtk_init(&argc, &argv);

        // create main loop
        main_loop = g_main_loop_new(NULL, FALSE);
        if (main_loop == NULL)
                handle_error("Couldn't create GMainLoop", "Unknown(OOM?)", TRUE);

        // connect to D-Bus
        print_debug("Connecting to D-Bus...", debug);
        bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
        if (error != NULL)
                handle_error("Couldn't connect to D-Bus",
                             error->message,
                             TRUE);
        print_debug_ok(debug);

        // get the proxy
        print_debug("Getting proxy...", debug);
        bus_proxy = dbus_g_proxy_new_for_name(bus,
                                              DBUS_SERVICE_DBUS,
                                              DBUS_PATH_DBUS,
                                              DBUS_INTERFACE_DBUS);
        if (bus_proxy == NULL)
                handle_error("Couldn't get a proxy for D-Bus",
                             "Unknown(dbus_g_proxy_new_for_name)",
                             TRUE);
        print_debug_ok(debug);

        // register the service
        print_debug("Registering the service...", debug);
        if (!dbus_g_proxy_call(bus_proxy,
                               "RequestName",
                               &error,

                               G_TYPE_STRING,
                               VALUE_SERVICE_NAME,
                               G_TYPE_UINT,
                               0,
                               G_TYPE_INVALID,

                               G_TYPE_UINT,
                               &result,
                               G_TYPE_INVALID))
                handle_error("D-Bus.RequestName RPC failed",
                             error->message,
                             TRUE);
        if (result != 1)
                handle_error("Failed to get the primary well-known name.",
                             "RequestName result != 1", TRUE);
        print_debug_ok(debug);

        // create the Volume object
        print_debug("Preparing data...", debug);
        status = g_object_new(VOLUME_TYPE_OBJECT, NULL);
        if (status == NULL)
                handle_error("Failed to create one VolumeObject instance.",
                             "Unknown(OOM?)", TRUE);

        status->debug = debug;
        status->timeout = timeout;
        status->settings = settings;

        // volume icons
        status->icon_high = gdk_pixbuf_new_from_file(i_high, &error);
        if (error != NULL)
                handle_error("Couldn't load volume_high_dark.svg.", error->message, TRUE);

        status->icon_medium = gdk_pixbuf_new_from_file(i_medium, &error);
        if (error != NULL)
                handle_error(concat("Couldn't load ", i_medium), error->message, TRUE);

        status->icon_low = gdk_pixbuf_new_from_file(i_low, &error);
        if (error != NULL)
                handle_error("Couldn't load volume_low_dark.svg.", error->message, TRUE);

        status->icon_off = gdk_pixbuf_new_from_file(i_off, &error);
        if (error != NULL)
                handle_error("Couldn't load volume_off_dark.svg.", error->message, TRUE);

        status->icon_muted = gdk_pixbuf_new_from_file(i_muted, &error);
        if (error != NULL)
                handle_error("Couldn't load volume_muted_dark.svg.", error->message, TRUE);

        status->icon_bright = gdk_pixbuf_new_from_file(i_brightness, &error);
        if (error != NULL)
                handle_error("Couldn't load display-brightness-dark.svg", error->message, TRUE);

        // progress bar
        status->image_progressbar_empty = gdk_pixbuf_new_from_file(pb_empty, &error);
        if (error != NULL)
                handle_error("Couldn't load progressbar_empty_dark.svg.", error->message, TRUE);

        status->image_progressbar_full = gdk_pixbuf_new_from_file(pb_full, &error);
        if (error != NULL)
                handle_error("Couldn't load progressbar_full_dark.svg.", error->message, TRUE);

        // check that the images are of the same size
        if (gdk_pixbuf_get_width(status->image_progressbar_empty) != gdk_pixbuf_get_width(status->image_progressbar_full) ||
            gdk_pixbuf_get_height(status->image_progressbar_empty) != gdk_pixbuf_get_height(status->image_progressbar_full) ||
            gdk_pixbuf_get_bits_per_sample(status->image_progressbar_empty) != gdk_pixbuf_get_bits_per_sample(status->image_progressbar_full))
                handle_error("Progress bar images aren't of the same size or don't have the same number of bits per sample.", "Unknown(OOM?)", TRUE);

        // create pixbuf for combined image
        status->width_progressbar = gdk_pixbuf_get_width(status->image_progressbar_empty);
        status->height_progressbar = gdk_pixbuf_get_height(status->image_progressbar_empty);
        status->image_progressbar = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                                   TRUE,
                                                   gdk_pixbuf_get_bits_per_sample(status->image_progressbar_empty),
                                                   status->width_progressbar,
                                                   status->height_progressbar);

        print_debug_ok(debug);

        // register the Volume object
        print_debug("Registering volume object...", debug);
        dbus_g_connection_register_g_object(bus,
                                            VALUE_SERVICE_OBJECT_PATH,
                                            G_OBJECT(status));
        print_debug_ok(debug);

        // daemonize
        if (!no_daemon) {
                print_debug("Daemonizing...\n", debug);
                if (daemon(0, 0) != 0)
                        handle_error("failed to daemonize", "unknown", FALSE);
        }

        // Run forever
        print_debug("Running the main loop...\n", debug);
        g_main_loop_run(main_loop);
        return EXIT_FAILURE;
}
