CC := gcc
RM := rm
SRCDIR := src
SOFADIR := ./sofa/20231011/c/src
BUILDDIR := build
SOFABUILDDIR := sofa_build
TARGET := calgen
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) $(SOFADIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(SOFADIR)/%,$(SOFABUILDDIR)/%,$(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o)))
CFLAGS := -fpic -std=c11
INC := -I $(SOFADIR)
LIB := -L lib -lm -lcalceph
DESTDIR := /usr/local/bin

$(TARGET): $(OBJECTS)
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB) -O0

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps -O0

$(SOFABUILDDIR)/%.o: $(SOFADIR)/%.$(SRCEXT)
	@mkdir -p $(SOFABUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $< -save-temps -O0

clean:
	@echo " $(RM) -r $(BUILDDIR) $(SOFABUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(SOFABUILDDIR) $(TARGET)

install:
	cp crank "$(DESTDIR)"

uninstall:
	rm $(DESTDIR)/$(TARGET)
