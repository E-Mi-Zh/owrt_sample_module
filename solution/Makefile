include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=solution
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/solution
  SUBMENU:=Other modules
  DEPENDS:=@TARGET_ramips
  TITLE:=Linki solution char driver
  AUTOLOAD:=$(call AutoLoad,60,solution)
  FILES:=$(PKG_BUILD_DIR)/solution.ko
endef

define KernelPackage/solution/description
  Example character driver for Linki.
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,solution))
