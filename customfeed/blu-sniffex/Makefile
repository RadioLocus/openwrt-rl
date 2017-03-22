##############################################
# OpenWrt Makefile for helloworld program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=blu-sniffex
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
define Package/blu-sniffex
	SECTION:=utils
	DEPENDS:=+libpcap +libuci +libcurl +sensorid +libjson-c +libpthread +libpolarssl +libbluetooth
	CATEGORY:=Utilities
	TITLE:=Sniffex -- prints a snarky message
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one

# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/blu-sniffex/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sniffex $(1)/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./resource/bluetooth.cfg $(1)/etc/config/bluetooth
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./resource/sniffex.init $(1)/etc/init.d/sniffex
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_CONF) ./resource/sniffex.ucidefs $(1)/etc/uci-defaults/99_add_sniffex_ucidefs
endef

define Package/blu-sniffex/postinst
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
		(. /etc/uci-defaults/99_add_sniffex_ucidefs)
		rm -f /etc/uci-defaults/99_add_sniffex_ucidefs
fi
exit 0
endef

define Package/blu-sniffex/prerm
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
		/etc/init.d/sniffex stop
		echo "Remove sniffex from crontab"
		sed -i '/sniffex/d' /etc/crontabs/root
		echo "Removing monitor interface"
		uci delete wireless.mon0.mode
		uci delete wireless.mon0.device
		uci delete wireless.mon0.hidden
		uci delete wireless.mon0.network
		uci delete wireless.mon0.ifname
		uci delete wireless.mon0
		uci commit
		wifi down;wifi up

	fi
exit 0
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,blu-sniffex))
