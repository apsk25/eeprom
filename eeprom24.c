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
#include "eeprom24.h"
//#include "eeprom24.h"

struct i2c_client *client1;
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


int eeprom24_read()
{
//	int reg_addr=client1->addr;
//	u8 reg_addr[2];
	char data_write[10]="Amrit";
	char data_read[10];
	int number_bytes=sizeof(data_write);
	struct i2c_msg msgs[1] =
	       {
	          { 
			  client1->addr, 0, sizeof(u8)*number_bytes, (void *)data_write 
		  }
		};
	if(i2c_transfer(client1->adapter,msgs,1)<0)
		printk("\nTrasnfer Write failed\n");
	
	struct i2c_msg msgs2[1] =
	{	  
		  { 
			  client1->addr, I2C_M_RD, sizeof(u8)*number_bytes, (void *)data_read
		  },
	};
	if(i2c_transfer(client1->adapter,msgs2,1)<0)
		printk("\nTrasnfer Read failed\n");
	printk("\nAddress=0x%x\n",client1->addr);

	printk("\ndata read back=%s len=%d\n",data_read,strlen(data_read));
}

static int eeprom24_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct at24_platform_data *pdata;     
	struct device_node *of_node = dev->of_node;
	const struct i2c_device_id *id;
	const struct at24_chip_data *cdata;
	int err,val;
	printk("\nmy probe called\n");

	pdata=dev_get_platdata(dev);// return platform data associated with the device.
				    // It essentially does "return dev->platform_data"
        if(pdata==NULL)
	{
		pdata=devm_kzalloc(dev,sizeof(struct at24_platform_data),GFP_KERNEL);
		printk("\n Platform data is null");
		id = i2c_match_id(at24_ids, to_i2c_client(dev));
		/*
		 * 	 * The I2C core allows OF nodes compatibles to match against the
		 * 	 	 * I2C device ID table as a fallback, so check not only if an OF
		 * 	 	 	 * node is present but also if it matches an OF device ID entry.
		 * 	 	 	 	 */
		if (of_node && of_match_device(at24_of_match, dev))
		{
			cdata = of_device_get_match_data(dev);
			printk("\n111\n");
		}
		else if (id)
		{
			printk("\n2222\n");
			cdata = (void *)id->driver_data;
		}
		else
			printk("\n3333\n");
	//		cdata = acpi_device_get_match_data(dev);

		if (!cdata)
			return -ENODEV;

			printk("\n4444\n");
		pdata->byte_len = cdata->byte_len;
		pdata->flags = cdata->flags;
			printk("\n5555\n");
/*	
		if(device_property_present(dev,"read-only"))
			pdata->flags |= AT24_FLAG_READONLY;
		if(device_property_present(dev,"no-read-rollover"))
			pdata->flags |= AT24_FLAG_NO_RDROL;
*/
			printk("\n6666\n");
		err = device_property_read_u32(dev, "address-width", &val);
		if(!err)
		{
			switch(val){
				case 8:
					if (pdata->flags & AT24_FLAG_ADDR16)
						dev_warn(dev, "Override address width to be 8, while default is 16\n");

					pdata->flags &= ~AT24_FLAG_ADDR16;
					printk("\nWidth is 8 bits\n");
					break;
				case 16:
					pdata->flags |= AT24_FLAG_ADDR16;
					printk("\nWidth is 16 bits\n");
					break;
				default:
					dev_warn(dev, "Bad \"address-width\" property: %u\n", val);
				}
			
		}
		else
			printk("\n Platform Data\n");
	

		err = device_property_read_u32(dev, "size", &val);
		if (!err)
			pdata->byte_len = val;

		err = device_property_read_u32(dev, "pagesize", &val);
		if (!err) {
				pdata->page_size = val;
			  } 
		else {
			/*
			 * 		 * This is slow, but we can't know all eeproms, so we better
			 * 		 		 * play safe. Specifying custom eeprom-types via platform_data
			 * 		 		 		 * is recommended anyhow.
			 * 		 		 		 		 */
			pdata->page_size = 1;
		    }

		printk("\n Length of single value=%d\n",pdata->byte_len);
		printk("\n pdata->page_size=%d\n",pdata->page_size);
	}
	client1=client;
	eeprom24_read();
	return 0;
};


EXPORT_SYMBOL(eeprom24_read);
EXPORT_SYMBOL(client1);

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
