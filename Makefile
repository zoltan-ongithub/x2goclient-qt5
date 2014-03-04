#!/usr/bin/make -f

SRC_DIR=$(CURDIR)
CLIENT_DIR=$(SRC_DIR)/client_build
CLIENT_BINARY=$(CLIENT_DIR)/x2goclient

PLUGIN_DIR=$(SRC_DIR)/plugin_build
PLUGIN_BINARY=$(PLUGIN_DIR)/libx2goplugin.so

SHELL=/bin/bash

INSTALL_DIR=install -d -o root -g root -m 755
INSTALL_FILE=install -o root -g root -m 644
INSTALL_SYMLINK=ln -s -f
INSTALL_PROGRAM=install -o root -g root -m 755

RM_FILE=rm -f
RM_DIR=rmdir -p --ignore-fail-on-non-empty

DESTDIR=
PREFIX=/usr/local
ETCDIR=/etc/x2go
BINDIR=$(PREFIX)/bin
SHAREDIR=$(PREFIX)/share
MANDIR=$(SHAREDIR)/man
MOZPLUGDIR=$(PREFIX)/lib/mozilla/plugins
QMAKE_BINARY=qmake-qt4
LRELEASE_BINARY=lrelease-qt4

all: build

build: build_man build_pluginprovider
	$(MAKE) build_client
	$(MAKE) build_plugin

build_client:
	$(LRELEASE_BINARY) x2goclient.pro
	mkdir -p $(CLIENT_DIR) && cd $(CLIENT_DIR) && $(QMAKE_BINARY) QMAKE_CFLAGS="${CPPFLAGS} ${CFLAGS}" QMAKE_CXXFLAGS="${CPPFLAGS} ${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" ../x2goclient.pro
	cd $(CLIENT_DIR) && $(MAKE)

build_plugin:
	$(LRELEASE_BINARY) x2goclient.pro
	mkdir -p $(PLUGIN_DIR) && cd $(PLUGIN_DIR) && X2GO_CLIENT_TARGET=plugin $(QMAKE_BINARY) QMAKE_CFLAGS="${CPPFLAGS} ${CFLAGS}" QMAKE_CXXFLAGS="${CPPFLAGS} ${CXXFLAGS}" QMAKE_LFLAGS="${LDFLAGS}" ../x2goclient.pro
	cd $(PLUGIN_DIR) && $(MAKE)

build_pluginprovider:

build_man:
	${MAKE} -f Makefile.man2html build

clean: clean_client clean_plugin clean_man clean_pluginprovider
	find . -maxdepth 2 -name '*.o' -exec rm -vf {} + -type f
	find . -maxdepth 2 -name 'moc_*.cpp' -exec rm -vf {} + -type f
	find . -maxdepth 2 -name 'ui_*.h' -exec rm -vf {} + -type f
	find . -maxdepth 2 -name 'qrc_*.cpp' -exec rm -vf {} + -type f
	rm -f x2goclient
	rm -f x2goclient.tag
	rm -f txt/changelog
	rm -f txt/git-info

clean_client:
	rm -fr $(CLIENT_DIR)

clean_plugin:
	rm -fr $(PLUGIN_DIR)

clean_pluginprovider:

clean_man:
	make -f Makefile.man2html clean

install: install_client install_plugin install_man install_pluginprovider

install_client:
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR)/
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/applications
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/x2goclient/icons
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/icons/hicolor/128x128/apps
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/icons/hicolor/16x16/apps
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/icons/hicolor/64x64/apps
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/icons/hicolor/32x32/apps
	$(INSTALL_PROGRAM) $(CLIENT_DIR)/x2goclient $(DESTDIR)$(BINDIR)/x2goclient
	$(INSTALL_FILE) desktop/x2goclient.desktop    $(DESTDIR)$(SHAREDIR)/applications/x2goclient.desktop
	$(INSTALL_FILE) icons/x2goclient.xpm          $(DESTDIR)$(SHAREDIR)/x2goclient/icons/x2goclient.xpm
	$(INSTALL_FILE) icons/128x128/x2goclient.png  $(DESTDIR)$(SHAREDIR)/x2goclient/icons/x2goclient.png
	$(INSTALL_FILE) icons/128x128/x2gosession.png $(DESTDIR)$(SHAREDIR)/x2goclient/icons/x2gosession.png
	$(INSTALL_FILE) icons/128x128/x2goclient.png  $(DESTDIR)$(SHAREDIR)/icons/hicolor/128x128/apps/x2goclient.png
	$(INSTALL_FILE) icons/16x16/x2goclient.png    $(DESTDIR)$(SHAREDIR)/icons/hicolor/16x16/apps/x2goclient.png
	$(INSTALL_FILE) icons/64x64/x2goclient.png    $(DESTDIR)$(SHAREDIR)/icons/hicolor/64x64/apps/x2goclient.png
	$(INSTALL_FILE) icons/32x32/x2goclient.png    $(DESTDIR)$(SHAREDIR)/icons/hicolor/32x32/apps/x2goclient.png

install_plugin:
	$(INSTALL_DIR) $(DESTDIR)$(MOZPLUGDIR)/
	$(INSTALL_PROGRAM) $(PLUGIN_DIR)/libx2goplugin.so $(DESTDIR)$(MOZPLUGDIR)/libx2goplugin.so

install_pluginprovider:
	$(INSTALL_DIR) $(DESTDIR)$(ETCDIR)/plugin-provider
	$(INSTALL_FILE) provider/etc/x2goplugin-apache.conf $(DESTDIR)$(ETCDIR)/x2goplugin-apache.conf
	$(INSTALL_FILE) provider/share/x2goplugin.html $(DESTDIR)$(ETCDIR)/plugin-provider/x2goplugin.html
	$(INSTALL_DIR) $(DESTDIR)$(SHAREDIR)/x2go/plugin/
	$(INSTALL_SYMLINK) ../../../../$(ETCDIR)/plugin-provider/x2goplugin.html $(DESTDIR)$(SHAREDIR)/x2go/plugin/x2goplugin.html

install_man:
	$(INSTALL_DIR) $(DESTDIR)$(MANDIR)/
	$(INSTALL_DIR) $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_FILE) man/man1/x2goclient.1    $(DESTDIR)$(MANDIR)/man1/x2goclient.1
	gzip -f $(DESTDIR)$(MANDIR)/man1/x2goclient.1

uninstall: uninstall_client uninstall_plugin uninstall_man

uninstall_client:
	$(RM_FILE) $(BINDIR)/x2goclient
	$(RM_FILE) $(SHAREDIR)/applications/x2goclient.desktop
	$(RM_FILE) $(SHAREDIR)/x2goclient/icons/x2goclient.png
	$(RM_FILE) $(SHAREDIR)/x2goclient/icons/x2goclient.xpm
	$(RM_FILE) $(SHAREDIR)/x2goclient/icons/x2gosession.png
	$(RM_FILE) $(SHAREDIR)/icons/hicolor/128x128/apps/x2goclient.png
	$(RM_FILE) $(SHAREDIR)/icons/hicolor/16x16/apps/x2goclient.png
	$(RM_FILE) $(SHAREDIR)/icons/hicolor/64x64/apps/x2goclient.png
	$(RM_FILE) $(SHAREDIR)/icons/hicolor/32x32/apps/x2goclient.png
	$(RM_DIR) $(SHAREDIR)/applications
	$(RM_DIR) $(SHAREDIR)/x2goclient/icons
	$(RM_DIR) $(SHAREDIR)/icons/hicolor/128x128/apps
	$(RM_DIR) $(SHAREDIR)/icons/hicolor/16x16/apps
	$(RM_DIR) $(SHAREDIR)/icons/hicolor/64x64/apps
	$(RM_DIR) $(SHAREDIR)/icons/hicolor/32x32/apps

uninstall_plugin:
	$(RM_FILE) $(MOZPLUGDIR)/libx2goplugin.so
	$(RM_DIR) $(MOZPLUGDIR)/

uninstall_pluginprovider:
	$(RM_FILE) $(ETCDIR)/x2goplugin-apache.conf
	$(RM_FILE) $(ETCDIR)/plugin-provider/x2goplugin.html
	$(RM_DIR) $(ETCDIR)/plugin-provider
	$(RM_DIR) $(ETCDIR)
	$(RM_FILE) $(SHAREDIR)/x2go/plugin/x2goplugin.html
	$(RM_DIR) $(SHAREDIR)/x2go/plugin/
	$(RM_DIR) $(SHAREDIR)/x2go/

uninstall_man:
	$(RM_FILE) $(MANDIR)/man1/x2goclient.1.gz
	$(RM_DIR) $(MANDIR)/man1
