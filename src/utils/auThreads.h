#ifndef AU_THREADS_H
#define AU_THREADS_H

namespace au
{
    //-------------------------------------------------------------------------------------------
    template<typename TLock>
    class AutoLock
    {
    public:
        typedef TLock lock_type;

        AutoLock(lock_type *pLock)
            :m_pLock(pLock)
        {
            if (m_pLock)
                m_pLock->Lock();
        }

        ~AutoLock()
        {
            if (m_pLock)
                m_pLock->Unlock();
        }

    private:
        lock_type *m_pLock;
    };
} // namespace au

#endif