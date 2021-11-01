#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include "driver.h"

MODULE_LICENSE("GPL");


static int motor_round = 1;
typedef struct pir_queue{
    struct list_head list;
    unsigned long timestamp;
    int is_detected;
}pir_q;

pir_q pir_list;
static int q_len = 0;
static int irq_num_switch;
static int irq_num_pir;
static int switch_value;

int blue[8] =   {1, 1, 0, 0, 0, 0, 0, 1};
int pink[8] =   {0, 1, 1, 1, 0, 0, 0, 0};
int yellow[8] = {0, 0, 0, 1, 1, 1, 0, 0};
int orange[8] = {0, 0, 0, 0, 0, 1, 1, 1};


void setstep(int p1, int p2, int p3, int p4){
    gpio_set_value(PIN1, p1);
    gpio_set_value(PIN2, p2);
    gpio_set_value(PIN3, p3);
    gpio_set_value(PIN4, p4);
}

void backward(int round, int delay){
    int i = 0;
    int j = 0;

    for (i = 0; i< ONE_ROUND * round; i++){
        for (j = STEPS; j > 0; j--){
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            udelay(delay);
            
        }
    }
    setstep(0, 0, 0, 0);
}


void forward(int round, int delay){
    int i = 0;
    int j = 0;

    for (i = 0; i< ONE_ROUND * round; i++){
        for (j = 0; j < STEPS; j++){
            setstep(blue[j], pink[j], yellow[j], orange[j]);
            udelay(delay);
        }
    }
    setstep(0, 0, 0, 0);
}

void moveDegree(int round, int delay, int direction){
    if (direction)
        backward(round, delay);
    else
        forward(round, delay);
    return;
}

static int pir_read_is_detected(void){
    struct list_head *pos, *next;
    pir_q *entry = NULL;
    list_for_each_safe(pos, next, &pir_list.list){
        entry = list_entry(pos, pir_q, list);
        break;
    }
    return entry->is_detected;
}

static unsigned long pir_read_timestamp(void){
    struct list_head *pos, *next;
    pir_q *entry;
    list_for_each_safe(pos, next, &pir_list.list){
        entry = list_entry(pos, pir_q, list);
        break;
    }
    return entry->timestamp;
}

static int pir_del(void){
    struct list_head *pos, *next;
    pir_q *entry;
    list_for_each_safe(pos, next, &pir_list.list){
        entry = list_entry(pos, pir_q, list);
        list_del(pos);
        kfree(entry);
        break;
    }
    q_len--;
    return 0;
}

static int set_motor_round(int round){
    motor_round = round;
    return 0;
}
static int motor_run(void){
    backward(motor_round, 1500);
    return 0;
}
static int switch_read(void){
    return switch_value;
}
static int soap_dispenser_open(struct inode *inode, struct file *file)
{
    printk("soap_dispenser: file opened\n");
    return 0;
}

static int soap_dispenser_release(struct inode *inode, struct file *file)
{
    printk("soap_dispenser: file closed\n");
    return 0;
}

static long soap_dispenser_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int result = -1;
    switch (cmd){
    case PIR_READ_QUEUE:
        if (q_len > 0){
            result = pir_read_timestamp();
            result = pir_read_is_detected();
        }
        else
            result = -1;
        break;

    case PIR_DELETE_QUEUE:
        if (q_len > 0)
            result = pir_del();
        else
            result = -1;
        break;

    case MOTOR_SET_ROUND:
        set_motor_round(arg);
        result = 0;
        break;

    case MOTOR_RUN:
        motor_run();
        result = 0;
        break;
    case SWITCH_READ:
        result = switch_read();
        break;
    default:
        break;
    }
    return result;
}

static irqreturn_t switch_irq_rising(int irq, void* dev_id)
{
    printk("soap_dispenser: switch rising edge\n");
    switch_value = 1;
    return IRQ_HANDLED;
}
static irqreturn_t switch_irq_falling(int irq, void* dev_id)
{
    printk("soap_dispenser: switch falling edge\n");
    switch_value = 0;
    return IRQ_HANDLED;
}
static irqreturn_t pir_irq_rising(int irq, void* dev_id)
{
    pir_q *tmp;
    struct list_head *pos;
    struct list_head *next;
    pir_q *entry;
    //disable_irq(irq_num_pir);
    //disable_irq(irq_num_switch);
    printk("soap_dispenser: pir rising\n");
    q_len++;
    tmp = (pir_q*)kmalloc(sizeof(pir_q), GFP_KERNEL);
    tmp->is_detected = 1;
    tmp->timestamp = jiffies;
    list_add(&tmp->list, &pir_list.list);

   //enable_irq(irq_num_switch);
   //enable_irq(irq_num_pir);
    return IRQ_HANDLED;
}
struct file_operations soap_dispenser_fops = {
    .open = soap_dispenser_open,
    .release = soap_dispenser_release,
    .unlocked_ioctl = soap_dispenser_ioctl,
};
static dev_t dev_num;
static struct cdev *cd_cdev;


static int __init soap_dispenser_init(void){
    
    int result;

    printk("soap_dispenser: Init Module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &soap_dispenser_fops);
    result = cdev_add(cd_cdev, dev_num, 1);
    if (result < 0){
        printk("soap_dispenser: cdev_add error");
        return -1;
    }


    pir_list.is_detected = 0;
    pir_list.timestamp = jiffies;
    INIT_LIST_HEAD(&pir_list.list);

    gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");
    gpio_request_one(PIR, GPIOF_IN, "PIR");
    gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
    gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
    gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
    gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");

    irq_num_pir = gpio_to_irq(PIR); 
    irq_num_switch = gpio_to_irq(SWITCH);
    
    result = request_irq(irq_num_pir, pir_irq_rising, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    result = request_irq(irq_num_switch, switch_irq_rising, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    //result = request_irq(irq_num_switch, switch_irq_falling, IRQF_TRIGGER_FALLING, "sensor_irq", NULL);

    return 0;
}

static void soap_dispenser_exit(void){
    struct list_head *pos;
    struct list_head *next;
    pir_q *entry;
    printk("soap_dispenser: Exit Module\n");

    switch_value = 0;
    gpio_free(SWITCH);
    gpio_free(PIR);
    gpio_free(PIN1);
    gpio_free(PIN2);
    gpio_free(PIN3);
    gpio_free(PIN4);

    list_for_each_safe(pos, next, &pir_list.list){
        entry = list_entry(pos, pir_q, list);
        list_del(pos);
        kfree(entry);
    }
    disable_irq(irq_num_switch);
    disable_irq(irq_num_pir);
    free_irq(irq_num_switch, NULL);
    free_irq(irq_num_pir, NULL);
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);

}
module_init(soap_dispenser_init);
module_exit(soap_dispenser_exit);
