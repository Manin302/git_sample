/**
 * @file   led.c
 * @author Derek Molloy
 * @date   19 April 2015
 * @brief  A kernel module for controlling a simple LED (or any signal) that is connected to
 * a GPIO. It is threaded in order that it can flash the LED.
 * The sysfs entry appears at /sys/ebb/led49
 * @see http://www.derekmolloy.ie/
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>       // Required for the GPIO functions
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/kthread.h>    // Using kthreads for the flashing functionality
#include <linux/delay.h>      // Using this header for the msleep() function

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Derek Molloy");
MODULE_DESCRIPTION("A simple Linux LED driver LKM for the BBB");
MODULE_VERSION("0.1");

static unsigned int gpioLED = 76;           ///< Default GPIO for the LED is 49
module_param(gpioLED, uint, 0644);       ///< Param desc. S_IRUGO can be read/not changed
//module_param(gpioLED, uint, S_IWUGO|S_IRUGO);       ///< Param desc. S_IRUGO can be read/not changed
MODULE_PARM_DESC(gpioLED, " GPIO LED number (default=49)");     ///< parameter description

static char ledName[7] = "gpioXXX";          ///< Null terminated default string -- just in case
static int mode = 0;             ///< Default mode is flashing

static int ledOn = 0;

static int direction  = 0;

/** @brief A callback function to display the LED mode
 *  @param kobj represents a kernel object device that appears in the sysfs filesystem
 *  @param attr the pointer to the kobj_attribute struct
 *  @param buf the buffer to which to write the number of presses
 *  @return return the number of characters of the mode string successfully displayed
 */
static ssize_t mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   switch(mode){
      case 0: return sprintf(buf, "off\n");      
		break;
      case 1: return sprintf(buf, "on\n");
		break;
   }
 //  printk(KERN_INFO "gpio7-> cat value %s",buf);
}

/** @brief A callback function to store the LED mode using the enum above */
static ssize_t mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   sscanf( buf,"%du", mode);
   gpio_set_value(gpioLED, mode);       // Use the LED state to light/turn off the LED
   printk(KERN_INFO "gpio7->pull high \n");
   return count;
}
static struct kobj_attribute mode_attr = __ATTR(mode, 0644, mode_show, mode_store);

static ssize_t direction_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   switch(direction){
      case 0: return sprintf(buf, "in\n");       // Display the state -- simplistic approach
      case 1: return sprintf(buf, "out\n");
   }
}

/** @brief A callback function to store the LED mode using the enum above */
static ssize_t direction_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){

   if ( strncmp(buf, "in",count-1) == 0){
	direction = 0;	
   }
   else if ( strncmp(buf,"out",count-1) == 0){
	direction = 1;	
   }

   gpio_direction_output(gpioLED, direction);
   printk("-------------->pull high for cs\n");
   return count;
}

static struct kobj_attribute direction_attr = __ATTR(direction, 0644, direction_show, direction_store);

/** The ebb_attrs[] is an array of attributes that is used to create the attribute group below.
 *  The attr property of the kobj_attribute is used to extract the attribute struct
 */
static struct attribute *ebb_attrs[] = {
   &mode_attr.attr,                         
//   &direction_attr.attr,                   
   NULL,
};

/** The attribute group uses the attribute array and a name, which is exposed on sysfs -- in this
 *  case it is gpio49, which is automatically defined in the ebbLED_init() function below
 *  using the custom kernel parameter that can be passed when the module is loaded.
 */
static struct attribute_group attr_group = {
   .name  = ledName,                        // The name is generated in ebbLED_init()
   .attrs = ebb_attrs,                      // The attributes array defined just above
};

static struct kobject *ebb_kobj;            /// The pointer to the kobject

static int __init ebbLED_init(void){
   int result = 0;

   sprintf(ledName, "gpio%d", gpioLED);      // Create the gpio115 name for /sys/ebb/led49
   printk(KERN_INFO "gpio76 init\n");

   ebb_kobj = kobject_create_and_add("ebb", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
   if(!ebb_kobj){
      printk(KERN_ALERT "EBB LED: failed to create kobject\n");
      return -ENOMEM;
   }
   printk(KERN_INFO "gpio76 kobject created\n");
   // add the attributes to /sys/ebb/ -- for example, /sys/ebb/led49/ledOn
   result = sysfs_create_group(ebb_kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "EBB LED: failed to create sysfs group\n");
      kobject_put(ebb_kobj);                // clean up -- remove the kobject sysfs entry
      return result;
   }
   ledOn = true;
   gpio_request(gpioLED, "sysfs");          // gpioLED is 49 by default, request it
   gpio_direction_output(gpioLED, ledOn);   // Set the gpio to be in output mode and turn on
   gpio_export(gpioLED, true);  // causes gpio49 to appear in /sys/class/gpio
   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebbLED_exit(void){
   kobject_put(ebb_kobj);                   // clean up -- remove the kobject sysfs entry
   gpio_set_value(gpioLED, 0);              // Turn the LED off, indicates device was unloaded
   gpio_unexport(gpioLED);                  // Unexport the Button GPIO
   gpio_free(gpioLED);                      // Free the LED GPIO
}

/// This next calls are  mandatory -- they identify the initialization function
/// and the cleanup function (as above).
module_init(ebbLED_init);
module_exit(ebbLED_exit);
