##############################################
# OpenWrt Makefile for minimal-ui program
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
PKG_NAME:=minimal-ui
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
define Package/minimal-ui
	SECTION:=utils
	DEPENDS:=+libuci
	CATEGORY:=Utilities
	TITLE:=MinimalUI -- to get minimal-ui
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
# the minimal-ui executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/minimal-ui/install
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_DIR) $(1)/www/styles
	$(INSTALL_DIR) $(1)/www/images
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/index.html $(1)/www/index.html
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/contact.html $(1)/www/contact.html
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcpdump_script.ash $(1)/www/cgi-bin/tcpdump_script.ash
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/interval.ash $(1)/www/cgi-bin/interval.ash
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/3g_config $(1)/www/cgi-bin/3g_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/3g_process $(1)/www/cgi-bin/3g_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sensor_config $(1)/www/cgi-bin/sensor_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sensor_process $(1)/www/cgi-bin/sensor_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/proxy_config $(1)/www/cgi-bin/proxy_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/proxy_process $(1)/www/cgi-bin/proxy_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test_config $(1)/www/cgi-bin/test_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test_process $(1)/www/cgi-bin/test_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_config $(1)/www/cgi-bin/wifi_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifi_process $(1)/www/cgi-bin/wifi_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ethernet_config $(1)/www/cgi-bin/ethernet_config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ethernet_process $(1)/www/cgi-bin/ethernet_process
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/overview $(1)/www/cgi-bin/overview
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/images/favicon.ico $(1)/www/images/favicon.ico
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/images/logo.png $(1)/www/images/logo.png
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/styles/style.css $(1)/www/styles/style.css
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/urldecode.sed $(1)/www/cgi-bin/urldecode.sed
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_CONF) ./resource/minimalui.ucidefs $(1)/etc/uci-defaults/99_add_minimalui_ucidefs
endef

define Package/minimal-ui/postinst
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
		(. /etc/uci-defaults/99_add_minimalui_ucidefs)
		rm -f /etc/uci-defaults/99_add_minimalui_ucidefs
	fi
exit 0
endef

define Package/minimal-ui/prerm
	#!/bin/sh
	# check if we are on real system
	if [ -z "$${IPKG_INSTROOT}" ]; then
		echo "Remove alerts from crontab"
		sed -i '/ifconfig/d' /etc/crontabs/root
	fi
exit 0
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,minimal-ui))
