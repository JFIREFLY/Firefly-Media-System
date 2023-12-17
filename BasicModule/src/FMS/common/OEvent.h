#ifndef My_Event_Header  
#define My_Event_Header  

#ifdef WIN32

#include <iostream>
#include <Windows.h>

using namespace std;

//---------------------------------------------------------------

class OEventImpl
{
protected:

    /*
     创建一个匿名事件对象
    `bAutoReset  true   人工重置
                 false  自动重置
    */
    OEventImpl(bool bManualReset);

    /*
     销毁一个事件对象
    */
    ~OEventImpl();

    /*
     将当前事件对象设置为有信号状态
     若自动重置，则等待该事件对象的所有线程只有一个可被调度
     若人工重置，则等待该事件对象的所有线程变为可被调度
    */
    void SetImpl();

    /*
     以当前事件对象，阻塞线程，将其永远挂起
     直到事件对象被设置为有信号状态
    */
    bool WaitImpl();

    /*
     以当前事件对象，阻塞线程，将其挂起指定时间间隔
     之后线程自动恢复可调度
    */
    bool WaitImpl(long lMilliseconds);

    /*
     将当前事件对象设置为无信号状态
    */
    void ResetImpl();

private:
    HANDLE h_Event;
};

inline void OEventImpl::SetImpl()
{
    if (!SetEvent(h_Event))
    {
        cout << "cannot signal event" << endl;
    }
}

inline void OEventImpl::ResetImpl()
{
    if (!ResetEvent(h_Event))
    {
        cout << "cannot reset event" << endl;
    }
}

//---------------------------------------------------------------

class OEvent : private OEventImpl
{
public:
    OEvent(bool bManualReset = true);
    ~OEvent();

    void Set();
    bool Wait();
    bool Wait(long milliseconds);
    bool TryWait(long milliseconds);
    void Reset();

private:
    OEvent(const OEvent&);
    OEvent& operator = (const OEvent&);
};


inline void OEvent::Set()
{
    SetImpl();
}

inline bool OEvent::Wait()
{
    return WaitImpl();
}

inline bool OEvent::Wait(long milliseconds)
{
    if (!WaitImpl(milliseconds))
    {
        cout << "time out" << endl;
        return false;
    }
    else
    {
        return true;
    }
}

inline bool OEvent::TryWait(long milliseconds)
{
    return WaitImpl(milliseconds);
}

inline void OEvent::Reset()
{
    ResetImpl();
}

#else

#include <iostream>  
#include <pthread.h>  
#include <errno.h>  

using namespace std;

//---------------------------------------------------------------  

class OEventImpl
{
protected:

    /*
     动态方式初始化互斥锁,初始化状态变量m_cond
    `bAutoReset  true   人工重置
                 false  自动重置
    */
    OEventImpl(bool manualReset);

    /*
     注销互斥锁,注销状态变量m_cond
    */
    ~OEventImpl();

    /*
     将当前事件对象设置为有信号状态
     若自动重置，则等待该事件对象的所有线程只有一个可被调度
     若人工重置，则等待该事件对象的所有线程变为可被调度
    */
    void SetImpl();

    /*
     以当前事件对象，阻塞线程，将其永远挂起
     直到事件对象被设置为有信号状态
    */
    bool WaitImpl();

    /*
     以当前事件对象，阻塞线程，将其挂起指定时间间隔
     之后线程自动恢复可调度
    */
    bool WaitImpl(long milliseconds);

    /*
     将当前事件对象设置为无信号状态
    */
    void ResetImpl();

private:
    bool            m_manual;
    volatile bool   m_state;
    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
};

inline void OEventImpl::SetImpl()
{
    if (pthread_mutex_lock(&m_mutex))
        cout << "cannot signal event (lock)" << endl;

    //设置状态变量为true，对应有信号  
    m_state = true;

    //cout<<"OEventImpl::SetImpl m_state = "<<m_state<<endl;  

    //重新激活所有在等待m_cond变量的线程  
    if (pthread_cond_broadcast(&m_cond))
    {
        pthread_mutex_unlock(&m_mutex);
        cout << "cannot signal event" << endl;
    }
    pthread_mutex_unlock(&m_mutex);
}

inline void OEventImpl::ResetImpl()
{
    if (pthread_mutex_lock(&m_mutex))
        cout << "cannot reset event" << endl;

    //设置状态变量为false，对应无信号  
    m_state = false;

    //cout<<"OEventImpl::ResetImpl m_state = "<<m_state<<endl;  

    pthread_mutex_unlock(&m_mutex);
}

//---------------------------------------------------------------  

class OEvent : private OEventImpl
{
public:
    OEvent(bool bManualReset = true);
    ~OEvent();

    void Set();
    bool Wait();
    bool Wait(long milliseconds);
    bool TryWait(long milliseconds);
    void Reset();

private:
    OEvent(const OEvent&);
    OEvent& operator = (const OEvent&);
};


inline void OEvent::Set()
{
    SetImpl();
}

inline bool OEvent::Wait()
{
    return WaitImpl();
}

inline bool OEvent::Wait(long milliseconds)
{
    if (!WaitImpl(milliseconds))
    {
        //cout << "time out" << endl;
        return false;
    }
    else
    {
        return true;
    }
}

inline bool OEvent::TryWait(long milliseconds)
{
    return WaitImpl(milliseconds);
}

inline void OEvent::Reset()
{
    ResetImpl();
}

#endif // WIN32

#endif  