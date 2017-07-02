#
#   General makefile to make the entire package
#
MACHINE=$(shell uname -m)
ifeq ($(MACHINE), BePC)
	CPU = x86
else
	CPU = ppc
endif
#	set the directory where object files and binaries will be created
OBJ_DIR		:= obj.$(CPU)

all: 		portlib portman squares brexx

portlib:
		@echo " "
		@echo "Creating PortManager library, and copying to /boot/home/config/lib"
		-cd PortLib; make; copyattr -d $(OBJ_DIR)/libPortManager.so /boot/home/config/lib


portman:
		@echo " "
		@echo "Creating PortManager"
		-cd PortManager; make

squares:
		@echo " "
		@echo "Creating test application Squares"
		-cd Squares; make

brexx:
		@echo " "
		@echo "Creating BRexx itself"
		-cd src; make

install:
		@echo " "
		@echo "Installing BRexx executables"
		-kill PortManager
		-copyattr -d PortManager/$(OBJ_DIR)/PortManager /boot/home/config/bin
		-copyattr -d Squares/$(OBJ_DIR)/Squares /boot/home/config/bin
		-copyattr -d src/$(OBJ_DIR)/rx /boot/home/config/bin

uninstall:
		@echo " "
		@echo "Removing all evidence of BRexx"
		-kill PortManager
		-rm /boot/home/config/lib/libPortManager.so
		-rm /boot/home/config/bin/Squares
		-rm /boot/home/config/bin/rx
		-rm /boot/home/config/bin/PortManager

clean:
		@echo " "
		@echo "Deleting object directories (but not installed binaries)"
		-rm -r PortLib/$(OBJ_DIR)
		-rm -r PortManager/$(OBJ_DIR)
		-rm -r Squares/$(OBJ_DIR)
		-rm -r src/$(OBJ_DIR)

test:
		@echo " "
		@echo "Testing installation"
		@echo " "
		-squares.rexx

world:		uninstall clean all install test

archive:	clean
		@echo "Creating archive"
		-rm BeBRexx.zip
		-cd ..; zip -9ry BeBRexx.zip BeBRexx
		-mv ../BeBRexx.zip .
