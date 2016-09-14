#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/string.h>

#define DRV_NAME		"tcs230"

#define TCS_DEFAULT_FREQ	500
#define TCS_DEFAULT_MODE	"clean"

#define TCS_MODE_MAX_BUFFER	10


enum {
	TCS_GPIO_INDEX_OE = 0,
	TCS_GPIO_INDEX_S0,
	TCS_GPIO_INDEX_S1,
	TCS_GPIO_INDEX_S2,
	TCS_GPIO_INDEX_S3,
	TCS_GPIO_INDEX_LAST
};


struct tcs_data {
	char s0 :1;
	char s1 :1;
	char s2 :1;
	char s3 :1;
	char oe :1;
	int gpios[5];
	int freq;
	char mode[TCS_MODE_MAX_BUFFER];
};

static struct tcs_data data;


static void set_freq(uint32_t freq)
{
	data.freq = freq;

	switch(data.freq) {
		case 500:
			data.s0 = 1;
			data.s1 = 1;
			break;
		case 100:
			data.s0 = 1;
			data.s1 = 0;
			break;
		case 10:
			data.s0 = 0;
			data.s1 = 1;
			break;
		default:
			data.s0 = 0;
			data.s1 = 0;
			break;
	}

	gpio_set_value(data.gpios[TCS_GPIO_INDEX_S0], data.s0);
	gpio_set_value(data.gpios[TCS_GPIO_INDEX_S1], data.s1);
}


static void set_mode(const char *mode)
{
	memcpy((char *)data.mode, (const char *)mode, strlen(mode));

	if(strcmp("clean", data.mode) == 0) {
		data.s2 = 1;
		data.s3 = 0;
		goto set_value;
	}

	if(strcmp("red", data.mode) == 0) {
		data.s2 = 0;
		data.s3 = 0;
		goto set_value;
	}

	if(strcmp("green", data.mode) == 0) {
		data.s2 = 1;
		data.s3 = 1;
		goto set_value;
	}

	if(strcmp("blue", data.mode) == 0) {
		data.s2 = 0;
		data.s3 = 1;
		goto set_value;
	}

set_value:
	gpio_set_value(data.gpios[TCS_GPIO_INDEX_S2], data.s2);
	gpio_set_value(data.gpios[TCS_GPIO_INDEX_S3], data.s3);
}


static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	if(strcmp(buff, "enable") == 0)
		data.oe = 0;
	else if(strcmp(buff, "disable") == 0)
		data.oe = 1;

	gpio_set_value(data.gpios[TCS_GPIO_INDEX_OE], data.oe);

	return count;
}
static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char* buff)
{
	int ret = 0;

	if(data.oe == 0)
		ret += sprintf(buff, "enabled");
	else
		ret += sprintf(buff, "disabled");

	return ret;
}
static DEVICE_ATTR_RW(enable);


static ssize_t freq_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	int freq = 0;

	sscanf(buff, "%d", &freq);

	if((freq != 500) || (freq != 100) || (freq != 10))
		pr_err("[%s]: %d frequency unknown (possible 500, 100, 10 kHz)", DRV_NAME, freq);
	else
		set_freq(freq);

	return count;
}
static ssize_t freq_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	return scnprintf(buff, PAGE_SIZE, "%d\n", data.freq);
}
static DEVICE_ATTR_RW(freq);


static ssize_t color_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
	char color[TCS_MODE_MAX_BUFFER] = {0x00};

	sscanf(buff, "%10s", color);
	set_mode(color);

	return count;
}
static ssize_t color_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	return scnprintf(buff, PAGE_SIZE, "%s\n", data.mode);
}
static DEVICE_ATTR_RW(color);


static ssize_t dump_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	return scnprintf(buff, PAGE_SIZE, "Mode: \t%s (s1: %d, s2: %d)\n \
					   Frequency: \t%d (s0: %d, s2: %d)\n \
					   Enabled: \t%s\n",
					   data.mode, data.s1, data.s2,
					   data.freq, data.s0, data.s1,
					   (data.oe == 0) ? "enabled" : "false");
}
static DEVICE_ATTR_RO(dump);


static struct attribute *tcs_attrs[] = {
	&dev_attr_dump.attr,
	&dev_attr_freq.attr,
	&dev_attr_enable.attr,
	&dev_attr_color.attr,
	NULL,
};


static struct attribute_group tcs_attrs_group = {
	.attrs = tcs_attrs,
};


static int parse_dt(struct platform_device *pdev)
{
	int i = 0;
	uint32_t freq;
	const char *mode;

	if(of_find_property(pdev->dev.of_node, "default-disable", NULL))
		data.oe = 0;
	else
		data.oe = 1;

	if(of_property_read_string(pdev->dev.of_node, "default-mode", &mode)) {
		set_mode(mode);
	} else {
		pr_warn("[%s]: Failed to obtain proper mode, setting default \"clean\" mode!\n", DRV_NAME);
		set_mode(TCS_DEFAULT_MODE);
	}

	if(of_property_read_u32(pdev->dev.of_node, "default-freq", &freq)) {
		set_freq(freq);
	} else {
		pr_warn("[%s]: Failed to obtain proper frequency, setting default \"500\" kHz!\n", DRV_NAME);
		set_freq(TCS_DEFAULT_FREQ);
	}

	if(of_gpio_named_count(pdev->dev.of_node, "gpios") < 5) {
		pr_err("[%s]: GPIO cannot be assigned\n", DRV_NAME);
		return -EINVAL;
	}

	for(i = 0; i < TCS_GPIO_INDEX_LAST; i++) {
		data.gpios[i] = of_get_named_gpio(pdev->dev.of_node, "gpios", i);
	}

	return 0;
}


static int init_gpio(void)
{
	int i = 0;
	int err = 0;

	err = gpio_request(data.gpios[TCS_GPIO_INDEX_OE], "oe");
	if(err < 0) {
		pr_err("[%s]: Failed to request gpio [%d]!\n", DRV_NAME, data.gpios[i]);
		return -EINVAL;
	}
	gpio_direction_output(data.gpios[TCS_GPIO_INDEX_OE], 1);

	for(i = 1; i < TCS_GPIO_INDEX_LAST; i++) {
		char name[2];
		snprintf(name, 2, "s%d", i);

		err = gpio_request(data.gpios[i], name);
		if(err < 0 ) {
			pr_err("[%s]: Failed to request gpio [%d]\n", DRV_NAME, data.gpios[i]);
			return -EINVAL;
		}

		gpio_direction_output(data.gpios[i], 0);
	}

	return 0;
}


static void free_gpio(void)
{
	int i;

	for(i = 0; i < TCS_GPIO_INDEX_LAST; i++)
		gpio_free(data.gpios[i]);
}


static int tcs_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = parse_dt(pdev);
	if(ret < 0)
		return ret;

	ret = init_gpio();
	if(ret < 0)
		return ret;

	ret = sysfs_create_group(&pdev->dev.kobj, &tcs_attrs_group);
	if(ret < 0) {
		pr_err("[%s]: Failed to create sysfs group!\n", DRV_NAME);
		goto free_gpios;
	}

	pr_info("[%s]: Probe sucessful!\n", DRV_NAME);
	return 0;

free_gpios:
	free_gpio();

	return ret;
}


static int tcs_remove(struct platform_device *pdev)
{
	free_gpio();

	sysfs_remove_group(&pdev->dev.kobj, &tcs_attrs_group);

	pr_info("[%s]: Remove sucessful!\n", DRV_NAME);
	return 0;
}


static const struct of_device_id tcs_id[] = {
	{ .compatible = "taos,tcs230" },
	{ },
};
MODULE_DEVICE_TABLE(of, tcs_id);


static struct platform_driver tcs_drv = {
	.probe = tcs_probe,
	.remove = tcs_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(tcs_id),
	},
};


static int __init tcs230_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&tcs_drv);
	if(ret < 0) {
		pr_err("[%s]: Failed to register platform driver! (errno = %d)\n", DRV_NAME, -ret);
		return ret;
	}

	pr_info("[%s]: Module initialized!\n", DRV_NAME);
	return 0;
}


static void __exit tcs230_exit(void)
{
	platform_driver_unregister(&tcs_drv);

	pr_info("[%s]: Module exit!\n", DRV_NAME);
}

module_init(tcs230_init);
module_exit(tcs230_exit);

MODULE_AUTHOR("Krzysztof Garczynski <krzysztof.garczynski@gmail.com>");
MODULE_DESCRIPTION("Training example of tcs230 driver");
MODULE_LICENSE("GPL");
