
rpiz-gtk:
	$(MAKE) -C src/. rpiz-gtk

rpiz-cli:
	$(MAKE) -C src/. rpiz-cli

all: rpiz-cli rpiz-gtk

.PHONY: all

clean:
	$(MAKE) -C src/. clean
