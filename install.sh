#!/bin/bash
##################################################
#  Filename:  install.sh
#  By:  Matthew Evans
#  Ver:  081821
#  See LICENSE.md for copyright information.
##################################################
#  Installs libwtf
#  Run with --uninstall to remove
##################################################

##################################################
#  Script variables
##################################################
#  Default location to install to
DEFAULT_INSTALL_LOCATION="/usr/local/include"
#  Default folder to install to
INSTALL_FOLDER="libwtf"
#  File extensions to install
INSTALL_FILES=".hpp"

##################################################
#  Function to create a confirmation prompt
#  First argument used as display text
#  Accepts ENTER, "Y" or "y" then returns true
#  All other input returns false
##################################################
confirm_prompt()
{
    read -p "$1 [Y/n]? " CONFIRM_RESULT
    CONFIRM_RESULT=${CONFIRM_RESULT:-true}
    if [ "$CONFIRM_RESULT" = true -o "$CONFIRM_RESULT" = "Y" -o "$CONFIRM_RESULT" = "y" ]; then
        true
        return
    fi
    false
    return
}

##################################################
#  Parse arguments
##################################################
RUN_UNINSTALL="false"
RUN_WIP="false"
for ARGS in "$@"; do
    #  Check if uninstall flag passed
    if [ "$ARGS" = "--uninstall" ]; then
        RUN_UNINSTALL="true"
    fi
    #  Check if wip flag passed
    if [ "$ARGS" = "--wip" ]; then
        RUN_WIP="true"
    fi
done

##################################################
#  Start main script
##################################################
echo
echo "*** Libwtf Installer ***"
echo

#  Get install location
read -p "Install location [$DEFAULT_INSTALL_LOCATION]: " INSTALL_LOCATION
#  If install location null, use default
INSTALL_LOCATION=${INSTALL_LOCATION:-$DEFAULT_INSTALL_LOCATION}
echo

#  Get the scripts' location
CURRENT_DIR="$( cd "$( dirname "$0" )" && pwd )"

##########################
#  Run uninstall
##########################
if [ "$RUN_UNINSTALL" = true ]; then
    echo "Removing '$INSTALL_FOLDER' from '$INSTALL_LOCATION'..."
    rm -rf $INSTALL_LOCATION/$INSTALL_FOLDER
##########################
#  Run install
##########################
else
    echo "Installing '$INSTALL_FOLDER' to '$INSTALL_LOCATION'..."
    if [ ! -d "$INSTALL_LOCATION/$INSTALL_FOLDER" ]; then
        mkdir $INSTALL_LOCATION/$INSTALL_FOLDER
    fi
    #  Iterate over header filenames, creating symlinks for each
    for FILE in "$CURRENT_DIR"/*; do
        if [ -f "$FILE" ] && [[ "$FILE" =~ ".hpp" ]]; then
            echo "Installing '$(basename $FILE)'..."
            ln -s "$FILE" "$INSTALL_LOCATION/$INSTALL_FOLDER/$(basename $FILE)"
        fi
    done

    #  Install WIP files
    if [ "$RUN_WIP" = true ]; then
        if [ ! -d "$INSTALL_LOCATION/$INSTALL_FOLDER/wip" ]; then
            mkdir "$INSTALL_LOCATION/$INSTALL_FOLDER/wip"
        fi

        for FILE in "$CURRENT_DIR"/wip/*; do
            if [ -f "$FILE" ] && [[ "$FILE" =~ ".hpp" ]]; then
                echo "Installing 'wip/$(basename $FILE)'..."
                ln -s "$FILE" "$INSTALL_LOCATION/$INSTALL_FOLDER/wip/$(basename $FILE)"
            fi
        done
    fi
fi

echo "Done!"
echo
