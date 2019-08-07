/*
 *      this file contain fuzz abstract class, and common header that need to include
 *    in my fuzzer.
 *      fuzz is simple thread, in my design, fuzz function called from java by jni,
 *    you can use java thread pool to parallel call these function, we need to make 
 *    sure functions are thread safe
 */
#include <iostream>
#include <thread>
#include <vector>
#include <random>

#include <stdio.h>      
#include <stdlib.h>    
#include <time.h>
#include <cmath>

#include <android/log.h>

class PluginBase{
  public: 
    /*
     *     in setup function:
     *     1. init enviroment, like get handle of driver 
     *     2. put all simple fuzz functions in vector functions
     */
  virtual bool Setup() = 0; 

  struct rand_time_func{
    int times;
    bool always;
    std::function<void()> func;
  };
  
  void random_call_collect(std::vector<rand_time_func> funcs){
    srand (time(NULL));
    uint32_t rand_which_func=rand() % ((uint32_t)pow(2,funcs.size()));
    uint32_t count=0;
    while(rand_which_func!=0){
      if(rand_which_func%2){
        for(int i=0;i<funcs[i].times;i++){
          funcs[count].func();              
        }
      }
      else if(funcs[count].always){
        funcs[count].func();
      }
      count++;
      rand_which_func=rand_which_func >> 1;
    }
  }
  void rand_functions(){
    std::random_shuffle ( functions.begin(), functions.end() );    
  }
  void rand_call_collect_rand_times(){
    srand(time(NULL));
    rand_functions();
    std::vector<rand_time_func> rand_time_funcs;    
    rand_time_funcs.clear();
    for(int i=0;i<static_cast<int>(functions.size());i++){
      rand_time_func f={
        .times=rand()%max_time,
        .always=false,
        .func=functions[i],
      };
      rand_time_funcs.push_back(f); 
    } 
    random_call_collect(rand_time_funcs);
  }

  void simple_random_call_collect(){
    rand_functions();
    std::vector<rand_time_func> simple_time_funcs;
    for(int i=0;i<static_cast<int>(functions.size());i++){
      rand_time_func f={
        .times=1,
        .always=1,
        .func=functions[i],
      };
      simple_time_funcs.push_back(f);
    }
    random_call_collect(simple_time_funcs);
  }

  virtual ~PluginBase(){}

  protected:
    std::vector<std::function<void()>> functions;
    int max_time=30;
    int max_thread=5;
};
