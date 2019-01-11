.PHONY: default fmt all distclean fullclean
export GOPATH := $(shell pwd)

ifeq ($(GOARCH),)
export GOARCH := amd64
endif

CROSS_COMPILER ?= arm-linux-

ifneq ($(ENV_BUILDMODE),)
BUILDMODE = $(ENV_BUILDMODE)
else
BUILDMODE = debug
endif

BUILDFLAGS = -s -w

default: all

fmt:
	@for i in $(shell find src -mindepth 1 -maxdepth 1 -type d -exec basename {} \; | grep -v .com | grep -v assets_ | sort); do \
		go fmt $$i/... || exit -1; \
	done

notice: fmt
	@echo '=============================='
	@env | grep 'GO'
	@echo '=============================='
	@go list $@/...
	@echo '=============================='
	go get -tags '$(BUILDMODE) $(BUILDTAGS)' -d -v $@/...
	CC=$(if $(findstring arm,$(GOARCH)),"$(CROSS_COMPILER)gcc","") go install -tags '$(BUILDMODE) $(BUILDTAGS)' -ldflags '$(BUILDFLAGS)' $@

all: notice

.PHONY: release-notice
release-notice: BUILDMODE = release
release-notice: notice

distclean:
	@rm -rf bin pkg
	@go clean -i -r notice/... > /dev/null 2>&1; exit 0

fullclean:
	@rm -rf src/gitee.com src/github.com
