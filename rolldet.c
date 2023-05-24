#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("Aaron Lieberman");
MODULE_LICENSE("GPL");

char * MODULE_NAME = "rolldet";

int rd_major = 0;
int rd_minor = 0;

module_param(rd_major, int, S_IRUGO);
module_param(rd_minor, int, S_IRUGO);

unsigned int NUM_DEVICES = 1;
unsigned int IRQ_LINE = 1;
dev_t dev;
int result;
struct platform_device *dev_id;

int handler(void) {
  printk(KERN_ALERT "BUTTON PRESSED");
  return IRQ_HANDLED;
}

int init_module(void) {

 	if (rd_major) {
		dev = MKDEV(rd_major, rd_minor);
		result = register_chrdev_region(dev, NUM_DEVICES, MODULE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, rd_minor, NUM_DEVICES, MODULE_NAME);
		rd_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_ALERT "rolldet: can't get major %d\n", rd_major);
		return result;
	}
	printk(KERN_ALERT "rolldet: major: %d minor: %d\n", rd_major, rd_minor);

  dev_id = platform_device_alloc(MODULE_NAME, -1);
  result = request_irq(IRQ_LINE, (irq_handler_t)handler, IRQF_SHARED,
                           MODULE_NAME, dev_id);
  printk(KERN_ALERT "rolldet: Requesting Line...\n");
  return 0;
}

void cleanup_module(void) {

  // Find out how to prevent double-frees

  /* free device */
  unregister_chrdev_region(dev, NUM_DEVICES); 
  /* free interrupt handler */
  free_irq(IRQ_LINE, dev_id); 

  printk(KERN_ALERT "rolldet: Cleaning Driver...\n");
}
