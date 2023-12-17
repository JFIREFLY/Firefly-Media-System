#include "OEvent.h"

#ifdef WIN32

OEventImpl::OEventImpl(bool bManualReset)
{
    h_Event = CreateEvent(NULL, bManualReset, FALSE, NULL);
    if (!h_Event)
        cout << "cannot create event" << endl;
}


OEventImpl::~OEventImpl()
{
    CloseHandle(h_Event);
}

bool OEventImpl::WaitImpl()
{
    switch (WaitForSingleObject(h_Event, INFINITE))
    {
    case WAIT_OBJECT_0:
        return true;
    default:
        cout << "wait for event failed" << endl;
    }
    return false;
}

bool OEventImpl::WaitImpl(long lMilliseconds)
{
    switch (WaitForSingleObject(h_Event, lMilliseconds + 1))
    {
    case WAIT_TIMEOUT:
        return false;
    case WAIT_OBJECT_0:
        return true;
    default:
        cout << "wait for event failed" << endl;
        return false;
    }
}

OEvent::OEvent(bool bManualReset) : OEventImpl(bManualReset)
{
}

OEvent::~OEvent()
{
}

#else

#include <sys/time.h> 

OEventImpl::OEventImpl(bool manualReset) : m_manual(manualReset), m_state(false)
{
    if (pthread_mutex_init(&m_mutex, NULL))
        cout << "cannot create event (mutex)" << endl;
    if (pthread_cond_init(&m_cond, NULL))
        cout << "cannot create event (condition)" << endl;
}

OEventImpl::~OEventImpl()
{
    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

bool OEventImpl::WaitImpl()
{
    if (pthread_mutex_lock(&m_mutex))
    {
        cout << "wait for event failed (lock)" << endl;
        return false;
    }
    while (!m_state)
    {
        //cout<<"OEventImpl::WaitImpl while m_state = "<<m_state<<endl;  

        //�Ի��������ԭ�ӵĽ�������,Ȼ��ȴ�״̬�ź�  
        if (pthread_cond_wait(&m_cond, &m_mutex))
        {
            pthread_mutex_unlock(&m_mutex);
            cout << "wait for event failed" << endl;
            return false;
        }
    }
    if (m_manual)
        m_state = false;
    pthread_mutex_unlock(&m_mutex);

    //cout<<"OEventImpl::WaitImpl end m_state = "<<m_state<<endl;  

    return true;
}

bool OEventImpl::WaitImpl(long milliseconds)
{
    int rc = 0;
    struct timespec abstime;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    abstime.tv_sec = tv.tv_sec + milliseconds / 1000;
    abstime.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;
    if (abstime.tv_nsec >= 1000000000)
    {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
    }

    if (pthread_mutex_lock(&m_mutex) != 0)
    {
        cout << "wait for event failed (lock)" << endl;
        return false;
    }
    while (!m_state)
    {
        //�Զ��ͷŻ����岢�ҵȴ�m_cond״̬,�������������ĵȴ�ʱ��  
        if ((rc = pthread_cond_timedwait(&m_cond, &m_mutex, &abstime)))
        {
            if (rc == ETIMEDOUT) break;
            pthread_mutex_unlock(&m_mutex);
            cout << "cannot wait for event" << endl;
            return false;
        }
    }
    if (rc == 0 && m_manual)
        m_state = false;
    pthread_mutex_unlock(&m_mutex);
    return rc == 0;
}

OEvent::OEvent(bool bManualReset) : OEventImpl(bManualReset)
{
}

OEvent::~OEvent()
{
}

#endif // WIN32