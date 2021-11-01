#include <unistd.h>
extern int pir_read();
extern int motor_run();
extern int switch_read();
extern int motor_set_round(int round);

int main(){
    int pir = 0;
    
    while(1){
       if (pir_read() == 1){
        if (switch_read() == 1)
            motor_set_round(2);
        else
            motor_set_round(1);
        motor_run();
       }
        sleep(1);
    }
}