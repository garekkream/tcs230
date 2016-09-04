#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>

#define DRV_NAME		"tcs230"

struct tcs_data {
	char s0 :1;
	char s1 :1;
	char s2 :1;
	char s3 :1;
	char oe :1;
};

static struct tcs_data data;

static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{

	return 0;
}
static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char* buff)
{

	return 0;
}
static DEVICE_ATTR_RW(enable);


static ssize_t freq_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{

	return 0;
}
static ssize_t freq_show(struct device *dev, struct device_attribute *attr, char *buff)
{

	return 0;
}
static DEVICE_ATTR_RW(freq);


static ssize_t color_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{

	return 0;
}
static ssize_t color_show(struct device *dev, struct device_attribute *attr, char *buff)
{

	return 0;
}
static DEVICE_ATTR_RW(color);


static ssize_t dump_show(struct device *dev, struct device_attribute *attr, char *buff)
{

	return 0;
}
static DEVICE_ATTR_RO(dump);

static struct attribute *tcs_attrs[] = {
	&dev_attr_dump.attr,
	&dev_attr_freq.attr,
	&dev_attr_enable.attr,
	&dev_attr_color.attr,
	NULL,
};

static int tcs_probe(struct platform_device *pdev)
{
	pr_info("[%s]: Probe sucessful!\n", DRV_NAME);
	return 0;
}

static int tcs_remove(struct platform_device *pdev)
{

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
		pr_err("[%s]: Failed to register platform driver!\n", DRV_NAME);
		return -EINVAL;
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
