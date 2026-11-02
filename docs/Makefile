SPHINXBUILD ?= sphinx-build
SOURCEDIR = source
BUILDDIR = build/html

.PHONY: html clean

html:
	mkdir -p build/doxygen
	doxygen Doxyfile
	$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR) -W

clean:
	rm -rf build
