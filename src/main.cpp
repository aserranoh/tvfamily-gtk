/*
main.cpp - This is the entry point to the application.

This file is part of tvfamily-gtk.

Copyright 2019 Antonio Serrano Hernandez

tvfamily-gtk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

tvfamily-gtk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tvfamily-gtk; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.
*/

#include <err.h>
#include <getopt.h>
#include <gtkmm/application.h>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "config.h"
#include "viewcontroller.h"

// Short options:
//   * h: help
//   * v: version
//   * a: server address
const char* OPTSTRING = "hva:";

// Print help message and exits
static void
print_help ()
{
    std::cout <<
"Usage: " PACKAGE_NAME " [options]\n"
"Options:\n"
"  -h, --help                  Show this message and exit.\n"
"  -v, --version               Show version information.\n"
"  -a ADDR, --address ADDR     Server address.\n\n"
"Report bugs to:\n"
"Antonio Serrano Hernandez (" PACKAGE_BUGREPORT ")"
        << std::endl;
    exit (0);
}

// Print version message and exits
static void
print_version ()
{
    std::cout <<
PACKAGE_STRING "\n"
"Copyright (C) 2019 Antonio Serrano\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
        << std::endl;
    exit (0);
}

// Parse the command line arguments
static void
parse_args (int argc, char **argv, std::string& server_address)
{
    struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"address", required_argument, 0, 'a'},
        {0, 0, 0, 0}
    };
    int o;

    server_address = "";
    do {
        o = getopt_long(argc, argv, OPTSTRING, long_opts, 0);
        switch (o) {
            case 'h':
                print_help ();
            case 'v':
                print_version ();
            case 'a':
                server_address = optarg;
                break;
            case '?':
                exit (1);
            default:
                break;
        }
    } while (o != -1);
    if (server_address == "") {
        errx (1, "error: missing argument --address");
    }
}

int
main (int argc, char *argv[])
{
    std::string server_address;

    // Parse the command line arguments.
    parse_args (argc, argv, server_address);

    // Create the Gtk Application and the MainWindow
    auto app = Gtk::Application::create ();
    ViewController controller (app, server_address);

    // Run the Gtk Application       
    return app->run (controller.get_window ());    
}

