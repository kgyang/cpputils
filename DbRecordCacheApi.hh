#ifndef  DB_CACHE_API_HH
#define  DB_CACHE_API_HH

#include <set>
#include <list>

#ifndef U32
typedef unsigned int U32;
#endif

//*****************************************************************************
//

namespace Db
{

template <class T>
class RecCacheApi_t
{
public:
  typedef std::set<T> recCache_t;
  typedef typename std::set<T>::iterator recCacheIter_t;
  typedef int ERecCacheDispatchResult;

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
  void initCache( bool dispatch = true )
  {
    return;
  }

public:
  // Methods for modifying Cache and DB
  bool add( const T& t)
  {
    // If the element already exists, then cannot add
    if( m_Cache.find(t) != m_Cache.end() ) {
      return false;
    }

    // Call validation method
    if (!validateAdd(t)) {
     return false;
    }

    if( !m_Cache.insert(t).second) {
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
    it = m_Cache.find(t);
    if( it == m_Cache.end() ) {
      return false;
    }

    T tPrev = *it;

    // Call validation method
    if (!validateModify(tPrev, t, mask)) {
     return false;
    }

    m_Cache.erase(tPrev);
    m_Cache.insert(t);

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
    it = m_Cache.find(t);
    if( it == m_Cache.end() ) {
      return false;
    }

    // Copy of the record to delete. Contains the full record, not just the key.
    // postRemove() might need more than the key.

    // Pre-validate the remove
    if (!validateRemove(t)) {
      return false;
    }

    // Remove the record.
    if( !m_Cache.erase(t)) {
      return false;
    }

    // Post-remove actions using record copy
    if (!postRemove(t)) {
      return false;
    }

    return(true);
  }

  bool get( T& t )
  {
    // User must exist in Cache and DB
    recCacheIter_t it = m_Cache.find(t);

    if( it == m_Cache.end() ) {
      return false;
    }

    // copy from cache to t struct
    t = *it;

    return(true);
  }

  bool getNext( T& t )
  {
    // User must exist in Cache and DB
    recCacheIter_t it = m_Cache.upper_bound(t);

    if( it == m_Cache.end() ) {
      return false;
    }

    // copy from cache to info struct
    t = *it;

    return(true);
  }

  bool getAll( std::list<T>& tlist)
  {
    T t;
    for (recCacheIter_t it = m_Cache.begin(); it != m_Cache.end(); it++) {
      // copy from cache to t struct
      t = *it;
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

  // Initialization
  RecCacheApi_t() { }
  virtual ~RecCacheApi_t() { }

};

#if 0
static int testDbApi(void)
{
struct Foo
{
  int a;
  int b;

  Foo() { a = 0; b = 0; }
  Foo(int a, int b) { this->a = a; this->b = b; }

  bool operator < (Foo const &o) const
  {
    return (a < o.a);
  }

};

    Db::RecCacheApi_t<Foo> dbApi;

    Foo f1(1,10);
    Foo f2(2,20);
    Foo f3(3,30);
    Foo f4(0,0);
    dbApi.add(f1);
    dbApi.add(f2);
    dbApi.add(f3);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.getNext(f4);
    printf("next is %d\n", f4.a);
    dbApi.modify(Foo(1,11));
    Foo f5(1,0);
    dbApi.get(f5);
    printf("f5 val is %d\n", f5.b);
    dbApi.remove(f5);
    dbApi.getNext(f5);
    printf("f5 next is %d\n", f5.a);
    std::list<Foo> all;
    dbApi.getAll(all);
    for (std::list<Foo>::iterator it = all.begin(); it != all.end(); ++it)
    {
        printf("all %d\n", it->a);
    }
    for (Db::RecCacheApi_t<Foo>::recCacheIter_t it = dbApi.begin(); it != dbApi.end(); ++it)
    {
        printf("iter %d\n", it->a);
    }
}
#endif


} // end of namespace Db

#endif
