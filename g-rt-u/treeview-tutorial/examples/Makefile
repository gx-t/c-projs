SUBDIRS = cell-renderer-spin       \
          custom-cell-renderer     \
          custom-list-model        \
          custom-list-model-sorted \
          hello-world              \
          simple-list              \
          treeview-demo

all: $(SUBDIRS)
	for dir in $(SUBDIRS); do\
	  cd $$dir && make && cd ..;\
	done; \

clean: $(SUBDIRS)
	for dir in $(SUBDIRS); do\
	  cd $$dir && make clean && cd ..;\
	done;
	rm -f `find -name "*~"` || /bin/true
