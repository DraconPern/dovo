#!/bin/bash
set -xe
#flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install builddir com.draconpern.dovo.yml
flatpak-builder --force-clean --disable-updates --user --install-deps-from=flathub --repo=repo --install builddir com.draconpern.dovo.yml

