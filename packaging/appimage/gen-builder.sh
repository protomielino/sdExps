#! /bin/sh

while getopts d: arg
do
    case $arg in
        d) dir="$OPTARG" ;;
        ?) printf "%s [-d <dir>] \n" "$0"
    esac
done

dir=${dir:-"/AppDir"}

cat <<EOF
version: 1
AppDir:
  path: $dir
  app_info:
    id: org.speed_dreams.SpeedDreams
    name: speed-dreams
    icon: icon
    version: $(git describe --tags --dirty)
    exec: usr/games/speed-dreams-2
    exec_args: \$@
  after_runtime: |
    cd "\$TARGET_APPDIR/lib/x86_64-linux-gnu/"
    ln -rs ../../runtime/compat/lib/x86_64-linux-gnu/libcrypt.so.1 libcrypt.so.1
    ln -rs ../../runtime/compat/lib/x86_64-linux-gnu/libcrypt.so.1.1.0 libcrypt.so.1.1.0
  apt:
    arch:
    - $(dpkg --print-architecture)
    allow_unauthenticated: true
    sources:
    - sourceline: deb [signed-by="/usr/share/keyrings/ubuntu-archive-keyring.gpg"]
        http://archive.ubuntu.com/ubuntu focal main universe
    - sourceline: deb [signed-by="/usr/share/keyrings/ubuntu-archive-keyring.gpg"]
        http://archive.ubuntu.com/ubuntu focal-updates main universe
    - sourceline: deb [signed-by="/usr/share/keyrings/ubuntu-archive-keyring.gpg"]
        http://security.ubuntu.com/ubuntu focal-security main universe
    include:
    - libc6:$(dpkg --print-architecture)
    - locales
    - libcrypt1
    - libenet7
    - libexpat1
    - libglu1-mesa
    - libglx0
    - libopengl0
    - libopenscenegraph160
    - libcurl4
    - libsdl2-2.0-0
    - libsdl2-mixer-2.0-0
    - librhash0
    - libpng16-16
    - libjpeg8
    - zlib1g
    - libminizip1
    - libopenal1
    - libplib1
    - bash
    - dash
    - perl
  files:
    include:
    - lib64/ld-linux-x86-64.so.2
    - usr/share/games/speed-dreams-2/*
    - usr/lib/games/speed-dreams-2/*
    - /usr/local/lib/libcjson.so*
    exclude:
    - usr/share/man
    - usr/share/doc/*/README.*
    - usr/share/doc/*/changelog.*
    - usr/share/doc/*/NEWS.*
    - usr/share/doc/*/TODO.*
AppImage:
  arch: $(uname -m)
  update-information: guess
EOF
