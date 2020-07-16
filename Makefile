TOPTARGETS := all clean install run
SUBDIRS := $(wildcard */.)

all:
$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

pru_mcp3208_host: pru_mcp3208_host.c
	$(CC) -o $@ $^

clean:
	rm -f pru_mcp3208_host


.PHONY: $(TOPTARGETS) $(SUBDIRS)
