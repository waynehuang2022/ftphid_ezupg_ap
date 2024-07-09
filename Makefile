# Makefile for Focal HID Tools
# Date: 2022/06/01
dir_ftphid_ezupg_ap := ftphid_ezupg_ap

.PHONY: all
all: 
	@for directory in $(dir_ftphid_ezupg_ap); \
	do							\
		$(MAKE) -C $$directory;	\
	done
		
.PHONY: clean
clean:
	@for directory in $(dir_ftphid_ezupg_ap); \
	do									\
		$(MAKE) clean -C $$directory;	\
	done

