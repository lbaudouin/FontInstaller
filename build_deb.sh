#!/bin/bash
cp FontInstall debian/usr/bin/fontinstall
dpkg-deb --build debian/
mv debian.deb fontinstall.deb
