#!/bin/bash

TEST="$(dirname $0)"
ROOT="$TEST/.."
SRC="$ROOT/src"
COVINFO="coverage.info"
COVINFO_PATH="$TEST/$COVINFO"
COVDIR="cov"
COVDIR_PATH="$TEST/$COVDIR"
T=$SRC/tvfamily-gtk
DATA="$ROOT/data"
TVFAMILY_LOGO="$DATA/tvfamily.svg"
SERVER="localhost:8888"
SERVER_SCRIPT="$ROOT/../tvfamily/run.sh"
SERVER_DATA_DIR="$HOME/.tvfamily"
PROFILES_DIR="$SERVER_DATA_DIR/profiles"
SERVER_PID_FILE="$SERVER_DATA_DIR/tvfamily.pid"

# Clean coverage files
function clean_cov {
    rm -v $SRC/*.gcda
    rm -v $COVINFO_PATH
    rm -rv $COVDIR_PATH
}

# Create coverage files
function do_cov {
    lcov --capture --directory $SRC --output-file $COVINFO_PATH
    genhtml $COVINFO_PATH --output-directory $COVDIR_PATH
}

# Test of command line arguments and other small stuff
function test_args {
    $T -h
    $T --version
    # Wrong argument
    $T -l
    # No address
    $T
    # Remove permissions from tvfamily logo
    chmod 000 $TVFAMILY_LOGO
    $T -a $SERVER
    chmod 644 $TVFAMILY_LOGO
}

function start_client {
    $T -a $SERVER &
}

function press_enter {
    read -p "Press ENTER when finished..."
}

# Test without connection #1
function test_no_connection1 {
    start_client
    echo "1) Click the button 'New profile'"
    echo "2) Click the button 'Back'"
    echo "3) Click the button 'New profile'"
    echo "4) Write 'a' in the box 'New profile's name'"
    echo "5) Click the button 'Create'"
    echo "6) Click the button 'OK'"
    # TODO: Cannot create the profile: (null)
    echo "7) Click the button 'Back'"
    echo "8) Click the key 'F11'"
    echo "9) Click the key 'F11'"
    echo "10) Click the button 'X'"
    press_enter
}

# Test profile creation
function test_profile_management {
    mv "$DATA/off-black.svg" "$DATA/off-black.svg.orig"
    mv "$DATA/off-white.svg" "$DATA/off-white.svg.orig"
    start_client
    sleep 1
    $SERVER_SCRIPT -b
    echo "1) Click the button 'New profile'"
    echo "2) Click the button 'Create'"
    echo "3) Click the button 'OK'"
    echo "4) Write 'a' in the box 'New profile's name'"
    echo "5) Click the button 'Create'"
    echo "6) Click the button 'ON/OFF'"
    echo "7) Click the button 'No'"
    echo "8) Click the button 'ON/OFF'"
    echo "9) Click the button 'Yes'"
    press_enter
    mv "$DATA/off-black.svg.orig" "$DATA/off-black.svg"
    mv "$DATA/off-white.svg.orig" "$DATA/off-white.svg"
    
    echo 'noimage' > $TEST/test.png
    start_client
    echo "1) Click the button 'New profile'"
    echo "2) Write 'b' in the box 'New profile's name'"
    echo "3) Click the button 'Profile picture'"
    echo "4) Select the picture '$TEST/test.png'"
    echo "5) Click the button 'OK'"
    echo "6) Click the button 'Profile picture'"
    echo "7) Select a picture"
    echo "8) Click the button 'Zoom out'"
    echo "9) Click the button 'Zoom in' until max zoom"
    echo "10) Click the button '<-' until min pos"
    echo "11) Click the button '->' until max pos"
    echo "12) Click the button 'Up' until min pos"
    echo "13) Click the button 'Down' until max pos"
    echo "14) Click the button 'Create'"
    press_enter
    mv "$PROFILES_DIR/b.png" "$PROFILES_DIR/b.png.orig"
    echo 'hello' > "$PROFILES_DIR/b.png"
    echo "15) Click the button 'New profile'"
    echo "16) Click the button 'Back'"
    press_enter
    mv "$PROFILES_DIR/b.png.orig" "$PROFILES_DIR/b.png"
    echo "17) Click the button 'New profile'"
    echo "18) Click the button 'Back'"
    echo "19) Select 'b' profile"
    echo "20) Click the menu button"
    echo "21) Click the button 'Change picture'"
    echo "22) Click the button 'Back'"
    echo "23) Click the menu button"
    echo "24) Click the button 'Change picture'"
    echo "25) Click the button 'Save'"
    echo "26) Click the menu button"
    echo "27) Click the button 'Change picture'"
    echo "28) Click the button 'Profile picture'"
    echo "29) Select a picture"
    echo "30) Click the button 'Save'"
    echo "31) Click the menu button"
    echo "32) Click the button 'Settings'"
    echo "33) Click the menu button"
    echo "34) Click the button 'Change profile'"
    echo "35) Select profile 'b'"
    echo "36) Click the menu button"
    echo "37) Click the button 'Quit'"
    echo "38) Click the button 'No'"
    echo "36) Click the menu button"
    echo "37) Click the button 'Quit'"
    echo "38) Click the button 'Yes'"
    press_enter

    start_client
    echo "1) Click the menu button"
    echo "2) Select 'Delete profile'"
    echo "3) Click the button 'Yes'"
    echo "4) Click the button 'New profile'"
    echo "5) Click the button 'Profile picture'"
    echo "6) Select a picture (vertical)"
    echo "7) Click the button 'X'"
    press_enter
    rm $TEST/test.png
}

# Test last profile supression
function test_delete_last_profile {
    start_client
    echo "1) Select 'a' profile"
    echo "2) Click the menu button"
    echo "3) Select 'Delete profile'"
    echo "4) Click the button 'Yes'"
    echo "5) Click the button 'ON/OFF'"
    echo "6) Click the button 'Yes'"
    press_enter
    kill -SIGINT $(cat $SERVER_PID_FILE)
}

# Clean previous coverage result files
clean_cov

# Tests
test_args
test_no_connection1
test_profile_management
test_delete_last_profile

# Generate coverage files
do_cov

exit 0

