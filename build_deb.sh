#!/bin/bash
#
# Debian builder for ASCIIpOrtal.
# This script eases the build of a debian package out of ASCIIpOrtal
# source code, by ssh'ing to a debian box.
# I (zorun) use it because I'm not on debian right now, and building a
# debian package from Arch is a pain...

usage () {
    echo "usage: $0 <address>"
    echo "    or $0 <user>@<address>"
    echo "Connects to the given host via ssh and builds a debian package."
    exit 1
}

debsrc=asciiportal_1.3.orig.tar.gz
srcdir=asciiportal-1.3-beta5-src
cmdfile=/tmp/build_deb_tmp

cat > "$cmdfile" <<EOF
mkdir asciiportal
cd asciiportal
rm -rf *
mv ../"$debsrc" .
tar xf "$debsrc"
cd "$srcdir"
make
EOF

send () {
    scp "$debsrc" "$1":.
}

build() {
    ssh "$1" "$(cat $cmdfile)"    
}

get () {
    echo "## TODO"
}


(( $# < 1 )) && usage

make debian-source

send "$1"
build "$1"
get "$1"