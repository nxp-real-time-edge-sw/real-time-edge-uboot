// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */

#include <common.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <netdev.h>
#include <net.h>
#include <linux/delay.h>

#define ipaddr		"192.168.1.1"
#define server_ip	"192.168.1.2"

struct pci_reg_info {
	const char *name;
	enum pci_size_t size;
	u8 offset;
};

static struct pci_reg_info regs_start[] = {
	{ "vendor ID", PCI_SIZE_16, PCI_VENDOR_ID },
	{ "device ID", PCI_SIZE_16, PCI_DEVICE_ID },
	{ "command register ID", PCI_SIZE_16, PCI_COMMAND },
	{ "status register", PCI_SIZE_16, PCI_STATUS },
	{ "revision ID", PCI_SIZE_8, PCI_REVISION_ID },
	{},
};

static struct pci_reg_info regs_rest[] = {
	{ "sub class code", PCI_SIZE_8, PCI_CLASS_SUB_CODE },
	{ "programming interface", PCI_SIZE_8, PCI_CLASS_PROG },
	{ "cache line", PCI_SIZE_8, PCI_CACHE_LINE_SIZE },
	{ "latency time", PCI_SIZE_8, PCI_LATENCY_TIMER },
	{ "header type", PCI_SIZE_8, PCI_HEADER_TYPE },
	{ "BIST", PCI_SIZE_8, PCI_BIST },
	{ "base address 0", PCI_SIZE_32, PCI_BASE_ADDRESS_0 },
	{},
};

static struct pci_reg_info regs_normal[] = {
	{ "base address 1", PCI_SIZE_32, PCI_BASE_ADDRESS_1 },
	{ "base address 2", PCI_SIZE_32, PCI_BASE_ADDRESS_2 },
	{ "base address 3", PCI_SIZE_32, PCI_BASE_ADDRESS_3 },
	{ "base address 4", PCI_SIZE_32, PCI_BASE_ADDRESS_4 },
	{ "base address 5", PCI_SIZE_32, PCI_BASE_ADDRESS_5 },
	{ "cardBus CIS pointer", PCI_SIZE_32, PCI_CARDBUS_CIS },
	{ "sub system vendor ID", PCI_SIZE_16, PCI_SUBSYSTEM_VENDOR_ID },
	{ "sub system ID", PCI_SIZE_16, PCI_SUBSYSTEM_ID },
	{ "expansion ROM base address", PCI_SIZE_32, PCI_ROM_ADDRESS },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "min Grant", PCI_SIZE_8, PCI_MIN_GNT },
	{ "max Latency", PCI_SIZE_8, PCI_MAX_LAT },
	{},
};

static struct pci_reg_info regs_bridge[] = {
	{ "base address 1", PCI_SIZE_32, PCI_BASE_ADDRESS_1 },
	{ "primary bus number", PCI_SIZE_8, PCI_PRIMARY_BUS },
	{ "secondary bus number", PCI_SIZE_8, PCI_SECONDARY_BUS },
	{ "subordinate bus number", PCI_SIZE_8, PCI_SUBORDINATE_BUS },
	{ "secondary latency timer", PCI_SIZE_8, PCI_SEC_LATENCY_TIMER },
	{ "IO base", PCI_SIZE_8, PCI_IO_BASE },
	{ "IO limit", PCI_SIZE_8, PCI_IO_LIMIT },
	{ "secondary status", PCI_SIZE_16, PCI_SEC_STATUS },
	{ "memory base", PCI_SIZE_16, PCI_MEMORY_BASE },
	{ "memory limit", PCI_SIZE_16, PCI_MEMORY_LIMIT },
	{ "prefetch memory base", PCI_SIZE_16, PCI_PREF_MEMORY_BASE },
	{ "prefetch memory limit", PCI_SIZE_16, PCI_PREF_MEMORY_LIMIT },
	{ "prefetch memory base upper", PCI_SIZE_32, PCI_PREF_BASE_UPPER32 },
	{ "prefetch memory limit upper", PCI_SIZE_32, PCI_PREF_LIMIT_UPPER32 },
	{ "IO base upper 16 bits", PCI_SIZE_16, PCI_IO_BASE_UPPER16 },
	{ "IO limit upper 16 bits", PCI_SIZE_16, PCI_IO_LIMIT_UPPER16 },
	{ "expansion ROM base address", PCI_SIZE_32, PCI_ROM_ADDRESS1 },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "bridge control", PCI_SIZE_16, PCI_BRIDGE_CONTROL },
	{},
};

static struct pci_reg_info regs_cardbus[] = {
	{ "capabilities", PCI_SIZE_8, PCI_CB_CAPABILITY_LIST },
	{ "secondary status", PCI_SIZE_16, PCI_CB_SEC_STATUS },
	{ "primary bus number", PCI_SIZE_8, PCI_CB_PRIMARY_BUS },
	{ "CardBus number", PCI_SIZE_8, PCI_CB_CARD_BUS },
	{ "subordinate bus number", PCI_SIZE_8, PCI_CB_SUBORDINATE_BUS },
	{ "CardBus latency timer", PCI_SIZE_8, PCI_CB_LATENCY_TIMER },
	{ "CardBus memory base 0", PCI_SIZE_32, PCI_CB_MEMORY_BASE_0 },
	{ "CardBus memory limit 0", PCI_SIZE_32, PCI_CB_MEMORY_LIMIT_0 },
	{ "CardBus memory base 1", PCI_SIZE_32, PCI_CB_MEMORY_BASE_1 },
	{ "CardBus memory limit 1", PCI_SIZE_32, PCI_CB_MEMORY_LIMIT_1 },
	{ "CardBus IO base 0", PCI_SIZE_16, PCI_CB_IO_BASE_0 },
	{ "CardBus IO base high 0", PCI_SIZE_16, PCI_CB_IO_BASE_0_HI },
	{ "CardBus IO limit 0", PCI_SIZE_16, PCI_CB_IO_LIMIT_0 },
	{ "CardBus IO limit high 0", PCI_SIZE_16, PCI_CB_IO_LIMIT_0_HI },
	{ "CardBus IO base 1", PCI_SIZE_16, PCI_CB_IO_BASE_1 },
	{ "CardBus IO base high 1", PCI_SIZE_16, PCI_CB_IO_BASE_1_HI },
	{ "CardBus IO limit 1", PCI_SIZE_16, PCI_CB_IO_LIMIT_1 },
	{ "CardBus IO limit high 1", PCI_SIZE_16, PCI_CB_IO_LIMIT_1_HI },
	{ "interrupt line", PCI_SIZE_8, PCI_INTERRUPT_LINE },
	{ "interrupt pin", PCI_SIZE_8, PCI_INTERRUPT_PIN },
	{ "bridge control", PCI_SIZE_16, PCI_CB_BRIDGE_CONTROL },
	{ "subvendor ID", PCI_SIZE_16, PCI_CB_SUBSYSTEM_VENDOR_ID },
	{ "subdevice ID", PCI_SIZE_16, PCI_CB_SUBSYSTEM_ID },
	{ "PC Card 16bit base address", PCI_SIZE_32, PCI_CB_LEGACY_MODE_BASE },
	{},
};

void pciinfo_header(int busnum, bool short_listing)
{
    printf("Scanning PCI devices on bus %d\n", busnum);

    if (short_listing) {
        printf("BusDevFun  VendorId   DeviceId   Device Class       Sub-Class\n");
        printf("_____________________________________________________________\n");
    }
}

static int pci_byte_size(enum pci_size_t size)
{
	switch (size) {
	case PCI_SIZE_8:
		return 1;
	case PCI_SIZE_16:
		return 2;
	case PCI_SIZE_32:
	default:
		return 4;
	}
}

static int pci_field_width(enum pci_size_t size)
{
	return pci_byte_size(size) * 2;
}

static void pci_show_regs(struct udevice *dev, struct pci_reg_info *regs)
{
	for (; regs->name; regs++) {
		unsigned long val;

		dm_pci_read_config(dev, regs->offset, &val, regs->size);
		printf("  %s =%*s%#.*lx\n", regs->name,
		       (int)(28 - strlen(regs->name)), "",
		       pci_field_width(regs->size), val);
	}
}

/**
 * pci_header_show() - Show the header of the specified PCI device.
 *
 * @dev: Bus+Device+Function number
 */
static void pci_header_show(struct udevice *dev)
{
	unsigned long class, header_type;

	dm_pci_read_config(dev, PCI_CLASS_CODE, &class, PCI_SIZE_8);
	dm_pci_read_config(dev, PCI_HEADER_TYPE, &header_type, PCI_SIZE_8);
	pci_show_regs(dev, regs_start);
	printf("  class code =                  0x%.2x (%s)\n", (int)class,
	       pci_class_str(class));
	pci_show_regs(dev, regs_rest);

	switch (header_type & 0x7f) {
	case PCI_HEADER_TYPE_NORMAL:	/* "normal" PCI device */
		pci_show_regs(dev, regs_normal);
		break;
	case PCI_HEADER_TYPE_BRIDGE:	/* PCI-to-PCI bridge */
		pci_show_regs(dev, regs_bridge);
		break;
	case PCI_HEADER_TYPE_CARDBUS:	/* PCI-to-CardBus bridge */
		pci_show_regs(dev, regs_cardbus);
		break;

	default:
		printf("unknown header\n");
		break;
    }
}

#ifdef CONFIG_DM_ETH
static const struct pci_flag_info {
	uint flag;
	const char *name;
} pci_flag_info[] = {
	{ PCI_REGION_IO, "io" },
	{ PCI_REGION_PREFETCH, "prefetch" },
	{ PCI_REGION_SYS_MEMORY, "sysmem" },
	{ PCI_REGION_RO, "readonly" },
	{ PCI_REGION_IO, "io" },
};

static void pci_show_regions(struct udevice *bus)
{
	struct pci_controller *hose = dev_get_uclass_priv(bus);
	const struct pci_region *reg;
	int i, j;

	if (!hose) {
		printf("Bus '%s' is not a PCI controller\n", bus->name);
		return;
	}

	printf("#   %-16s %-16s %-16s  %s\n", "Bus start", "Phys start", "Size",
	       "Flags");
	for (i = 0, reg = hose->regions; i < hose->region_count; i++, reg++) {
		printf("%d   %#016llx %#016llx %#016llx  ", i,
		       (unsigned long long)reg->bus_start,
		       (unsigned long long)reg->phys_start,
		       (unsigned long long)reg->size);
		if (!(reg->flags & PCI_REGION_TYPE))
			printf("mem ");
		for (j = 0; j < ARRAY_SIZE(pci_flag_info); j++) {
			if (reg->flags & pci_flag_info[j].flag)
				printf("%s ", pci_flag_info[j].name);
		}
		printf("\n");
	}
}

/**
 * pci_header_show_brief() - Show the short-form PCI device header
 *
 * Reads and prints the header of the specified PCI device in short form.
 *
 * @dev: PCI device to show
 */
static void pci_header_show_brief(struct udevice *dev)
{
	ulong vendor, device;
	ulong class, subclass;

	dm_pci_read_config(dev, PCI_VENDOR_ID, &vendor, PCI_SIZE_16);
	dm_pci_read_config(dev, PCI_DEVICE_ID, &device, PCI_SIZE_16);
	dm_pci_read_config(dev, PCI_CLASS_CODE, &class, PCI_SIZE_8);
	dm_pci_read_config(dev, PCI_CLASS_SUB_CODE, &subclass, PCI_SIZE_8);

	printf("0x%.4lx     0x%.4lx     %-23s 0x%.2lx\n",
	       vendor, device,
		pci_class_str(class), subclass);
}

static void pciinfo(struct udevice *bus, bool short_listing)
{
	struct udevice *dev;

	pciinfo_header(dev_seq(bus), short_listing);

	for (device_find_first_child(bus, &dev);
	dev;
	device_find_next_child(&dev)) {
		struct pci_child_plat *pplat;

		pplat = dev_get_parent_plat(dev);
		if (short_listing) {
			printf("%02x.%02x.%02x   ", dev_seq(bus),
			       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));
			pci_header_show_brief(dev);
		} else {
			printf("\nFound PCI device %02x.%02x.%02x:\n", dev_seq(bus),
			       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));
			pci_header_show(dev);
		}
	}
}

#else

/**
 * pci_header_show_brief() - Show the short-form PCI device header
 *
 * Reads and prints the header of the specified PCI device in short form.
 *
 * @dev: Bus+Device+Function number
 */
void pci_header_show_brief(pci_dev_t dev)
{
	u16 vendor, device;
	u8 class, subclass;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(dev, PCI_DEVICE_ID, &device);
	pci_read_config_byte(dev, PCI_CLASS_CODE, &class);
	pci_read_config_byte(dev, PCI_CLASS_SUB_CODE, &subclass);

	printf("0x%.4x     0x%.4x     %-23s 0x%.2x\n",
	       vendor, device,
	       pci_class_str(class), subclass);
}

/**
 * pciinfo() - Show a list of devices on the PCI bus
 *
 * Show information about devices on PCI bus. Depending on @short_pci_listing
 * the output will be more or less exhaustive.
 *
 * @bus_num: The number of the bus to be scanned
 * @short_pci_listing: true to use short form, showing only a brief header
 * for each device
 */
void pciinfo(int bus_num, int short_pci_listing)
{
	struct pci_controller *hose = pci_bus_to_hose(bus_num);
	int device;
	int function;
	unsigned char header_type;
	unsigned short vendor_id;
	pci_dev_t dev;
	int ret;

	if (!hose)
		return;

	pciinfo_header(bus_num, short_pci_listing);

	for (device = 0; device < PCI_MAX_PCI_DEVICES; device++) {
		header_type = 0;
		vendor_id = 0;
		for (function = 0; function < PCI_MAX_PCI_FUNCTIONS;
			function++) {
			/*
			 * If this is not a multi-function device, we skip
			 * the rest.
			 */
			if (function && !(header_type & 0x80))
				break;

			dev = PCI_BDF(bus_num, device, function);

			if (pci_skip_dev(hose, dev))
				continue;

			ret = pci_read_config_word(dev, PCI_VENDOR_ID,
						   &vendor_id);
			if (ret)
				goto error;
			if ((vendor_id == 0xFFFF) || (vendor_id == 0x0000))
				continue;

			if (!function) {
				pci_read_config_byte(dev, PCI_HEADER_TYPE,
						     &header_type);
			}

			if (short_pci_listing) {
				printf("%02x.%02x.%02x   ", bus_num, device,
				       function);
				pci_header_show_brief(dev);
			} else {
				printf("\nFound PCI device %02x.%02x.%02x:\n",
				       bus_num, device, function);
				pci_header_show(dev);
			}
		}
	}

	return;
error:
	printf("Cannot read bus configuration: %d\n", ret);
}
#endif

void test_pcie(void)
{
	struct udevice *dev, *bus;
	int busnum = 0;
	int ret = 0;

	/* Init PCIe bus */
	printf("\nPCIe controller Init ...\n");
	pci_init();

	ret = uclass_get_device_by_seq(UCLASS_PCI, busnum, &bus);
	if (ret) {
		printf("No such bus\n");
		return;
	}

#ifdef CONFIG_DM_ETH
	printf("\nPCIe controller regions:\n");
	pci_show_regions(bus);

	printf("\nPCIe bus information:\n");
	pciinfo(bus, 1);

	ret = uclass_get_device_by_seq(UCLASS_PCI, busnum + 1, &bus);
	if (ret) {
		printf("No such bus\n");
		return;
	}
	pciinfo(bus, 1);
#else
	printf("\nPCIe bus information:\n");
	pciinfo(busnum, 1);
	pciinfo(busnum + 1, 1);
	if (pci_eth_init(gd->bd) <= 0)
		printf("Not found Net device\n");
#endif
}
