//*****************************************************************************
//
// Description:
// -----------
// This file contains the database record cache api class.
//
// If you know the structure and recType of your database record,
// this template class will provide set, get, and delete functions
// that minimize database interactions via a cache local to the owning process.
//
//*****************************************************************************

#ifndef  DBRECCACHE_API_HH
#define  DBRECCACHE_API_HH

#include <DbRecordCache.hh>

//*****************************************************************************
//

namespace Db
{

template <class T>
class RecCacheApi_t
{
public:
  typedef Db::RecCache_t<T> recCache_t;
  typedef typename Db::RecCache_t<T>::Cache_t::iterator recCacheIter_t;
  typedef typename Db::RecCache_t<T>::ERecCacheDispatchResult ERecCacheDispatchResult;

protected:
  recCache_t m_Cache; // cache

  // override validation routines as needed
  virtual bool validateAdd( const T& t)         { return true; }
  virtual bool validateModify( const T& tPrev,
                               const T& t,
                               const U32& mask) { return true; }
  virtual bool validateRemove( const T& t)      { return true; }

  // override post methods as needed
  virtual bool postAdd( const T& t)          { return true; }
  virtual bool postModify( const T& tPrev,
                           const T& t,
                           const U32& mask)  { return true; }
  virtual bool postRemove( const T& t)       { return true; }

  //Initialize the Cache
  void initCache( ERecordType recType, Ec::id_type ecid, bool dispatch = true )
  {
    m_Cache.initCache( recType, ecid, dispatch );
  }

public:
  // Methods for modifying Cache and DB
  bool add( const T& t)
  {
    // If the element already exists, then cannot add
    if( m_Cache.find(&t) != m_Cache.end() ) {
      return false;
    }

    // Call validation method
    if (!validateAdd(t)) {
     return false;
    }

    if( !m_Cache.setRec(&t)) {
      return false;
    }

    // Call post method
    if (!postAdd(t)) {
     return false;
    }

    return(true);
  }

  bool modify( const T& t )
  {
    U32 mask = 0;
    return modify(t, mask);
  }

  bool modify( const T& t, const U32& mask )
  {
    recCacheIter_t it;

    // If the element does not exist in Cache, then cannot modify
    it = m_Cache.find(&t);
    if( it == m_Cache.end() ) {
      return false;
    }

    T tPrev = *((*it)->t);

    // Call validation method
    if (!validateModify(tPrev, t, mask)) {
     return false;
    }

    if( !m_Cache.setRec(&t)) {
      return false;
    }

    // Call post method
    if (!postModify(tPrev, t, mask)) {
     return false;
    }

    return(true);
  }

  bool remove( const T& t)
  {
    // Record to delete
    recCacheIter_t it;

    // If the element does not exist in Cache, then cannot modify
    it = m_Cache.find(&t);
    if( it == m_Cache.end() ) {
      return false;
    }

    // Copy of the record to delete. Contains the full record, not just the key.
    // postRemove() might need more than the key.
    T tPrev = *((*it)->t);

    // Pre-validate the remove
    if (!validateRemove(tPrev)) {
      return false;
    }

    // Remove the record.
    if( !m_Cache.deleteRec(it)) {
      return false;
    }

    // Post-remove actions using record copy
    if (!postRemove(tPrev)) {
      return false;
    }

    return(true);
  }

  bool get( T& t )
  {
    // User must exist in Cache and DB
    recCacheIter_t it = m_Cache.find(&t);

    if( it == m_Cache.end() ) {
      return false;
    }

    // copy from cache to t struct
    t = *((*it)->t);

    return(true);
  }

  bool getNext( T& t )
  {
    // User must exist in Cache and DB
    recCacheIter_t it = m_Cache.findNext(&t);

    if( it == m_Cache.end() ) {
      return false;
    }

    // copy from cache to info struct
    t = *((*it)->t);

    return(true);
  }

  bool getAll( std::list<T>& tlist)
  {
    T t;
    for (recCacheIter_t it = m_Cache.begin(); it != m_Cache.end(); it++) {
      // copy from cache to t struct
      t = *((*it)->t);
      // add to list.
      tlist.push_back(t);
    }

    return(true);
  }

  int getSize()
  {
    return( m_Cache.size() );
  }

  bool isEmpty()
  {
    return( m_Cache.empty() );
  }

  recCacheIter_t begin()
  {
    return(m_Cache.begin() );
  }

  recCacheIter_t end()
  {
    return(m_Cache.end() );
  }

  // dispatch the signal to the DB Cache
  bool dispatchSignal( Cmn::Signal &sig, ERecCacheDispatchResult &result, U32 &index, T &tref )
  {
    return m_Cache.dispatchSignal( sig, result, index, tref );
  }

  // dispatch the signal to the DB Cache
  bool dispatchSignal( Cmn::Signal &sig )
  {
    return m_Cache.dispatchSignal( sig );
  }

  // Initialization
  RecCacheApi_t() { }
  RecCacheApi_t(ERecordType recType, Ec::id_type ecid) : m_Cache(recType,ecid) { }
  virtual ~RecCacheApi_t() { }

};

} // end of namespace Db

#endif
