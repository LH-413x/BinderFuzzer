
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <poll.h>

#include "fake-header/IBinder.h"
#include <android/log.h>
#include <linux/android/binder.h>
#include <fuzz-commond.h>
#define TAG "binder_interface_tester.cpp:" 

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__)  
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)  
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__)  
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__)  
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) 

#define BINDER_DEV_NAME "/dev/binder"

int m_binderFd;

class BinderInterfaceTest {
public:
  static int init(){
    LOGD("call init");
    m_binderFd=0;
    m_binderFd = open(BINDER_DEV_NAME, O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if(m_binderFd < 0){
      LOGE("open fail"); 
      return -1;
    }
    return 1; 
  }
  static int IncRefsAcquireReleaseDecRefs(){
    LOGD("call IncRefsAcquireReleaseDecRefs");
    const uint32_t bc[] = {
        BC_INCREFS,
        0,
        BC_ACQUIRE,
        0,
        BC_RELEASE,
        0,
        BC_DECREFS,
        0,
    };
    struct binder_write_read bwr = binder_write_read();
    bwr.write_buffer = (uintptr_t)bc;
    bwr.write_size = sizeof(bc);
    binder_ioctl(BINDER_WRITE_READ,&bwr);
    return 1;
  }  
  static int Transaction(){
    LOGD("call Transaction");
    struct{
      uint32_t cmd1;
      struct binder_transaction_data arg1;
    } __attribute__ ((packed)) bc1={
      .cmd1=BC_TRANSACTION,
      .arg1={
        .target={0},
        .cookie=0,
        .code=android::IBinder::PING_TRANSACTION,
        .flags=0,
        .sender_pid=0,
        .sender_euid=0,
        .data_size=0,
        .offsets_size=0,
        .data={
          .ptr={0, 0},
        }
      }
    };
    struct {
      uint32_t cmd0;
      uint32_t cmd1;
      uint32_t cmd2;
      binder_transaction_data arg2;
      uint32_t pad[16];
    } __attribute__ ((packed)) br;
    struct binder_write_read bwr=binder_write_read();
    bwr.write_size=sizeof(bc1);
    bwr.write_buffer=(uintptr_t)&bc1;
    bwr.read_size=sizeof(br);
    bwr.read_buffer=(uintptr_t)&br;
    binder_ioctl(BINDER_WRITE_READ,&bwr);

    struct {
        uint32_t cmd1;
        binder_uintptr_t arg1;
    } __attribute__((packed)) bc2 = {
        .cmd1 = BC_FREE_BUFFER,
        .arg1 = br.arg2.data.ptr.buffer,
    };

    bwr.write_buffer = (uintptr_t)&bc2;
    bwr.write_size = sizeof(bc2);
    bwr.write_consumed = 0;
    bwr.read_size = 0;

    binder_ioctl(BINDER_WRITE_READ, &bwr);

    return 1;
  }

  static int RequestDeathNotification(){
    LOGD("call RequestDeathNotification");
    binder_uintptr_t  cookie=1234;
    struct{
      uint32_t cmd0;
      uint32_t arg0;
      uint32_t cmd1;
      struct binder_handle_cookie arg1;
      uint32_t cmd2;
      struct binder_handle_cookie arg2;
      uint32_t cmd3;
      uint32_t arg3;
    } __attribute__ ((packed)) bc={
      .cmd0=BC_INCREFS,
      .arg0=0,
      .cmd1=BC_REQUEST_DEATH_NOTIFICATION,
      .arg1={
        .handle=0,
        .cookie=cookie,
      },
      .cmd2=BC_CLEAR_DEATH_NOTIFICATION,
      .arg2={
        .handle=0,
        .cookie=cookie,
      },
      .cmd3=BC_DECREFS,
      .arg3=0,
    };
  
    struct {
      uint32_t cmd0;
      uint32_t cmd1;
      binder_uintptr_t arg1;
      uint32_t pad[16];
    } __attribute__ ((packed)) br;
    struct binder_write_read bwr=binder_write_read();
    bwr.read_size=sizeof(br);
    bwr.read_buffer=(uintptr_t)&br;
    bwr.write_size=sizeof(bc);
    bwr.write_buffer=(uintptr_t)&bc;
    binder_ioctl(BINDER_WRITE_READ,&bwr);
  
    return 1; 
  }
  static void binder_ioctl(int cmd,void* arg){
    ioctl(m_binderFd,cmd,arg); 
  }
};

class BinderInterfaceFuzzPlug : PluginBase{
public:
  BinderInterfaceFuzzPlug(){}
  virtual bool Setup() {
    BinderInterfaceTest::init();
    functions.push_back(BinderInterfaceTest::IncRefsAcquireReleaseDecRefs);
    functions.push_back(BinderInterfaceTest::RequestDeathNotification);
    functions.push_back(BinderInterfaceTest::Transaction);
    return true;
  }
  virtual ~BinderInterfaceFuzzPlug(){ } 
  BinderInterfaceTest* interface;
public:
  void simple_random_call_collect(){
    PluginBase::simple_random_call_collect();
  }
  void rand_call_collect_rand_times(){
    PluginBase::rand_call_collect_rand_times();
  }
};

extern "C" {
  BinderInterfaceFuzzPlug* plug;

  __attribute__ ((visibility ("default")))
  void setup(){
    LOGD("call so interface: setup");
    plug=new BinderInterfaceFuzzPlug();   
    plug->Setup();
  } 

  __attribute__ ((visibility ("default")))
  void random_call_collect(){
    LOGD("call so interface: random_call_collect");
    plug->rand_call_collect_rand_times(); 
  }

  __attribute__ ((visibility ("default")))
  void simple_random_call_collect(){
    LOGD("call so interface: simple_random_call_collect");
    plug->simple_random_call_collect();
  }
}
/*
int main(){
  init();
  Transaction();
  IncRefsAcquireReleaseDecRefs();
  RequestDeathNotification(); 
}
*/
