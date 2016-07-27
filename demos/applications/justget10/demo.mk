DEMODIR = $(GFXLIB)/demos/applications/justget10

GFXINC +=   $(DEMODIR) \
			$(DEMODIR)/resources/romfs
			
GFXSRC +=	$(DEMODIR)/main.c \
			$(DEMODIR)/jg10.c
			
DEFS += -DJG10_SHOW_SPLASH=TRUE
