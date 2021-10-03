
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

static char *int_str;

/* [X1: point 1]
 * These are calls that provide general information about the kernel
 * plugin. They describe the kind of code-license, the author name,
 * and the description for the module.
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gregory Bolet");
MODULE_DESCRIPTION("LKP Exercise 3");

/* [X2: point 1]
 * This call sets up the int_str character pointer to be a module
 * parameter that gets passed on an 'insmod' (insert module) call.
 * This allows a user to preconfigure their kernel module at start up
 * by passing in some string. The first argument is the name of the
 * parameter, the second argument is the type of parameter (char pointer)
 * and the third specifies the file permissions as being: the read
 * permission for the file owner (S_IRUSR), the read permission for
 * the group owner of the file (S_IRGRP), and the read permission for
 * other users (S_IROTH). 
 */
module_param(int_str, charp, S_IRUSR | S_IRGRP | S_IROTH);

/* [X3: point 1]
 * This line simply adds a description for the module parameter 
 * string 'int_str'. Thi is so that anyone using the module 
 * can know what the module input parameter does.
 */
MODULE_PARM_DESC(int_str, "A comma-separated list of integers");

/* [X4: point 1]
 * This is a compile-time list initialization where we define the list
 * called 'mylist'. It creates a list_head variable called 'mylist' 
 * and initializes it.
 */
static LIST_HEAD(mylist);

/* [X5: point 1]
 * These are going to be the structs that represent an entry in our list.
 */
struct entry {
	int val;
	struct list_head list;
};

static int store_value(int val)
{
	/* [X6: point 10]
	 * Allocate a struct entry of which val is val
	 * and add it to the tail of mylist.
	 * Return 0 if everything is successful.
	 * Otherwise (e.g., memory allocation failure),
	 * return corresponding error code in error.h (e.g., -ENOMEM).
	 */

	// Allocate the new struct
	struct entry* new_node = kmalloc(sizeof(struct entry), GFP_KERNEL);

	// Handle error of failed kmalloc
	if(!new_node){
		return -ENOMEM;
	}

	// Add the new value
	new_node->val = val;

	// Add the entry before the specified head
	// thus making it the new tail element
	list_add_tail(&(new_node->list), &mylist);
	return 0;
}

static void test_linked_list(void)
{
	/* [X7: point 10]
	 * Print out value of all entries in mylist.
	 */
	// Pointer used to hold the current item 
	// while iterating
	struct entry *current_elem;

	// Iterate over the list
	list_for_each_entry(current_elem, &mylist, list) {
		printk(KERN_INFO "Adding Item: '%d'\n", current_elem->val);
	}
}


static void destroy_linked_list_and_free(void)
{
	/* [X8: point 10]
	 * Free all entries in mylist.
	 */
	// Pointer used to hold the current item 
	// while iterating
	struct entry *current_elem, *next;
	
	// Iterate and remove elements from the list
	list_for_each_entry_safe(current_elem, next, &mylist, list){
		list_del(&current_elem->list);
		kfree(current_elem);
	}
}


static int parse_params(void)
{
	int val, err = 0;
	char *p, *orig, *params;


	/* [X9: point 1]
	 * This is a kernel call to duplicate the int_str param into
	 * the 'params' variable. If the copied string 'params' is
	 * is empty (NULL ptr) or the string copy failed, then the 
	 * if-statement will trigger and exit the code with
	 * a memory error.
	 * We have to make a copy because strsep ends up modifying
	 * the string we loop over.
	 */
	params = kstrdup(int_str, GFP_KERNEL);
	if (!params)
		return -ENOMEM;
	orig = params;

	/* [X10: point 1]
	 * Here we iterate over the comma-separated characters in the
	 * string. We split the string up by the ',' character and get
	 * the substring that comes before the next comma. If there are 
	 * no more commas, strsep will return NULL and we stop looping
	 * over the string characters.
	 * The extracted sub-strings are stored in the 'p' pointer.
	 * We've also got a check to see if the extracted string itself
	 * is not the 0 character, otherwise we continue to the next
	 * iteration, this is done for null-terminated strings.
	 */
	while ((p = strsep(&params, ",")) != NULL) {
		if (!*p)
			continue;
		/* [X11: point 1]
		 * Here we use a kernel function that converts the given
		 * string 'p' to an integer and stores it into 'val'. 
		 * The '0' that gets passed in is the 'base' of the 
		 * integer. This '0' signifies to automatically determine
		 * the base of the value represented by the string. In
		 * effect, it will be a decimal number.
		 */
		err = kstrtoint(p, 0, &val);
		if (err)
			break;

		/* [X12: point 1]
		 * Here we make the call to store_value function which 
		 * will append the given integer value we extracted
		 * to the linked list. The function returns 0 on success
		 * so the if-statement is not triggered. Otherwise, on
		 * an error, the if-statement is triggered because all
		 * the error values are nonzero. On trigger, we just end
		 * the loop.
		 */
		err = store_value(val);
		if (err)
			break;
	}

	/* [X13: point 1]
	 * Here we free the string copy that was done to 'int_str'.
	 * We needed this extra variable 'orig' because 'params' is
	 * the copy, but our loop procedure above moves the ponter head
	 * and makes 'params' point at different places. 'orig' is made
	 * as a backup of the pointer starting location for 'params'. 
	 */
	kfree(orig);
	return err;
}

static void run_tests(void)
{
	/* [X14: point 1]
	 * Here we call our test_linked_list function which will
	 * print all the elements in the list.
	 */
	test_linked_list();
}

static void cleanup(void)
{
	/* [X15: point 1]
	 * Here we print out to the dmesg screen that we are cleaning
	 * up the module. We then call our destroy function that
	 * goes through our list and deletes each element.
	 * This cleanup function is necessary to make sure we free
	 * each allocation that was made for the list elements.
	 */
	printk(KERN_INFO "\nCleaning up...\n");

	destroy_linked_list_and_free();
}

static int __init ex3_init(void)
{
	int err = 0;

	/* [X16: point 1]
	 * This is the first step taken when we start the module. We
	 * check whether int_str is NULL (i.e: whether or not it was 
	 * passed in as a command line argument on the insmod call).
	 * If the module wasn't given the 'int_str' param, we abort 
	 * the program.
	 */
	if (!int_str) {
		printk(KERN_INFO "Missing \'int_str\' parameter, exiting\n");
		return -1;
	}

	/* [X17: point 1]
	 * This calls our parse_params function that will process the
	 * input string and populate our list. If an error is encountered,
	 * we jump to the out tag to skip running the tests.
	 */
	err = parse_params();
	if (err)
		goto out;

	/* [X18: point 1]
	 * If we don't have any string parsing errors, we continue and
	 * run our tests which simply prints out all the list items.
	 */
	run_tests();
out:
	/* [X19: point 1]
	 * After printing all the list items, we need to deallocate the
	 * memory that had been allocated in the parse_params() call.
	 * This cleanup loops through all the elements in the list and
	 * calls kfree on them to clean everything up.
	 */
	cleanup();
	return err;
}

static void __exit ex3_exit(void)
{
	/* [X20: point 1]
	 * This gets called when we 'rmmod' our module. The exit call
	 * does nothing and just immediately returns.
	 */
	return;
}

/* [X21: point 1]
 * This sets up the module by calling the initialization function
 * that we defined as 'ex3_init'.
 */
module_init(ex3_init);

/* [X22: point 1]
 * This tears down the module by calling the 'ex3_exit' function we
 * defined that should free any memory still in use at the end of 
 * this module's execution.
 */
module_exit(ex3_exit);
