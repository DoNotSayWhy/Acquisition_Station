#!/bin/sh
# Start the program from this installation directory.
APP_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
export LD_LIBRARY_PATH="$(find /usr/local/x86libHS -type d 2>/dev/null | tr '\n' ':')${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export GTK_IM_MODULE=fcitx
export QT_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
cd "$APP_DIR" || exit 1
exec "$APP_DIR/Acquisition_Station" "$@"
