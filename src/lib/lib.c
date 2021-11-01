
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "../driver/driver.h"
#include <stdio.h>
#include <unistd.h>

int pir_read();
int motor_run();
int switch_read();
int motor_set_round(int round);

int pir_read(){
    int dev = open("/dev/soap_dispenser",O_RDWR);
    int result = ioctl(dev, PIR_READ_QUEUE, NULL);
    ioctl(dev, PIR_DELETE_QUEUE, NULL);
    close(dev);
    return result;
}
int motor_run(){
    int dev = open("/dev/soap_dispenser",O_RDWR);
    int result = ioctl(dev, MOTOR_RUN, NULL);
    close(dev);
    return result;
}
int switch_read(){
    int dev = open("/dev/soap_dispenser",O_RDWR);
    int result = ioctl(dev, SWITCH_READ, NULL);
    close(dev);
    return result;
}
int motor_set_round(int round)
{
    int dev = open("/dev/soap_dispenser",O_RDWR);
    int result = ioctl(dev, MOTOR_SET_ROUND, (unsigned long)round);
    close(dev);
    return result;
}