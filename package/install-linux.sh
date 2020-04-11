#!/bin/sh

plugin_name="scrab"
install_dir="/home/$USER/.config/obs-studio/plugins/$plugin_name"

echo "Uninstalling old plugin"
rm -r $install_dir/
echo "Installing plugin"
mkdir -p $install_dir/
mv ./$plugin_name $install_dir/
