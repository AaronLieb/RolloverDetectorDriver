#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

MODULE_AUTHOR("Aaron Lieberman");
MODULE_LICENSE("GPL");

int rd_major = 0;
int rd_minor = 0;

module_param(rd_major, int, S_IRUGO);
module_param(rd_minor, int, S_IRUGO);

char *MODULE_NAME = "rolldet";
char *KEYBOARD_MODULE_NAME = "i8042";

unsigned int NUM_DEVICES = 1;
unsigned int IRQ_LINE = 1;
dev_t dev;
int result;
int data;
int num_keys_down = 0;
int max_rollover = 0;
struct platform_device *dev_id;

struct set_element *ele;
struct list_head *pos, *tmp;

static LIST_HEAD(keys_pressed);

struct set_element {
  int key;
  struct list_head list;
};

static int key_event_handler(struct notifier_block *nblock, unsigned long code,
                             void *_param);

static struct notifier_block key_block = {
    .notifier_call = key_event_handler,
};

int key_event_handler(struct notifier_block *nblock, unsigned long code,
                      void *_param) {
  struct keyboard_notifier_param *param = _param;

  if (param->down) {
    struct set_element *key_element =
        kmalloc(sizeof(struct set_element), GFP_KERNEL);
    key_element->key = param->value;
    list_add(&key_element->list, &keys_pressed);

    num_keys_down = 0;

    list_for_each(pos, &keys_pressed) {
      ele = list_entry(pos, struct set_element, list);
      num_keys_down++;
    }

    if (num_keys_down > max_rollover)
      max_rollover = num_keys_down;

    printk(KERN_ALERT "Key down: %u\n", param->value);
  } else {
    int search_key = param->value;
    list_for_each_safe(pos, tmp, &keys_pressed) {
      ele = list_entry(pos, struct set_element, list);
      if (ele->key == search_key) {
        list_del(pos);
        kfree(ele);
      }
    }

    printk(KERN_ALERT "Key up: %u\n", param->value);
  }

  printk(KERN_ALERT "Max Rollover: %d\n", max_rollover);

  return IRQ_NONE;
}

int init_module(void) {

  // if (rd_major) {
  //   dev = MKDEV(rd_major, rd_minor);
  //   result = register_chrdev_region(dev, NUM_DEVICES, MODULE_NAME);
  // } else {
  //   result = alloc_chrdev_region(&dev, rd_minor, NUM_DEVICES, MODULE_NAME);
  //   rd_major = MAJOR(dev);
  // }
  // if (result < 0) {
  //   printk(KERN_ALERT "rolldet: can't get major %d\n", rd_major);
  //   return result;
  // }
  // printk(KERN_ALERT "rolldet: major: %d minor: %d\n", rd_major, rd_minor);

  // dev_id = platform_device_alloc(MODULE_NAME, -1);
  register_keyboard_notifier(&key_block);
  printk(KERN_ALERT "rolldet: Requesting Line...\n");
  return 0;
}

void cleanup_module(void) {

  // Find out how to prevent double-frees
  unregister_keyboard_notifier(&key_block);

  /* free device */
  // unregister_chrdev_region(dev, NUM_DEVICES);
  /* free interrupt handler */
  // free_irq(IRQ_LINE, dev_id);

  printk(KERN_ALERT "rolldet: Cleaning Driver...\n");
}
