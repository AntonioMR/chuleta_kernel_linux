# Generic Linux Kernel Module Makefile Template with Cross-Compilation Support
# This Makefile provides a flexible template for building Linux kernel modules
# Supports native compilation and cross-compilation for multiple architectures
#
# Features:
# - Cross-compilation support (ARM, ARM64, MIPS, RISC-V, x86)
# - Configurable compiler and kernel paths
# - Automatic validation and error checking
# - Compatible with standard kernel build system (ARCH= CROSS_COMPILE=)
# 
# Author: A.M.R.
# Date: $(shell date)
# License: GPL v2
# Version: 0.1

# =============================================================================
# MAIN CONFIGURATION - MODIFY THESE VARIABLES FOR YOUR PROJECT
# =============================================================================

# -----------------------------------------------------------------------------
# MODULE CONFIGURATION
# -----------------------------------------------------------------------------
# Module name (can be set from command line: make MODULE=my_module)
MODULE ?= my_kernel_module

# Source files for multi-file modules (uncomment and modify if needed)
# $(MODULE)-objs := file1.o file2.o file3.o

# -----------------------------------------------------------------------------
# CROSS-COMPILATION CONFIGURATION
# -----------------------------------------------------------------------------
# Target architecture - uses standard ARCH variable (compatible with kernel build system)
# Can be set from command line: make ARCH=arm64
# Supported values: arm, arm64, x86, mips, riscv
ARCH ?= 

# Cross-compiler prefix - uses standard CROSS_COMPILE variable
# Can be set from command line: make CROSS_COMPILE=aarch64-linux-gnu-
# Leave empty for auto-detection based on ARCH
CROSS_COMPILE ?= 

# -----------------------------------------------------------------------------
# KERNEL SOURCE DIRECTORIES
# -----------------------------------------------------------------------------
# Default kernel directory (for native compilation)
KDIR_DEFAULT := /lib/modules/$(shell uname -r)/build

# Architecture-specific kernel directories
# Modify these paths according to your cross-compilation setup
# Common paths: /usr/src/linux-headers-*, /opt/cross/*, /home/user/kernels/*
KDIR_X86     := /lib/modules/$(shell uname -r)/build
KDIR_ARM     := /opt/kernels/arm-linux/
KDIR_ARM64   := /opt/kernels/arm64-linux/
KDIR_MIPS    := /opt/kernels/mips-linux/
KDIR_RISCV   := /opt/kernels/riscv-linux/

# -----------------------------------------------------------------------------
# COMPILER AND BUILD CONFIGURATION
# -----------------------------------------------------------------------------
# Extra compiler flags (add common useful flags by default)
EXTRA_CFLAGS ?= -Wall

# Debug flags (used with 'make debug')
DEBUG_FLAGS ?= -DDEBUG -g -Wall -Wextra

# Additional include directories
EXTRA_INCLUDES ?= 

# Module installation path (leave empty for default)
INSTALL_MOD_PATH ?= 

# -----------------------------------------------------------------------------
# DEFAULT CROSS-COMPILER DEFINITIONS
# -----------------------------------------------------------------------------
# Default cross-compiler prefixes (used only when CROSS_COMPILE is empty)
# Modify these according to your toolchain installation
DEFAULT_CROSS_COMPILE_ARM     := arm-linux-gnueabihf-
DEFAULT_CROSS_COMPILE_ARM64   := aarch64-linux-gnu-
DEFAULT_CROSS_COMPILE_MIPS    := mips-linux-gnu-
DEFAULT_CROSS_COMPILE_RISCV   := riscv64-linux-gnu-
DEFAULT_CROSS_COMPILE_X86     := 

# =============================================================================
# AUTOMATIC CONFIGURATION - DO NOT MODIFY BELOW THIS LINE
# =============================================================================

# Object files that make up the module
obj-m += $(MODULE).o

# =============================================================================
# KERNEL BUILD SYSTEM CONFIGURATION
# =============================================================================

# Auto-detect cross-compiler prefix based on architecture (only if not provided)
ifeq ($(CROSS_COMPILE),)
    ifeq ($(ARCH),arm)
        CROSS_COMPILE := $(DEFAULT_CROSS_COMPILE_ARM)
    else ifeq ($(ARCH),arm64)
        CROSS_COMPILE := $(DEFAULT_CROSS_COMPILE_ARM64)
    else ifeq ($(ARCH),mips)
        CROSS_COMPILE := $(DEFAULT_CROSS_COMPILE_MIPS)
    else ifeq ($(ARCH),riscv)
        CROSS_COMPILE := $(DEFAULT_CROSS_COMPILE_RISCV)
    else ifeq ($(ARCH),x86)
        CROSS_COMPILE := $(DEFAULT_CROSS_COMPILE_X86)
    endif
endif

# Auto-select kernel directory based on architecture
ifeq ($(ARCH),)
    KDIR := $(KDIR_DEFAULT)
else ifeq ($(ARCH),x86)
    KDIR := $(KDIR_X86)
else ifeq ($(ARCH),arm)
    KDIR := $(KDIR_ARM)
else ifeq ($(ARCH),arm64)
    KDIR := $(KDIR_ARM64)
else ifeq ($(ARCH),mips)
    KDIR := $(KDIR_MIPS)
else ifeq ($(ARCH),riscv)
    KDIR := $(KDIR_RISCV)
else
    KDIR := $(KDIR_DEFAULT)
endif

# Current directory (where this Makefile is located)
PWD := $(shell pwd)

# Set up cross-compilation environment
ifneq ($(CROSS_COMPILE),)
    export ARCH := $(ARCH)
    export CROSS_COMPILE := $(CROSS_COMPILE)
endif

# Add extra includes to CFLAGS if specified
ifneq ($(EXTRA_INCLUDES),)
    EXTRA_CFLAGS += $(addprefix -I,$(EXTRA_INCLUDES))
endif

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
all: validate-config
	@echo "Building kernel module: $(MODULE)"
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	@echo "Build complete!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Clean complete!"

# Install the module (requires root privileges)
install: all
	@echo "Installing module $(MODULE)..."
ifneq ($(INSTALL_MOD_PATH),)
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(INSTALL_MOD_PATH) modules_install
else
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
endif
	depmod -a
	@echo "Module installed successfully!"

# Load the module
load:
	@echo "Loading module $(MODULE)..."
	@if [ ! -f "$(MODULE).ko" ]; then \
		echo "Error: Module $(MODULE).ko not found. Run 'make' first."; \
		exit 1; \
	fi
	@if lsmod | grep -q "^$(MODULE) "; then \
		echo "Warning: Module $(MODULE) is already loaded."; \
		echo "Use 'make reload' to unload and reload, or 'make unload' first."; \
		exit 1; \
	fi
	sudo insmod $(MODULE).ko
	@echo "Module loaded successfully! Check with: lsmod | grep $(MODULE)"

# Unload the module
unload:
	@echo "Unloading module $(MODULE)..."
	@if ! lsmod | grep -q "^$(MODULE) "; then \
		echo "Warning: Module $(MODULE) is not currently loaded."; \
		echo "Use 'make status' to check loaded modules."; \
		exit 1; \
	fi
	sudo rmmod $(MODULE)
	@echo "Module unloaded successfully!"

# Show module information
info:
	@echo "Module information:"
	modinfo $(MODULE).ko

# Check if module is loaded
status:
	@echo "Checking if $(MODULE) is loaded:"
	@lsmod | grep $(MODULE) || echo "Module not loaded"

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
	$(MAKE) -C $(KDIR) M=$(PWD) EXTRA_CFLAGS="$(EXTRA_CFLAGS) $(DEBUG_FLAGS)" modules

# Show verbose build output
verbose:
	@echo "Building with verbose output..."
	$(MAKE) -C $(KDIR) M=$(PWD) V=1 modules

# =============================================================================
# UTILITY TARGETS
# =============================================================================

# Show current configuration
config:
	@echo "Current Configuration:"
	@echo "  MODULE          = $(MODULE)"
	@echo "  ARCH            = $(ARCH)"
	@echo "  CROSS_COMPILE   = $(CROSS_COMPILE)"
	@echo "  KDIR            = $(KDIR)"
	@echo "  EXTRA_CFLAGS    = $(EXTRA_CFLAGS)"
	@echo "  EXTRA_INCLUDES  = $(EXTRA_INCLUDES)"
	@echo "  PWD             = $(PWD)"

# Validate configuration before building
validate-config:
	@echo "Validating configuration..."
	@if [ ! -d "$(KDIR)" ]; then \
		echo "Error: Kernel directory $(KDIR) does not exist!"; \
		echo "Please check your KDIR_* variables in the configuration section."; \
		exit 1; \
	fi
	@if [ -n "$(ARCH)" ] && [ -z "$(CROSS_COMPILE)" ]; then \
		echo "Warning: ARCH is set but CROSS_COMPILE is empty."; \
		echo "Using default cross-compiler for $(ARCH): $(CROSS_COMPILE)"; \
	fi
	@echo "Configuration is valid!"


# =============================================================================
# STATIC ANALYSIS
# =============================================================================

# sa : "wrapper" target over the following kernel static analyzer targets
sa:
	make sa-sparse
	make sa-gcc
	make sa-flawfinder
	make sa-cppcheck

# static analysis with sparse
sa-sparse:
ifeq (,$(shell which sparse))
	$(error ERROR: install sparse first)
endif
	make clean
	@echo
	@echo "--- static analysis with sparse ---"
	@echo
	sparse -I$(KDIR)/include -I$(PWD) $(MODULE).c

# static analysis with gcc
sa-gcc:
	make clean
	@echo
	@echo "--- static analysis with gcc ---"
	@echo
	make W=1 -C $(KDIR) M=$(PWD) modules

# static analysis with flawfinder
sa-flawfinder:
ifeq (,$(shell which flawfinder))
	$(error ERROR: install flawfinder first)
endif
	make clean
	@echo
	@echo "--- static analysis with flawfinder ---"
	@echo
	flawfinder *.[ch]

# static analysis with cppcheck
sa-cppcheck:
ifeq (,$(shell which cppcheck))
	$(error ERROR: install cppcheck first)
endif
	make clean
	@echo
	@echo "--- static analysis with cppcheck ---"
	@echo
	cppcheck -v --force --enable=all -i .tmp_versions/ -i *.mod.c -i bkp/ --suppress=missingIncludeSystem .


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
	@echo "  config     - Show current configuration"
	@echo "  validate-config - Validate build configuration"
	@echo "  sa         - Run all static analysis checks"
	@echo "  sa-sparse  - Static analysis with sparse"
	@echo "  sa-gcc     - Static analysis with gcc"
	@echo "  sa-flawfinder - Static analysis with flawfinder"
	@echo "  sa-cppcheck - Static analysis with cppcheck"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Cross-compilation usage:"
	@echo "  make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-"
	@echo "  make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-"
	@echo "  make ARCH=mips CROSS_COMPILE=mips-linux-gnu-"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build for native architecture"
	@echo "  make MODULE=gpio_driver  # Build specific module"
	@echo "  make config       # Show current configuration"
	@echo "  make validate-config  # Check if configuration is valid"
	@echo "  make ARCH=arm64   # Cross-compile for ARM64 (auto-detect compiler)"
	@echo "  make MODULE=spi_sensor load  # Build and load specific module"
	@echo "  make load         # Load the module"
	@echo "  make reload       # Quick development cycle"
	@echo "  make clean        # Clean build files"
	@echo ""
	@echo "Configuration:"
	@echo "  Edit the top section of this Makefile to customize:"
	@echo "  - MODULE: Your module name"
	@echo "  - KDIR_*: Kernel source paths for each architecture"
	@echo "  - DEFAULT_CROSS_COMPILE_*: Default cross-compiler prefixes"

# =============================================================================
# PHONY TARGETS
# =============================================================================

# Declare phony targets (targets that don't create files with the same name)
.PHONY: all clean install load unload reload build-load unload-clean debug verbose \
		help info status config validate-config sa sa-sparse sa-gcc sa-flawfinder \
		sa-cppcheck

# =============================================================================
# ADDITIONAL NOTES
# =============================================================================

# To use this Makefile:
# 1. Change MODULE to your actual module name (or use: make MODULE=my_module)
# 2. Add your source files to the obj-m or $(MODULE)-objs variables
# 3. Modify EXTRA_CFLAGS if you need special compilation flags
# 4. Run 'make' to build your module
# 5. Use 'make load' to load it and 'make unload' to remove it
#
# For multi-file modules, use this format:
# obj-m := mymodule.o
# mymodule-objs := main.o helper1.o helper2.o