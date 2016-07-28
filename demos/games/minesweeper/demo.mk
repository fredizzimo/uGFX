DEMODIR = $(GFXLIB)/demos/applications/minesweeper

GFXINC +=   $(DEMODIR) \
			$(DEMODIR)/resources/romfs
			
GFXSRC +=	$(DEMODIR)/main.c \
			$(DEMODIR)/mines.c
			
DEFS += -DMINES_SHOW_SPLASH=TRUE
