#!/usr/bin/make -f
# -*- makefile -*-
#
#  A small set of rules to help compiling pd-aubio
#
#  Note: simply running `make` should attempt to build this

WAFCMD=python waf

#WAFOPTS:=
# turn on verbose mode
#WAFOPTS += --verbose

all: build

checkwaf:
	@[ -f waf ] || make getwaf

getwaf:
	./scripts/get_waf.sh

expandwaf: getwaf
	[ -d wafilb ] || rm -fr waflib
	$(WAFCMD) --help > /dev/null
	mv .waf*/waflib . && rm -fr .waf*
	sed '/^#==>$$/,$$d' waf > waf2 && mv waf2 waf
	chmod +x waf && chmod -R go-w waflib

cleanwaf:
	rm -rf waf waflib .waf*

configure: checkwaf
	$(WAFCMD) configure $(WAFOPTS)

build: configure
	$(WAFCMD) build $(WAFOPTS)

install:
	# install
	$(WAFCMD) install $(WAFOPTS)

clean: checkwaf
	# optionnaly clean before build
	-$(WAFCMD) clean
