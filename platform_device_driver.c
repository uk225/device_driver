#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "platform.h"


#define MAX_DEVICE 3

/*Device private data structure */


struct pcdev_private_data
{
	struct pcdev_platform_data pdata;
        char *buffer;
        dev_t dev_num;
        struct cdev cdev;
};

/*Driver private data structure*/

struct pcdrv_private_data
{
        int total_devices;
        dev_t device_num_base;
        struct class *class_pcd;
	struct device *device_pcd;
};



struct pcdrv_private_data pcdrv_data;

static ssize_t eep_read(struct file *file, char __user *buff, size_t len, loff_t *off);
static ssize_t eep_write(struct file *file, const char __user *buff, size_t len, loff_t *off);
static int eep_open(struct inode *inode, struct file *file);
static int eep_release (struct inode *inode, struct file *file);

static int eep_open(struct inode *inode, struct file *file)
{
	return 0;

}

static int eep_release (struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t eep_read(struct file *file, char __user *buff, size_t len, loff_t *off)
{
	return 0;
}

static ssize_t eep_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
	return 0;

}

static struct file_operations fops={
	.owner  =  THIS_MODULE,
	.open   =  eep_open,
	.write  =  eep_write,
	.read   =  eep_read,
	.release=  eep_release 


};

int pcd_platform_driver_probe(struct platform_device *pdev)
{
	struct pcdev_private_data *dev_data;
	struct pcdev_platform_data *pdata;
	int ret;
        /*get platform data*/	
	pr_info("probe function called\n");
	pdata=(struct pcdev_platform_data *)pdev->dev.platform_data;
	if(!pdata)
	{
		pr_info("theres no platform data available\n");
		ret = -EINVAL;
		goto out;
	}
	/* dynamically allocate memory for device private data */

	dev_data=kzalloc(sizeof(struct pcdev_private_data),GFP_KERNEL);
	if(!dev_data){
		pr_info("cannot allocate memory\n");
		ret = -ENOMEM;
		goto out;
	}
	/*set device private data structure*/
	pdev->dev.driver_data =  dev_data;

	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
        dev_data->pdata.serial_number = pdata->serial_number;
	
	pr_info("device size =%d\n",dev_data->pdata.size);
	pr_info("device serial number =%s\n",dev_data->pdata.serial_number);
	pr_info("device permission =%d\n",dev_data->pdata.perm);
	
	/*dyanamically allocate memory for the device buffer using size info from platform data*/
	
	dev_data->buffer=kzalloc(dev_data->pdata.size,GFP_KERNEL);
        if(!dev_data){
                pr_info("cannot allocate memory\n");
                ret = -ENOMEM;
                goto dev_data_free;
        }


	/*get device number*/
	dev_data->dev_num=pcdrv_data.device_num_base + pdev->id;

	/*do cdev init and cdev add*/
	cdev_init(&dev_data->cdev,&fops);
	
	dev_data->cdev.owner = THIS_MODULE;
	ret=cdev_add(&dev_data->cdev,dev_data->dev_num,1);
  	if(ret<0)
	{
		pr_err("cdev add failed\n");
		goto buffer_free;
	}

	/*create device file for detected platform device*/

	pcdrv_data.device_pcd=device_create(pcdrv_data.class_pcd,NULL,dev_data->dev_num,NULL,"pcdev-%d",pdev->id);
        

	if(IS_ERR(pcdrv_data.device_pcd))
	{
		pr_err("Device create failed\n");
		ret =PTR_ERR(pcdrv_data.device_pcd);
		goto cdev_del;
	}

	pcdrv_data.total_devices++;
	pr_info("The probe was successful\n");
	return 0;

cdev_del:
	cdev_del(&dev_data->cdev);

buffer_free:
	kfree(dev_data->buffer);
dev_data_free:
	kfree(dev_data);
out:
	pr_info("Device probe  failed\n");
	
	return ret;

}
int pcd_platform_driver_remove(struct platform_device *pdev)
{
	struct pcdev_private_data *dev_data;
	dev_data = pdev->dev.driver_data;

	device_destroy(pcdrv_data.class_pcd,dev_data->dev_num);
	

	cdev_del(&dev_data->cdev);	

	kfree(dev_data->buffer);
	kfree(dev_data);
	pcdrv_data.total_devices--;
	pr_info("memory released for device\n");
	return 0;
}

struct platform_driver pcd_platform_driver =
{
	.probe  = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
			.name = "pseudo-char-device"
		   }

};

//static int i;

static int __init pcd_platform_driver_init(void)
{
	
	int ret;

	ret = alloc_chrdev_region(&pcdrv_data.device_num_base,0,MAX_DEVICE,"pcdevs");
	if(ret < 0)
	{
	pr_err("Alloc chrdev failed\n");
	return ret;
	}
	
	/* create class under /sys/class */
	pcdrv_data.class_pcd = class_create(THIS_MODULE,"pcd_class");
	
        if (IS_ERR(pcdrv_data.class_pcd))
	{
	 pr_err("class creation failed\n");
	 unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICE);
	 return -1;
	}
	platform_driver_register(&pcd_platform_driver);
	pr_info("platform driver registerd successfully\n");
	return 0;	
}




static void __exit pcd_platform_driver_cleanup(void)
{


	platform_driver_unregister(&pcd_platform_driver);
	pr_info("platform driver unregisterd successfully\n");
	
	class_destroy(pcdrv_data.class_pcd);
        
	unregister_chrdev_region(pcdrv_data.device_num_base,MAX_DEVICE);
	
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_DESCRIPTION("device driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("UTKARSH KASHYAP");


