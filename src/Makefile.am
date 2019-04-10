
bin_PROGRAMS = tvfamily-gtk

tvfamily_gtk_SOURCES = main.c mainwindow.h profilesview.h splashview.h \
    profilesview.c splashview.c paths.h paths.c widgets.h widgets.c core.h \
    core.c requests.h requests.c mediasview.h mediasview.c newprofileview.h \
    newprofileview.c pictureview.h pictureview.c coretypes.h coretypes.c

tvfamily_gtk_CFLAGS = ${gtk_CFLAGS} ${libcurl_CFLAGS} ${jansson_CFLAGS} -g -O0
tvfamily_gtk_LDADD = ${gtk_LIBS} ${libcurl_LIBS} ${jansson_LIBS}
tvfamily_gtk_LDFLAGS = -g -O0

