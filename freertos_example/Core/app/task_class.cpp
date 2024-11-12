#include "task_class.h"

int myClass::obj_count = 0; 

myClass::myClass()
{
    xTaskCreate(myClassTask, "Task in Class", 128, this, 1, &class_task_hdl);
    obj_count++;
    num = obj_count;
}

void myClass::myClassTask(void* arg)
{
        myClass* obj = (myClass*)arg;
        obj->myTask();
}

void myClass::myTask()
{
    static int obj_num = num; 
    
    while(1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}