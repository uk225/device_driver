#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include "platform.h"


void pcdev_release(struct device *dev)
{
	pr_info("device released\n");
}


struct pcdev_platform_data pcdev_platform_data_array[2]=
{
	[0] = {.size = 512,  .perm = RDWR, .serial_number= "AX1111"},
	[1] = {.size =1024 , .perm = RDWR, .serial_number= "AX1112"}

};

struct platform_device platform_pcddev_1 = {
	.name  = "pseudo-char-device",
	.id    = 0,
	.dev   = {
		.platform_data = &pcdev_platform_data_array[0],
		.release = pcdev_release
		}
}; 

struct platform_device platform_pcddev_2 = {

	.name = "pseudo-char-device",
	.id   = 1,
	.dev  = {
		.platform_data = &pcdev_platform_data_array[1],
		.release = pcdev_release
		
		}
};






static int __init pcdev_platform_init( void )
{

	
	platform_device_register(&platform_pcddev_1);
	platform_device_register(&platform_pcddev_2);
	pr_info("Device setup module inserted\n");
	return 0;
}

static void __exit pcdev_platform_exit( void )
{
	platform_device_unregister(&platform_pcddev_1);
	platform_device_unregister(&platform_pcddev_2);
	pr_info("device setup module removed\n");


}


module_init(pcdev_platform_init);
module_exit(pcdev_platform_exit);


MODULE_DESCRIPTION("Device Setup");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("UTKARSH KASHYAP");

