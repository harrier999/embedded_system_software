


#define DEV_NAME "soap_dispenser"

#define SWITCH 17
#define PIR 5

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define STEPS 8
#define ONE_ROUND 512

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4
#define IOCTL_NUM5 IOCTL_START_NUM+5

#define IOCTL_NUM 'z'
#define PIR_READ_QUEUE      _IOWR(IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define PIR_DELETE_QUEUE    _IOWR(IOCTL_NUM, IOCTL_NUM2, unsigned long *)  
#define MOTOR_SET_ROUND    _IOWR(IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define MOTOR_RUN           _IOWR(IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define SWITCH_READ         _IOWR(IOCTL_NUM, IOCTL_NUM5, unsigned long *)
