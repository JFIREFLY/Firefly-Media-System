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
     ����һ�������¼�����
    `bAutoReset  true   �˹�����
                 false  �Զ�����
    */
    OEventImpl(bool bManualReset);

    /*
     ����һ���¼�����
    */
    ~OEventImpl();

    /*
     ����ǰ�¼���������Ϊ���ź�״̬
     ���Զ����ã���ȴ����¼�����������߳�ֻ��һ���ɱ�����
     ���˹����ã���ȴ����¼�����������̱߳�Ϊ�ɱ�����
    */
    void SetImpl();

    /*
     �Ե�ǰ�¼����������̣߳�������Զ����
     ֱ���¼���������Ϊ���ź�״̬
    */
    bool WaitImpl();

    /*
     �Ե�ǰ�¼����������̣߳��������ָ��ʱ����
     ֮���߳��Զ��ָ��ɵ���
    */
    bool WaitImpl(long lMilliseconds);

    /*
     ����ǰ�¼���������Ϊ���ź�״̬
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
     ��̬��ʽ��ʼ��������,��ʼ��״̬����m_cond
    `bAutoReset  true   �˹�����
                 false  �Զ�����
    */
    OEventImpl(bool manualReset);

    /*
     ע��������,ע��״̬����m_cond
    */
    ~OEventImpl();

    /*
     ����ǰ�¼���������Ϊ���ź�״̬
     ���Զ����ã���ȴ����¼�����������߳�ֻ��һ���ɱ�����
     ���˹����ã���ȴ����¼�����������̱߳�Ϊ�ɱ�����
    */
    void SetImpl();

    /*
     �Ե�ǰ�¼����������̣߳�������Զ����
     ֱ���¼���������Ϊ���ź�״̬
    */
    bool WaitImpl();

    /*
     �Ե�ǰ�¼����������̣߳��������ָ��ʱ����
     ֮���߳��Զ��ָ��ɵ���
    */
    bool WaitImpl(long milliseconds);

    /*
     ����ǰ�¼���������Ϊ���ź�״̬
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

    //����״̬����Ϊtrue����Ӧ���ź�  
    m_state = true;

    //cout<<"OEventImpl::SetImpl m_state = "<<m_state<<endl;  

    //���¼��������ڵȴ�m_cond�������߳�  
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

    //����״̬����Ϊfalse����Ӧ���ź�  
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