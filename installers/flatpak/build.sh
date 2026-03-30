#!/bin/bash
set -xe
flatpak-builder --force-clean --install-deps-from=flathub --repo=repo builddir com.draconpern.dovo.yml