/*
 * =====================================================================================
 *
 *       Filename:  pci_led.c
 *
 *    Description:  a led device via pci bus.
 *
 *        Version:  1.0
 *        Created:  2011年08月13日 14时48分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Liunx 
 *        Company:  
 *
 * =====================================================================================
 */
/* For crc32 */
#include <zlib.h>

#include "hw.h"
#include "pci.h"
#include "qemu-timer.h"
#include "net.h"
#include "loader.h"
#include "sysemu.h"
#include "iov.h"

#define PCI_VENDOR_ID_LIUNX             0xff00
#define PCI_DEVICE_ID_LIUNX_LED         0xff01
#define LED_PCI_REVID                   0x44

typedef struct PCILedState {
    PCIDevice dev;
    NICState *nic;
    NICConf conf;
    int pci_led_mmio_io_addr;

} PCILedState;

static void pci_led_reset(DeviceState *d)
{

}

static void pci_led_mmio_writeb(void *opaque, target_phys_addr_t addr, uint32_t val)
{
    // do nothing
}

static void pci_led_mmio_writew(void *opaque, target_phys_addr_t addr, uint32_t val)
{

}

static void pci_led_mmio_writel(void *opaque, target_phys_addr_t addr, uint32_t val)
{

}

static uint32_t pci_led_mmio_readb(void *opaque, target_phys_addr_t addr)
{
    fprintf(stdout, "pci_led_mmio_readb !\n");
    return 0x1F;
}

static uint32_t pci_led_mmio_readw(void *opaque, target_phys_addr_t addr)
{
    return 0;
}

static uint32_t pci_led_mmio_readl(void *opaque, target_phys_addr_t addr)
{
    return 0;
}

static CPUReadMemoryFunc * const pci_led_mmio_read[3] = {
    pci_led_mmio_readb,
    pci_led_mmio_readw,
    pci_led_mmio_readl,
};

static CPUWriteMemoryFunc * const pci_led_mmio_write[3] = {
    pci_led_mmio_writeb,
    pci_led_mmio_writew,
    pci_led_mmio_writel,
};

static int pci_led_init(PCIDevice *dev)
{
    PCILedState *s = DO_UPCAST(PCILedState, dev, dev);
    
    /* I/O handler for memory-mapped I/O */
    s->pci_led_mmio_io_addr =
        cpu_register_io_memory(pci_led_mmio_read, pci_led_mmio_write, s,
                               DEVICE_LITTLE_ENDIAN);
    pci_register_bar_simple(&s->dev, 1, 0x100, 0, s->pci_led_mmio_io_addr);
    
    return 0; 
}

static int pci_led_uninit(PCIDevice *dev)
{
    return 0;
}

static PCIDeviceInfo led_info = {
    .qdev.name  = "pci-led",
    .qdev.size  = sizeof(PCILedState),
    .qdev.reset = pci_led_reset,
//    .qdev.vmsd  = &vmstate_led,
    .init       = pci_led_init,
    .exit       = pci_led_uninit,
    .vendor_id  = PCI_VENDOR_ID_LIUNX,
    .device_id  = PCI_DEVICE_ID_LIUNX_LED,
    .revision   = LED_PCI_REVID,
    .class_id   = PCI_CLASS_OTHERS,
//    .class_id   = PCI_CLASS_NETWORK_ETHERNET,
    .qdev.props = (Property[]) {
        DEFINE_NIC_PROPERTIES(PCILedState, conf),
        DEFINE_PROP_END_OF_LIST(),
    }

};

static void led_register_devices(void)
{
    pci_qdev_register(&led_info);
}

device_init(led_register_devices)
