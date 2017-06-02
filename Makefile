
rpiz-cli:
	$(MAKE) -C src/. rpiz-cli

rpiz-gtk:
	$(MAKE) -C src/. rpiz-gtk

all: rpiz-cli rpiz-gtk

.PHONY: all

clean:
	$(MAKE) -C src/. clean
