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
#include <stdio.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <unistd.h>

#include "common.h"
#include "gopt.h"

#include "value-client-stub.h"

static void print_usage(const char* filename, int failure) {
    g_print("Usage: %s [-hnm] [-0 <image>] [-1 <image>] [-2 <image>] [-3 <image>] [-4 <image>] <value>\n"
            "  -h\t--help\t\thelp\n"
            "  -v\t--verbose\tverbose\n"
            "  -n\t--nobar\t\tdo not display progress bar\n"
            "  -m\t--mute\t\tmuted\n\n"
            " Icon options:\n\n"
            "  -0\t--mute-icon\tchange mute icon\n"
            "  -1\t--off-icon\tchange off icon\n"
            "  -2\t--low-icon\tchange low icon\n"
            "  -3\t--med-icon\tchange medium icon\n"
            "  -4\t--high-icon\tchange high icon\n\n"
            "  <value>\t\tint 0-100\n\n"
            "  If \"-m\" is invoked, <value> is ignored.\n" 
            "  Icon options can be individually called and expect a full path the the image.\n"
            "  Volume images will be used if icon images are unspecified\n\n" , filename);
    if (failure)
        exit(EXIT_FAILURE);
    else
        exit(EXIT_SUCCESS);
}

int main(int argc, const char* argv[]) {
    void *options = gopt_sort(&argc, argv, gopt_start(
            gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help", "HELP")),
            gopt_option('m', 0, gopt_shorts('m'), gopt_longs("mute")),
            gopt_option('n', 0, gopt_shorts('n'), gopt_longs("noprogress")),
            gopt_option('0', GOPT_ARG, gopt_shorts('0'), gopt_longs("mute-icon")),
            gopt_option('1', GOPT_ARG, gopt_shorts('1'), gopt_longs("off-icon")),
            gopt_option('2', GOPT_ARG, gopt_shorts('2'), gopt_longs("low-icon")),
            gopt_option('3', GOPT_ARG, gopt_shorts('3'), gopt_longs("med-icon")),
            gopt_option('4', GOPT_ARG, gopt_shorts('4'), gopt_longs("high-icon")),
            gopt_option('v', GOPT_REPEAT, gopt_shorts('v'), gopt_longs("verbose"))));
    const gchar* muteicon;
    const gchar* officon;
    const gchar* lowicon;
    const gchar* medicon;
    const gchar* highicon;
    int help = gopt(options, 'h');
    int debug = gopt(options, 'v');
    int muted = gopt(options, 'm');
    int nopr = gopt(options, 'n');
    int micon = gopt_arg(options, '0', &muteicon);
    int oicon = gopt_arg(options, '1', &officon);
    int licon = gopt_arg(options, '2', &lowicon);
    int meicon = gopt_arg(options, '3', &medicon);
    int hicon = gopt_arg(options, '4', &highicon);
    gopt_free(options);

    if (help)
        print_usage(argv[0], FALSE);

    gint nobar = 0;
    if (nopr) {
        nobar = 1;
    }

    gint volume = -1;
    if (!muted) {
        if (argc != 2)
            print_usage(argv[0], TRUE);

        if (sscanf(argv[1], "%d", &volume) != 1)
            print_usage(argv[0], TRUE);

        if (volume > 100 || volume < 0)
            print_usage(argv[0], TRUE);
    }

    if (!micon) {
        muteicon = NULL;
    }

    if (!oicon) {
        officon = NULL;
    }

    if (!licon) {
        lowicon = NULL;
    }

    if (!meicon) {
        medicon = NULL;
    }

    if (!hicon) {
        highicon = NULL;
    }

    DBusGConnection *bus = NULL;
    DBusGProxy *proxy = NULL;
    GError *error = NULL;

    // initialize GObject
    g_type_init();

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
    proxy = dbus_g_proxy_new_for_name(bus,
                                      VALUE_SERVICE_NAME,
                                      VALUE_SERVICE_OBJECT_PATH,
                                      VALUE_SERVICE_INTERFACE);
    if (proxy == NULL)
        handle_error("Couldn't get a proxy for D-Bus",
                    "Unknown(dbus_g_proxy_new_for_name)",
                    TRUE);
    print_debug_ok(debug);

    print_debug("Sending volume...", debug);
    uk_ac_cam_db538_VolumeNotification_notify(proxy, volume, nobar, muteicon, officon, lowicon, medicon, highicon, &error);
    if (error !=  NULL) {
        handle_error("Failed to send notification", error->message, FALSE);
        g_clear_error(&error);
        return EXIT_FAILURE;
    }
    print_debug_ok(debug);

    return EXIT_SUCCESS;
}
