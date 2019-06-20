
AC_INIT([tvfamily-gtk], [0.1], [toni.serranoh@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
PKG_CHECK_MODULES([gtkmm], [gtkmm-3.0 >= 3.8.0])
PKG_CHECK_MODULES([libcurl], [libcurl])
PKG_CHECK_MODULES([jansson], [jansson])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

