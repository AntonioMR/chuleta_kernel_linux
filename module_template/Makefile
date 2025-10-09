# Generic Linux Kernel Module Makefile Template
# This Makefile provides a flexible template for building Linux kernel modules
# 
# Author: A.M.R.
# Date: $(shell date)
# License: GPL v2

# =============================================================================
# MODULE CONFIGURATION
# =============================================================================

# Module name (change this)
MODULE_NAME := my_kernel_module

# Object files that make up the module
# Add all your .c files here (without the .c extension)
obj-m += $(MODULE_NAME).o

# Uncomment for multi-file modules
# $(MODULE_NAME)-objs := file1.o file2.o file3.o

# =============================================================================
# KERNEL BUILD SYSTEM CONFIGURATION
# =============================================================================

# Kernel source directory
# This will automatically detect the running kernel version
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory (where this Makefile is located)
PWD := $(shell pwd)

# =============================================================================
# COMPILER FLAGS AND OPTIONS
# =============================================================================

# Extra compiler flags (optional)
# EXTRA_CFLAGS += -DDEBUG
# EXTRA_CFLAGS += -Wall -Wextra
# EXTRA_CFLAGS += -I$(PWD)/include

# =============================================================================
# BUILD TARGETS
# =============================================================================

# Default target - build the module
all:
	@echo "Building kernel module: $(MODULE_NAME)"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@echo "Build complete!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Clean complete!"

# Install the module (requires root privileges)
install: all
	@echo "Installing module $(MODULE_NAME)..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
	depmod -a
	@echo "Module installed successfully!"

# Load the module
load:
	@echo "Loading module $(MODULE_NAME)..."
	sudo insmod $(MODULE_NAME).ko
	@echo "Module loaded! Check with: lsmod | grep $(MODULE_NAME)"

# Unload the module
unload:
	@echo "Unloading module $(MODULE_NAME)..."
	sudo rmmod $(MODULE_NAME)
	@echo "Module unloaded!"

# Show module information
info:
	@echo "Module information:"
	modinfo $(MODULE_NAME).ko

# Check if module is loaded
status:
	@echo "Checking if $(MODULE_NAME) is loaded:"
	@lsmod | grep $(MODULE_NAME) || echo "Module not loaded"

# =============================================================================
# DEVELOPMENT HELPERS
# =============================================================================

# Quick test cycle: clean, build, unload old, load new
reload: clean all unload load
	@echo "Module reloaded successfully!"

# Build and load in one step
build-load: all load

# Unload and clean
unload-clean: unload clean

# =============================================================================
# DEBUGGING TARGETS
# =============================================================================

# Enable debug output during compilation
debug:
	@echo "Building with debug information..."
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS="-DDEBUG -g" modules

# Show verbose build output
verbose:
	@echo "Building with verbose output..."
	$(MAKE) -C $(KDIR) M=$(PWD) V=1 modules

# =============================================================================
# HELP AND INFORMATION
# =============================================================================

# Display help information
help:
	@echo "Available targets:"
	@echo "  all        - Build the kernel module (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install the module to the system"
	@echo "  load       - Load the module into the kernel"
	@echo "  unload     - Unload the module from the kernel"
	@echo "  reload     - Clean, build, unload old, and load new module"
	@echo "  info       - Show module information"
	@echo "  status     - Check if module is loaded"
	@echo "  debug      - Build with debug information"
	@echo "  verbose    - Build with verbose output"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build the module"
	@echo "  make load         # Load the module"
	@echo "  make reload       # Quick development cycle"
	@echo "  make clean        # Clean build files"

# =============================================================================
# PHONY TARGETS
# =============================================================================

# Declare phony targets (targets that don't create files with the same name)
.PHONY: all clean install load unload reload build-load unload-clean debug verbose help info status dmesg

# =============================================================================
# ADDITIONAL NOTES
# =============================================================================

# To use this Makefile:
# 1. Change MODULE_NAME to your actual module name
# 2. Add your source files to the obj-m or $(MODULE_NAME)-objs variables
# 3. Modify EXTRA_CFLAGS if you need special compilation flags
# 4. Run 'make' to build your module
# 5. Use 'make load' to load it and 'make unload' to remove it
#
# For multi-file modules, use this format:
# obj-m := mymodule.o
# mymodule-objs := main.o helper1.o helper2.o