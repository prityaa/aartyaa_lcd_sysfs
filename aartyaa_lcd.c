
/** aartyaa lcd iec driver code */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/of.h>

int foo;
int r = 0x20, y = 0xff, b = 0x0;
int display_off;

/** 1. new group */
#define __AARTYAA_LCD_ATTR(_name, _mode, _show, _store) {		\
		.attr = {						\
			.name = __stringify(_name),			\
			.mode = VERIFY_OCTAL_PERMISSIONS(_mode),	\
		},							\
		.show = _show,						\
		.store = _store,					\
}

struct aartyaa_lcd_data
{
	struct i2c_client *i2c_client;
	struct device *device;
	struct mutex mutex_lock;
};

/** write a byte to lcd */
static int aartyaa_lcd_write_bytes(const struct i2c_client  *t_client, 
			int command, char value)
{
	int flags = 0, read_write = 0, size = 2;
	union i2c_smbus_data temp;
	temp.byte = value;

	pr_debug("addr = %x, flags = %x, read_write = %x, command = %x, size = %x, temp = %x\n",
		t_client->addr, flags, read_write, command, size, temp.byte);

	return i2c_smbus_xfer(t_client->adapter, t_client->addr,
			flags, read_write, command, size, &temp);
}

/** sysfs to change colour of display */
static ssize_t aartyaa_lcd_show_ryb(struct device *dev, 
			struct device_attribute *attr,
                        char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	pr_debug("aartyaa_lcd_show : id = %x, name = %s\n",
		 client->addr, client->name);
	return sprintf(buf, "%02x%02x%02x", r,y,b);
}
 
static ssize_t aartyaa_lcd_store_ryb(struct device *dev, 
			struct device_attribute *attr,
                        const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	pr_debug("aartyaa_lcd_show : id = %x, name = %s\n",
		 client->addr, client->name);

	sscanf(buf, "%02x%02x%02x", &r, &y, &b);
	pr_debug("aartyaa_lcd_store : ryb= %x, %x, %x\n", r, y, b);
	
	aartyaa_lcd_write_bytes(client, 4, r);
	aartyaa_lcd_write_bytes(client, 3, y);
	aartyaa_lcd_write_bytes(client, 2, b);
		
	return count;
}

/** sysfs to make display off */
static ssize_t aartyaa_lcd_show_display_on_off(struct device *dev, 
			struct device_attribute *attr,
                        char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	pr_debug("aartyaa_lcd_show_display_off : id = %x, name = %s\n",
		 client->addr, client->name);
	return sprintf(buf, "%d", display_off);
}
 
static ssize_t aartyaa_lcd_store_display_on_off(struct device *dev, 
			struct device_attribute *attr,
                        const char *buf, size_t count)
{
	// struct i2c_client *client = to_i2c_client(dev);
        struct aartyaa_lcd_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->i2c_client;

	pr_debug("aartyaa_lcd_show_display_off : id = %x, name = %s\n",
		 client->addr, client->name);

	sscanf(buf, "%d", &display_off);
	pr_debug("aartyaa_lcd_store : using driver data display_off = %d", display_off);

	mutex_lock(&data->mutex_lock);
	if(!display_off) {
		aartyaa_lcd_write_bytes(client, 4, 0);
		aartyaa_lcd_write_bytes(client, 3, 0);
		aartyaa_lcd_write_bytes(client, 2, 0);
	} else {	
		aartyaa_lcd_write_bytes(client, 4, r);
		aartyaa_lcd_write_bytes(client, 3, y);
		aartyaa_lcd_write_bytes(client, 2, b);
	}
	mutex_unlock(&data->mutex_lock);
	return count;
}

/** sysfs to change colour of display */
static ssize_t aartyaa_lcd_show(struct device *dev, 
			struct device_attribute *attr,
                        char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	pr_debug("aartyaa_lcd_show : id = %x, name = %s\n",
		 client->addr, client->name);
	return sprintf(buf, "%02x%02x%02x", r,y,b);
}
 
static ssize_t aartyaa_lcd_store(struct device *dev, 
			struct device_attribute *attr,
                        const char *buf, size_t count)
{
	sscanf(buf, "%02x%02x%02x", &r, &y, &b);
	pr_debug("aartyaa_lcd_store : ryb= %x, %x, %x\n", 
		r, y, b);

	return count;
}

#if 0		    
static int aartyaa_lcd_detect(struct i2c_client *i2c_client, 
			struct i2c_board_info *board_info)
{
	pr_debug("aartyaa_lcd_detect : calling detect\n");
	return 0;
}


static int aartyaa_lcd_attach(struct i2c_client *i2c_client)
{
	pr_debug("aartyaa_lcd_attach : calling attach\n");
	return 0;
}
#endif

/* sysfs device entry lcd_data */
struct device_attribute aartyaa_lcd_attribute_lcd_on = 
	__AARTYAA_LCD_ATTR(lcd_on, 0664, aartyaa_lcd_show, aartyaa_lcd_store);

/* sysfs device entry rbg */
struct device_attribute aartyaa_lcd_attribute_lcd_rbg = 
	__AARTYAA_LCD_ATTR(lcd_rbg, 0664, aartyaa_lcd_show_ryb, aartyaa_lcd_store_ryb);

/** 3. actual thing */
struct device_attribute aartyaa_lcd_attribute_lcd_display = {
	.attr = {
		.name = "lcd_display",                            
		.mode = VERIFY_OCTAL_PERMISSIONS(S_IRUGO),
	},             
        .show   = aartyaa_lcd_show,                                                
        .store  = aartyaa_lcd_store,                                               
};

/** 4. actual thing */
struct device_attribute aartyaa_lcd_attribute_lcd_display_on_off = {
	.attr = {
		.name = "lcd_display_on_off",                            
		.mode = VERIFY_OCTAL_PERMISSIONS(0664),
	},             
        .show   = aartyaa_lcd_show_display_on_off,                                                
        .store  = aartyaa_lcd_store_display_on_off,                                               
};


static struct attribute *aartyaa_lcd_attrs[] = {
	&aartyaa_lcd_attribute_lcd_rbg.attr,	
	&aartyaa_lcd_attribute_lcd_on.attr,
	&aartyaa_lcd_attribute_lcd_display.attr,	
	&aartyaa_lcd_attribute_lcd_display_on_off.attr,	
	NULL,
};

static struct attribute_group aartyaa_lcd_attribute_group = {
	.attrs = aartyaa_lcd_attrs,
};



static int init_lcd(struct aartyaa_lcd_data *t_lcd_data)
{
	char command_response = 0;
	pr_debug("init_lcd : addr = %x\n",
			t_lcd_data->i2c_client->addr); 

	mutex_lock(&t_lcd_data->mutex_lock);	
	command_response |= (aartyaa_lcd_write_bytes(t_lcd_data->i2c_client, 0, 0) << 0);
	command_response |= (aartyaa_lcd_write_bytes(t_lcd_data->i2c_client, 1, 0) << 1);
	command_response |= (aartyaa_lcd_write_bytes(t_lcd_data->i2c_client, 8, 0xaa) << 2);
	mutex_unlock(&t_lcd_data->mutex_lock);	
	
	pr_debug("command_response = %x\n", command_response);
	return command_response;
}

static int aartyaa_lcd_probe(struct i2c_client *i2c_client, 
			const struct i2c_device_id *i2c_id)
{
	int ret  = 0;
	struct aartyaa_lcd_data *lcd_data;
	struct device *dev = &i2c_client->dev;
	
	pr_debug("aartyaa_lcd_probe : i2c_client->addr = %d, i2c_client_name = %s\n", 
		i2c_client->addr, i2c_client->name);
	pr_debug("aartyaa_lcd_probe : driver name = %s\n", 
			i2c_client->dev.driver->name);
	
	if (!i2c_check_functionality(i2c_client->adapter, 
				I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA)) {
		printk(KERN_INFO "aartyaa_lcd_probe : falied to check i2c fun\n");
		return -1;
	}
	
	if (!(lcd_data = devm_kzalloc(dev, sizeof(struct  aartyaa_lcd_data), 
					GFP_KERNEL)) )
		return -ENOMEM;
	
	lcd_data->i2c_client = i2c_client;
	//i2c_set_clientdata(i2c_client, lcd_data);
	dev_set_drvdata(&i2c_client->dev, lcd_data);
	mutex_init(&lcd_data->mutex_lock);
	
	if (init_lcd(lcd_data) ) {
		pr_debug("aartyaa_lcd_init : Device falied to connect\n");
		dev_dbg(&i2c_client->adapter->dev,
                        "aartyaa device not connected\n");
		return -1;
	}	

	sysfs_create_group(&dev->kobj, &aartyaa_lcd_attribute_group);
	dev_dbg(&i2c_client->adapter->dev, "aartyaa_lcd init succeccful\n");
	return ret;
}

static int  aartyaa_lcd_remove(struct i2c_client *client)
{
	
	//struct aartyaa_lcd_data *data = dev_get_drvdata(&client->dev);

	pr_debug("aartyaa_lcd_remove : calling remove\n");
	//device_unregister(data->device);
	sysfs_remove_group(&client->dev.kobj, &aartyaa_lcd_attribute_group);
	printk(KERN_INFO "aartyaa_lcd device removed\n");
	return 0;
}

static struct i2c_device_id aartyaa_lcd_id[] = {
	{"aartyaa_lcd", 0},
	{}
}; 
MODULE_DEVICE_TABLE(i2c, aartyaa_lcd_id);

static struct i2c_driver aartyaa_lcd_driver = {
	.driver = {
		.name = "aartyaa_lcd",
		.owner = THIS_MODULE,
	},
	.probe = aartyaa_lcd_probe,
	.remove = aartyaa_lcd_remove,
#if 0
	.attach_adapter = aartyaa_lcd_attach,
	.detect = aartyaa_lcd_detect,
#endif
	.id_table = aartyaa_lcd_id,
};

static int __init aartyaa_lcd_init(void)
{
	pr_debug("aartyaa_lcd_init : calling init\n");	
	return i2c_add_driver(&aartyaa_lcd_driver);
}

static void __exit aartyaa_lcd_exit(void)
{
	pr_debug("aartyaa_exit : exiting the driver\n");
	i2c_del_driver(&aartyaa_lcd_driver);
}

module_init(aartyaa_lcd_init);
module_exit(aartyaa_lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("prityaa");
MODULE_DESCRIPTION("aartyaa lcd driver");
