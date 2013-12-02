#!/bin/sh
# -*- coding: utf-8, tab-width: 2 -*-

PKG_NEED='libasound2-dev'
# are there any pkgs not yet installed? if not, instantly succeed:
LANG=C dpkg-query --show $PKG_NEED 2>&1 | grep -qPe '\t$' || exit 0

APT_CMD="$(which apt-get aptitude 2>&1 | tail -n 1)"
[ -x "$APT_CMD" ] || { echo "E: unable to find apt-get" >&2; exit 1; }

SUDO_CMD="$(which sudo 2>&1 | tail -n 1)"
[ -x "$SUDO_CMD" ] || { echo "E: unable to find sudo" >&2; exit 1; }

"$SUDO_CMD" -E "$APT_CMD" install $PKG_NEED || exit $?

exit 0
