#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#include "fake-header/Parcel.h"
#include "fake-header/IBinder.h"
#include "fake-header/BpBinder.h"
#include <binder/IServiceManager.h>

using namespace android;
using namespace std;
string svc_name[] = {
        string("media.audio_flinger"),
        string("media.camera"),
        string("media.camera.proxy"),
        string("media.log"),
        string("media.metrics"),
        string("media.codec"),
        string("media.audio_policy"),
        string("media.resource_manager"),
        string("media.extractor.update"),
        string("media.extractor"),
        string("media.sound_trigger_hw"),
        string("media.drm"),
        };

static vector< sp<IBinder>> ibinders;

class DriverTalker{
private:
    string dev_name = "/dev/binder";
    vector<thread*> thread_pool;
    int mFd;
public:
    DriverTalker(){
        mFd = open(dev_name.data() , O_RDWR | O_CLOEXEC);
    }
    void talkWithDriver(int isize, uint8_t* ibuf, int osize, uint8_t* obuf) {
        binder_write_read bwr;
        bwr.read_size = isize;
        bwr.read_buffer = reinterpret_cast<binder_uintptr_t>(ibuf) ;
        bwr.write_size = osize;
        bwr.write_buffer = reinterpret_cast<binder_uintptr_t> (obuf);
        ioctl(mFd, BINDER_WRITE_READ, &bwr);
    }
    struct Message{
        int cmd;
        struct binder_transaction_data tr;
        Message(int cmd,uint32_t handle){
            this->cmd=cmd;
            tr={
                    .sender_pid=getpid(),
                    .sender_euid=getuid(),
                    .target.handle=handle,
            };
        }
    };

public:
    void transact(uint32_t handle){
        struct Message message(BC_TRANSACTION,handle);
        talkWithDriver(sizeof(Message),reinterpret_cast<uint8_t *>(&message),0, nullptr);
    }
    void aquire(uint32_t handle){
        struct Message message(BC_ACQUIRE,handle);
        talkWithDriver(sizeof(Message),reinterpret_cast<uint8_t *>(&message),0, nullptr);
    }
    void increfs(uint32_t handle){
        struct Message message(BC_INCREFS,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }
    void dead_binder(uint32_t handle){
        struct Message message(BC_DEAD_BINDER_DONE,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }
    void clear_death_notification(int handle){
        struct Message message(BC_CLEAR_DEATH_NOTIFICATION,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }
    void transact_thread(uint32_t handle, int times){

        std::thread* myThread = new std::thread([&]() {
            while(true){
                transact(handle);
                cout <<"handle " << handle << " call transact success" << endl;
            }
            for(int i=times;i>0;i--){
                transact(handle);
                cout <<"handle" << handle << "call transact success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void aquire_thread(uint32_t handle, int times){
        cout <<times;
        std::thread* myThread = new std::thread([&]() {
            while(true){
                aquire(handle);
                cout <<"handle " << handle << " call aquire success" << endl;
            }
            for(int i=times;i>0;i--){
                aquire(handle);
                cout <<"handle" << handle << "call aquire success, times:" << i << endl;;
            }
        });
        thread_pool.push_back(myThread);
    }

    void increfs_thread(uint32_t handle, int times){
        cout <<times;
        std::thread* myThread = new std::thread([&]() {
            while(true){
                increfs(handle);
                cout <<"handle " << handle << " call increfs success" << endl;
            }
            for(int i=times;i>0;i--){
                increfs(handle);
                cout <<"handle" << handle << "call increfs success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void dead_binder_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            while(true){
                dead_binder(handle);
                cout <<"handle " << handle << " call dead_binder success" << endl;
            }
            for(int i=times;i>0;i--){
                dead_binder(handle);
                cout <<"handle" << handle << "call dead_binder success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void reply(uint32_t handle){
        struct Message message(BC_REPLY,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }

    void aquire_down(uint32_t handle){
        struct Message message(BC_ACQUIRE_DONE,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }

    void incref_down(uint32_t handle){
        struct Message message(BC_INCREFS_DONE,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }

    void incref_down_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            //cout <<times;
            while(true){
                incref_down(handle);
                cout <<"handle " << handle << " call incref_down success" << endl;

            }
            for(int i=times;i>0;i--){
                incref_down(handle);
                cout <<"handle" << handle << "call incref_down success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void aquire_down_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            //cout << "------------------------" << times<< endl;
            while(true){
                aquire_down(handle);
                cout <<"handle " << handle << " call aquire_down success" << endl;
            }
            for(int i=times;i>0;i--){
                aquire_down(handle);
                cout <<"handle" << handle << "call aquire_down success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void reply_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            while(true){
                reply(handle);
                cout <<"handle " << handle << " call reply success" << endl;
            }
            for(int i=times;i>0;i--){
                reply(handle);
                cout <<"handle" << handle << "call reply success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void clear_death_notification_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            while(true){
                clear_death_notification(handle);
                cout <<"handle " << handle << " call clear_death_notification success" << endl;
            }
            for(int i=times;i>0;i--){
                clear_death_notification(handle);
                cout <<"handle" << handle << "call clear_death_notification success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    void release(uint32_t handle){
        struct Message message(BC_RELEASE,handle);
        talkWithDriver(sizeof(message), reinterpret_cast<uint8_t *>(&message),0 , nullptr);
    }

    void release_thread(uint32_t handle, int times){
        std::thread* myThread = new std::thread([&]() {
            while(true){
                release(handle);
                cout <<"handle " << handle << " call releae success" << endl;
            }
            for(int i=times;i>0;i--){
                release(handle);
                cout <<"handle " << handle << " call releae success, times:" << i << endl;
            }
        });
        thread_pool.push_back(myThread);
    }

    virtual ~DriverTalker(){
        for(std::thread* thd:thread_pool){
            thd->join();
        }
    }

};

class Fuzzer{
private:
    DriverTalker* talker;
    vector<int> mhandles;
public:
    Fuzzer(){
        talker = new DriverTalker();
    }

    void addHandle(int handle){
        mhandles.push_back(handle);
    }

    void handleTest(){
        for(int handle:mhandles){
            //cout << "handle test" << handle << endl;
            //cout << "aquire: "<<endl;
            //talker->aquire(443);
            //cout << "transact: "<<endl;
            //talker->transact(443);
            //thread* thread_pool[0xff];
            for(int i=0;i<2;i++){
                cout << "now:" << i << endl;
                talker->aquire_thread(handle,20);
                talker->increfs_thread(handle,20);
                talker->transact_thread(handle,20);
                talker->dead_binder_thread(handle,20);
                talker->clear_death_notification_thread(handle,20);
                talker->reply_thread(handle,20);
                talker->incref_down_thread(handle,20);
                talker->aquire_down_thread(handle,20);
                talker->release_thread(handle,20);
            }

        }
    }
    virtual ~Fuzzer(){
        delete talker;
    }
};

int main(){

  for(int i=0;i<2;i++){
      fork();
  }

  Fuzzer* f=new Fuzzer();
  for(const string name:svc_name){
      sp<IBinder> ibinder = defaultServiceManager()->getService(String16(name.data()) );
      if(ibinder != nullptr){
          if(ibinder->remoteBinder()){
              BpBinder* bpBinder=ibinder->remoteBinder();

              if(bpBinder == nullptr){
                  cout << "bpBinder nullptr" << endl;
              }
              else{
                  cout << "bpBinder->mHandle: " << bpBinder->mHandle << endl;

                  cout << "get " << name << " success" <<endl;
                  f->addHandle(bpBinder->mHandle);
                  /*
                      for(BpBinder::Obituary ob : *bpBinder->mObituaries){
                          cout << "ob cookie: " << ob.cookie << "flags: " << ob.flags << endl;
                      }
                       */
              }
          }
          else{
              BBinder* bBinder=ibinder->localBinder();
              if(bBinder == nullptr){
                  cout << "bBinder nullptr" << endl;
              }else{

              }
          }
          ibinders.push_back(ibinder);
      }

  }
  f->handleTest();
  delete f;

	Parcel data;

    /*
     * int code=1;
    Parcel reply;
    Parcel data;
    */


/*
	data.writeString16(String16("abc"));
    data.writeInt32(123);
    data.writeString16(String16("word"));
	data.writeStrongBinder(ibinder);
*/
    //ibinder->transact(code,data,&reply,0);



	return 0;
}
