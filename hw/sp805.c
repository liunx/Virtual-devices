/*
 * =====================================================================================
 *
 *       Filename:  sp805.c
 *
 *    Description:  watchdog for arm versatile
 *
 *        Version:  1.0
 *        Created:  2011年09月05日 11时04分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include "sysbus.h"

#define DEBUG_SP805 1

#ifdef DEBUG_SP805
#define DPRINTF(fmt, ...) \
do { printf("sp805: " fmt , ## __VA_ARGS__); } while (0)
#define BADF(fmt, ...) \
do { fprintf(stderr, "sp805: error: " fmt , ## __VA_ARGS__); exit(1);} while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#define BADF(fmt, ...) \
do { fprintf(stderr, "sp805: error: " fmt , ## __VA_ARGS__);} while (0)
#endif

/*
 * The kernel will detect the id, then chose a matched 
 * driver to do probing work.
 */
static const uint8_t sp805_id[8] =
  { 0x05, 0x18, 0x14, 0x00, 0x0d, 0xf0, 0x05, 0xb1 };

typedef struct {
    SysBusDevice busdev;
    uint32_t load;
    uint32_t value;
    uint8_t ctrl;
    uint8_t int_clr;
    uint8_t ris;
    uint8_t mis;
    uint32_t lock;
    uint8_t itcr;
    uint8_t itop;
    qemu_irq irq;
    qemu_irq out[8];
    const unsigned char *id;
} sp805_state;


static uint32_t sp805_read(void *opaque, target_phys_addr_t offset)
{
    sp805_state *s = (sp805_state *)opaque;

    DPRINTF("offset: 0x%x\n", offset);
    /* get peripheral id */
    if (offset >= 0xfe0 && offset < 0xffd) {
        return s->id[(offset - 0xfe0) >> 2];
    }

    switch (offset) {
    case 0x00: /* Load */
        return s->load;
    case 0x04: /* value */
        return s->value;
    case 0x08: /* control */
        return s->ctrl;
    case 0x0c: /* interrupt clear */
        return s->int_clr;
    case 0x10: /* raw interrupt status */
        return s->ris;
    case 0x14: /* mask interrupt status */
        return s->mis;
    case 0xc00:
	return s->lock;
    case 0xf00:
	return s->itcr;
    case 0xf04:
	return s->itop;
    default:
        hw_error("sp805_read: Bad offset %x\n", (int)offset);
        return 0;
    }
}

static void sp805_write(void *opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    sp805_state *s = (sp805_state *)opaque;
    uint8_t mask;

    DPRINTF("offset: 0x%x\n", offset);
    switch (offset) {
    case 0x00: /* Load */
        s->load = value;
	break;
    case 0x04: /* value */
        s->value = value;
	break;
    case 0x08: /* control */
        s->ctrl = value;
	break;
    case 0x0c: /* interrupt clear */
        s->int_clr = value;
	break;
    case 0x10: /* raw interrupt status */
        s->ris = value;
	break;
    case 0x14: /* mask interrupt status */
        s->mis = value;
	break;
    case 0xc00:
	s->lock = value;
	break;
    case 0xf00:
	s->itcr = value;
	break;
    case 0xf04:
	s->itop = value;
	break;
    default:
        hw_error("sp805_write: Bad offset %x\n", (int)offset);
    }
}

static void sp805_reset(sp805_state *s)
{
  s->lock = 0x00;
  s->ctrl = 0x00;
}

static void sp805_set_irq(void * opaque, int irq, int level)
{
    /* do nothing */
}

static CPUReadMemoryFunc * const sp805_readfn[] = {
   sp805_read,
   sp805_read,
   sp805_read
};

static CPUWriteMemoryFunc * const sp805_writefn[] = {
   sp805_write,
   sp805_write,
   sp805_write
};

static void sp805_save(QEMUFile *f, void *opaque)
{
    sp805_state *s = (sp805_state *)opaque;

    qemu_put_be32(f, s->load);
    qemu_put_be32(f, s->value);
    qemu_put_be32(f, s->ctrl);
    qemu_put_be32(f, s->int_clr);
    qemu_put_be32(f, s->ris);
    qemu_put_be32(f, s->mis);
    qemu_put_be32(f, s->lock);
    qemu_put_be32(f, s->itcr);
    qemu_put_be32(f, s->itop);
}

static int sp805_load(QEMUFile *f, void *opaque, int version_id)
{
    sp805_state *s = (sp805_state *)opaque;
    if (version_id != 1)
        return -EINVAL;

    s->load = qemu_get_be32(f);
    s->value = qemu_get_be32(f);
    s->ctrl = qemu_get_be32(f);
    s->int_clr = qemu_get_be32(f);
    s->ris = qemu_get_be32(f);
    s->mis = qemu_get_be32(f);
    s->lock = qemu_get_be32(f);
    s->itcr = qemu_get_be32(f);
    s->itop = qemu_get_be32(f);

    return 0;
}

static int sp805_init(SysBusDevice *dev, const unsigned char *id)
{
    int iomemtype;
    sp805_state *s = FROM_SYSBUS(sp805_state, dev);
    s->id = id;
    iomemtype = cpu_register_io_memory(sp805_readfn,
                                       sp805_writefn, s,
                                       DEVICE_NATIVE_ENDIAN);
    sysbus_init_mmio(dev, 0x1000, iomemtype);
    sysbus_init_irq(dev, &s->irq);
    qdev_init_gpio_in(&dev->qdev, sp805_set_irq, 8);
    qdev_init_gpio_out(&dev->qdev, s->out, 8);
    sp805_reset(s);
    register_savevm(&dev->qdev, "sp805_watchdog", -1, 1, sp805_save, sp805_load, s);
    return 0;
}

static int sp805_init_arm(SysBusDevice *dev)
{
    /*
     * keep the dev pointer,here, we can 
     * read/write to the device via this 
     * interface.
     */
    return sp805_init(dev, sp805_id);
}

static void sp805_register_devices(void)
{
    sysbus_register_dev("sp805", sizeof(sp805_state),
                        sp805_init_arm);
}

device_init(sp805_register_devices)
