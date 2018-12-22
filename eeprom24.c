#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/log2.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include <linux/i2c.h>
#include <linux/nvmem-provider.h>
#include <linux/regmap.h>
#include <linux/platform_data/at24.h>
#include <linux/pm_runtime.h>
#include <linux/gpio/consumer.h>
/* I2C Device ID List */
struct at24_chip_data {
		/*
		 * 	 * these fields mirror their equivalents in
		 * 	 	 * struct at24_platform_data
		 * 	 	 	 */
		u32 byte_len;
		u8 flags;
};
#define AT24_CHIP_DATA(_name, _len, _flags)						\
				static const struct at24_chip_data _name = {		\
					.byte_len=_len, .flags=_flags,			\
				}

AT24_CHIP_DATA(at24_data_24c256, 262144 / 8, AT24_FLAG_ADDR16);

static const struct i2c_device_id at24_ids[] = {
	{ "24c256",	(kernel_ulong_t)&at24_data_24c256 },
	{ "at24",	0},
	{}
};
MODULE_DEVICE_TABLE(i2c, at24_ids);

static const struct of_device_id at24_of_match[] = {
        /* Generic Entry */
		{ .compatible = "atmel,24c256",		.data = &at24_data_24c256 },	
 		{}
	 	/* Terminating entry */
};
MODULE_DEVICE_TABLE(of, at24_of_match);


static int eeprom24_probe(struct i2c_client *client)
{
       // struct si470x_device *radio;
       // int retval = 0;
        printk("\nmy probe called\n");
        return 0;
};


static struct i2c_driver eeprom24_i2c_driver = {
        .driver = {
                .name           	 = "eeprom24",
                .of_match_table          = at24_of_match,
	},
	.probe_new 	        = eeprom24_probe,
        .id_table               =  at24_ids,
};

/**************************************************************************
 * Module Interface
 **************************************************************************/

/*
 * si470x_i2c_init - module init
 */
static int  eeprom24_i2c_init(void)
{
        printk("\n Insertion of I2c driver   \n");
        return i2c_add_driver(&eeprom24_i2c_driver);
}

/*
 * si470x_i2c_exit - module exit
 */
static void eeprom24_i2c_exit(void)
{
        i2c_del_driver(&eeprom24_i2c_driver);
}


module_init(eeprom24_i2c_init);
module_exit(eeprom24_i2c_exit);

MODULE_LICENSE("GPL");
