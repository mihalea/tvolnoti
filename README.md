# tvolnoti


`tvolnoti` is a lightweight volume notification daemon for GNU/Linux and other POSIX operating systems. It is based on GTK+ and D-Bus and should work with any sensible window manager. The original aim was to create a volume notification daemon for lightweight window managers like LXDE or XMonad. It is known to work with a wide range of WMs, including GNOME, KDE, Xfce, LXDE, XMonad, i3 and many others. The source code is heavily based on the GNOME notification-daemon.

![tvolnoti-preview](img/montage.png)

Compared to the original tvolnoti, this fork adds some additional features such as theme support.


## Distribution packages


 Available for Arch Linux and derivates in Arch User Repository as [tvolnoti](https://aur.archlinux.org/packages/tvolnoti)

 ```
 $ yay -S tvolnoti
 ```

## Manual installation 

### Dependencies

You need the following libraries to compile `tvolnoti` yourself. Please
install them through the package manager of your distribution, or follow
installation instructions on the projects' websites.

 - [D-Bus](http://dbus.freedesktop.org)
 - [D-Bus Glib](http://dbus.freedesktop.org/releases/dbus-glib)
 - [GTK+ 2.0](http://www.gtk.org)
 - [GDK-Pixbuf 2.0](http://www.gtk.org)

You can compile it with standard `GCC`, with `make` and `pkg-config`
installed, and you will need `autoconf` and `automake` if you choose
to compile the Git version.

### Compilation from Git

Start by downloading the source code from GitHub:

```
$ git clone https://github.com/LightAir/tvolnoti.git
$ cd tvolnoti
```

Let Autotools create the configuration scripts:

```
$ ./prepare.sh
```

Then just follow the basic GNU routine:

```
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

You can have the `.tar.gz` source archive prepared simply by calling
a provided script:

```
$ ./package.sh
```

### Compilation from source archive


Download the `.tar.gz` source archive from the GitHub page, and then extract its contents by calling:

```
$ tar xvzf tvolnoti-*.tar.gz
```

Then just follow the basic GNU routine:

```
$ ./configure --prefix=/usr
$ make
$ sudo make install
```

## Running the application


Firstly, the daemon needs to be started:

```
$ tvolnoti
```

```
Usage: tvolnoti [arguments]
 -h		--help			help
 -v		--verbose		verbose
 -n		--no-daemon		do not daemonize

Configuration:
 -t <int>       --timeout <int>         notification timeout in seconds
 -a <float>	    --alpha <float>		    transparency level (0.0 - 1.0, default 0.80)
 -b <int>	    --border <int>		    border size in pixels
 -p <int>,<int>	--pos <int>,<int>		horizontal and vertical position
 -r <int>	    --corner-radius <int>   radius of the round corners in pixels (default 5)
 -T <string>	--theme <string>	    theme name
```

Once the daemon is running, you can run for example the following command to show a notification for volume level 25%

```
$ tvolnoti-show 25
```
```
Usage: tvolnoti-show [OPTION]... value

Positional parameters:
  <value>		int 0-100

Toggle parameters:
  -h	--help		    help
  -v	--verbose	    verbose
  -n	--nobar		    do not display progress bar
  -m	--mute		    muted
  -b	--brightness	show brightness icon

Icon options:
  -0	--mute-icon	    change mute icon
  -1	--off-icon  	change off icon
  -2	--low-icon  	change low icon
  -3	--med-icon  	change medium icon
  -4	--high-icon	    change high icon
  -s	--single-icon	use same icon for all values
  -x	--bright-icon	change brightness icon

  If "-m" is invoked, <value> is ignored.
  Icon options can be individually called and expect a full path the the image.
  Volume images will be used if icon images are unspecified
```

## Theming

Some parameters of the notifications can be changed through the
parameters of the daemon, however theme files are needed to access the full theming capabilities. Two example themes can be found in `/usr/share/tvolnoti/themes` after installation, or on GitHub as [light](res/themes/light/theme.conf) and [dark](res/themes/dark/theme.conf).

### Directory structure

Theme should be placed in `~/.config/tvolnoti/themes`. Each theme should have its own directory. Minimal configuration requires a `theme.conf` INI file within in.

### Properties

#### [General]
  * timeout (int) = time in seconds before the dialog disappears

#### [Style]
  * bg_color (hex color) = color of the background
  * corner_radius (int) = radius in pixels of the corners
  * border (int) = size in pixels of the border
  * pos_x (int) = horizontal position on the screen
  * pos_y (int) = vertical position on the screen
  * alpha (float) = opacity of the background between 0-1
  * horizontal (TRUE, FALSE) = enable or disable horizontal layout

#### [Icons]
  * high (path) = icon shown when value between 75 and 100
  * medium (path) = icon shown when value between 50 and 75
  * low (path) = icon shown when value between 25 and 50
  * off (path) = icon shown when value between 0 and 25
  * muted (path) = icon shown values value is 0
  * brightness (path) = icon shown when displaying brightness
  * progressbar_empty (path) = empty progressbar
  * progressbar_full (path) = full progressbar

### Examples

Using a horizontal layout is possible by making use of theme files, and setting `horizontal=TRUE`.

![tvolnoti-preview](img/horizontal.png)

For example, to display a notification with no progress bar and a custom icon. In general, you would most likely use this along with the `-m` flag as that does not take a value argument, but different icons can be specified.

```
$ tvolnoti-show -n -m -0 /usr/share/pixmaps/volnoti/media-eject.svg
```



To control brightness, with different icons for the varying levels:

```
$ tvolnoti-show -1 /usr/share/pixmaps/bright-off.png
                -2 /usr/share/pixmaps/bright-low.svg
                -3 /usr/share/pixmaps/bright-med.svg
                -4 /usr/share/pixmaps/bright-high.svg
                <value>
```

There is also the option to use a single icon for all values:

```
$ tvolnoti-show -s /usr/share/pixmaps/volnoti/display-brightness-dark.svg <value>
```


## Credits


 - Faenza Icon Set (tiheum.deviantart.com)
 - Notification-daemon (www.gnome.org)
 - Gopt (www.purposeful.co.uk/software/gopt)
