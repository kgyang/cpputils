//*****************************************************************************
//
// Description:
// -----------
// This file contains the database record cache class.
//
// If you know the structure and recType of your database record,
// this template class will provide set, get, and delete functions
// that minimize database interactions via a cache local to the owning process.
//
//*****************************************************************************

#ifndef  DBUSERCACHE_HH
#define  DBUSERCACHE_HH

#include <stdlib.h>
#include <string.h>
#include <DbAPI.hh>
#include <DbRecTypes.hh>
#include <GlobalTypes.h>
#include <CmnAssert.hh>
#include <CmnSignal.hh>
#include <set>
#include <functional>
#include <algorithm>
#include <Ec.hh>
#include <EC/EcExtInterface.hh>
#include "../comp/DataBase/include/DbAPI.sig"
#include <EC/EcSignals.sig>
#include <CmnSoftwareError.hh>
#include <heapapi.h>
#include <SharedAllocator.hh>
#include <TroubleBlock.h>


//*****************************************************************************
//

namespace Db
{
template <class T>
class RecCache_t
{
public:

  class MyPair_t
  {
    public:
    MyPair_t()
    {
      index = 0;
      t = (T*)heap_alloc_shared(sizeof(T), 0, 0);
      memset(t, 0, sizeof(T));
    }
    MyPair_t(U32 indexIn, const T *tIn):
        index(indexIn)
    {
      t = (T*)heap_alloc_shared(sizeof(T), 0, 0);
      memcpy(t, tIn, sizeof(T));
    }
    ~MyPair_t()
    {
      heap_free_shared(t);
    }
    void init(U32 index, const T *tIn)
    {
      memcpy(t, tIn, sizeof(T));
    }

    U32 index;
    T *t;

    void * operator new(std::size_t size)
    {
      return heap_alloc_shared(size, 0, 0);
    }
    void operator delete(void *ptr)
    {
      heap_free_shared(ptr);
    }

    bool operator == (MyPair_t const &other) const
    {
      return *t == *(other.t);
    }
    bool operator > (MyPair_t const &other) const
    {
      return *t > *(other.t);
    }
    bool operator < (MyPair_t const &other) const
    {
      return *t < *(other.t);
    }
  };

  struct Comp_t
  {
    bool operator()(MyPair_t const *p1, MyPair_t const *p2) const
    {
      return *(p1->t) < *(p2->t);
    }
  };

  struct Greater_t : public std::binary_function<MyPair_t *, MyPair_t *, bool>
  {
    bool operator()(const MyPair_t* p1, const MyPair_t* p2) const
    {
      return *(p1->t) > *(p2->t);
    }
  };

    typedef std::set<MyPair_t *, Comp_t, Cmn::SharedAllocator<MyPair_t> >Cache_t;

private:
    Cache_t cache_m;
public:
    typedef enum {
      eIgnored  = 0,
      eAdded    = 1,
      eModified = 2,
      eDeleted  = 3
    }ERecCacheDispatchResult;

    typedef typename Cache_t::iterator iterator;

    U32 size() { return cache_m.size();}
    bool empty() {return cache_m.empty();}
    Ec::id_type getEcid() { return ecid_m;}
    ERecordType getRecordType() {return recordType_m;}

    iterator begin()
    {
      return cache_m.begin();
    }

    iterator end()
    {
      return cache_m.end();
    }

    RecCache_t():
      ecid_m(Ec::NILEC),
      recordType_m(ERecordType(0)),
      dataSize_m (sizeof(T)),
      init_m(false)
    {
      temp_mp = new MyPair_t();
    }

    RecCache_t(ERecordType recType, Ec::id_type ecid):
      ecid_m(ecid),
      recordType_m(recType),
      dataSize_m (sizeof(T)),
      init_m(false)
    {
      temp_mp = new MyPair_t();
      initCache( recType, ecid, true );
    }

    void initCache(ERecordType recType, Ec::id_type ecid, bool dispatch = true )
    {
      // ensure initialization of the Cache can only occur once.
      if( !init_m ) {
        // member variables
        init_m = true;
        ecid_m = ecid;
        recordType_m = recType;

        EcAPIAllCards::RegisterForEcidChanges(ecid_m, true, dispatch); // send initial values and indicate

        if( dispatch ) {
          // This will initialize the Cache using the dispatcher below
          // If your process has its own dispatcher then you may not want this
          SIGSELECT const chg[] = {2, ECID_CHANGE_NOTIFICATION_SIG, ECID_NOTIFY_NOMORE_INITIAL_VALUES};
                                                                               // when all have been sent
          // populate the cache
          //
          Cmn::Signal rxSig;

          for(;;) {
            rxSig.receive(chg);

            // see what has arrived in queue
            if (dispatchSignal(rxSig)) {
              if(rxSig.signo() == ECID_NOTIFY_NOMORE_INITIAL_VALUES) {
                return;
              }
            }
          }
        }
      }
    }

    ~RecCache_t()
    {
      typename Cache_t::iterator iter;
      for (iter = cache_m.begin(); iter != cache_m.end(); ++iter)
      {
        delete (*iter);
      }
      delete (temp_mp);
      cache_m.clear();
    }

    // if returned true, then the signal was consumed
    // fills in tref with copy of the T that has been added, modified, or deleted.
    // fills in flag references that indicate if entry added/deleted/modified
    //
    bool dispatchSignal(Cmn::Signal &rxSig, ERecCacheDispatchResult &result, U32 &index, T &tref)
    {
      result = eIgnored;
      if (rxSig.signo() == ECID_NOTIFY_NOMORE_INITIAL_VALUES) {
        return true;
      } else if (rxSig.signo() == ECID_CHANGE_NOTIFICATION_SIG) {
        EcidChangeNotificationSig const& rx = *reinterpret_cast<EcidChangeNotificationSig const*>(rxSig.get());
        Ec::Payload payload(rx.payload, rx.len, Ec::Payload::COUPLET);
        Ec::Payload::const_iterator it = payload.begin();
        bool valid = it.strVal().size() > 0;
        if (it.id().id == ecid_m) {
          index = getSimpleIndex(rx.oid);
          typename Cache_t::iterator iter = find(getSimpleIndex(rx.oid));

          if (iter == cache_m.end()) {
            result = eAdded;
            MyPair_t *pr_p = new MyPair_t(getSimpleIndex(rx.oid), reinterpret_cast<T const*>(it.strVal().data()));
            cache_m.insert(pr_p);
            tref = *(pr_p->t);
          } else if (!valid) {
            result = eDeleted;
            tref = *(*iter)->t;
            delete(*iter);
            cache_m.erase(iter);
          } else {
            result = eModified;
            memcpy((*iter)->t, it.strVal().data(), dataSize_m);
            index = (*iter)->index;
            tref = *(*iter)->t;
          }
          return true;
        } else {
          // found an ecid not meant for this dispatcher
          return false;
        }
      }
      return false;
    }

    // handle dispatching of signal.  Only interested in the Cache being updated.
    bool dispatchSignal( Cmn::Signal &rxSig )
    {
      ERecCacheDispatchResult result;
      U32 index;
      T tref;
      return dispatchSignal( rxSig, result, index, tref );
    }

    // Validation is done in setHandler in equipConf
    //
    bool deleteRec(const T *t)
    {
      temp_mp->init(0, t);

      typename Cache_t::iterator iter = cache_m.find(temp_mp);

      if (iter != cache_m.end())
      {
        return deleteRec(iter);
      }else{
        return false;
      }
    }

    // Validation is done in setHandler in equipConf
    bool deleteRec(typename Cache_t::iterator &iter)
    {
      std::string strEc; //empty string means delete

      ObjectIdClass oid(0);
      oid.setSimpleIndex((*iter)->index);

      EcExtInterface ecApi;
      EcReturnedCodes rc = ecApi.EcSet(oid, ecid_m, strEc);

      if (rc == EC_RC_SUCCESS)
      {
        delete(*iter);
        cache_m.erase(iter);
      }

      return rc == EC_RC_SUCCESS;
    }

    // Validation is done in setHandler in equipConf
    bool setRec(T const* t)
    {
      temp_mp->init(0, t);

      typename Cache_t::iterator iter = cache_m.find(temp_mp);
      if (iter != cache_m.end()) {
        temp_mp->index = (*iter)->index;
      } else {
        temp_mp->index = lowestAvailableIndex();
      }

      std::string strEc(reinterpret_cast<char const *>(t), dataSize_m);

      ObjectIdClass oid(0);
      oid.setSimpleIndex(temp_mp->index);

      EcExtInterface ecApi;
      EcReturnedCodes rc = ecApi.EcSet(oid, ecid_m, strEc);

      if (rc == EC_RC_SUCCESS) {
        if (iter != cache_m.end()) {
          memcpy((*iter)->t, temp_mp->t, sizeof(T));
          (*iter)->index = temp_mp->index;
        } else {
          MyPair_t *pr_p = new MyPair_t(oid.getSimpleIndex(), t);

          cache_m.insert(pr_p);
        }
      }

      return rc == EC_RC_SUCCESS;
    }

    // Find first struct in cache which is == than the specified struct.
    typename Cache_t::iterator find(const T *t)
    {
      temp_mp->init(0, t);
      return cache_m.find(temp_mp);
    }

    // Find the next struct in cache which is > than the specified struct.
    typename Cache_t::iterator findNext(const T *t)
    {
      temp_mp->init(0, t);

      return std::find_if(begin(), end(), bind2nd(Greater_t(), temp_mp ));
    }

    typename Cache_t::iterator find(U32 index)
    {
      typename Cache_t::iterator iter;
      for (iter = cache_m.begin(); iter != cache_m.end(); ++iter)
      {
        if ((*iter)->index == index)
        {
          break;
        }
      }
      return iter;
    }

private:
    ERecordType recordType_m;
    Ec::id_type ecid_m;
    U32 dataSize_m;
    bool init_m;
    MyPair_t *temp_mp;

    U32 lowestAvailableIndex()
    {
      U16 recIndex = 0;
      // starting at index 0, keep doing get's until one fails - that will be the first available index.
      while (getRec(++recIndex))
      {
        U16 maxRec = 0;
        if (Ec::getId(ecid_m)->hasProperty(Ec::Properties::ExtDbMulti) ) {
          maxRec = 4000;
        } else {
          maxRec = 0xff;
        }
        if (recIndex > (maxRec - 0xf))
        {
          // Start swerring as we approach the limit 0xff
          //
          SOFTWARE_ERROR("About to exceed the record index limit: recType: idx", recordType_m, recIndex);
        }
	if (recIndex == maxRec)
	{
	  // crash
	  TROUBLE_BLOCK_PANIC(eCmnAssert, "Exceed Db::RecCache_t index limit: recType: idx  Resetting CC", recordType_m, recIndex);
	}
      }
      return recIndex;
    }

    T * getRec(U32 recIndex )
    {
      for (typename Cache_t::iterator iter = cache_m.begin(); iter != cache_m.end(); ++iter)
      {
        if ((*iter)->index == recIndex)
        {
          return (*iter)->t;
        }
      }
      return 0;
    }
};

} // end of namespace Db


#endif  // DBUSERCACHE_HH

