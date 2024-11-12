#pragma once

#include "FreeRTOS.h"
#include "task.h"

class myClass
{
    public:
    myClass(); // launch task in constructor
    static void myClassTask(void *arg); 
    void myTask();

    //void fxn();
    //static void fxn2();

    private:
    static int obj_count; 
    int num;
    TaskHandle_t class_task_hdl;
};


//in main
/*

myClass obj;
obj.fxn();
myClass::fxn2();
*/