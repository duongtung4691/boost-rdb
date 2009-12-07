namespace boost { namespace rdb { namespace ct {
    
    struct map_tag;

    struct map0 {
      typedef map_tag tag;
      template<class F> void for_each(const F& f) { }
      template<class F> void for_each(const F& f) const { }
      template<class F> fusion::vector<> transform(const F& f) { return fusion::vector<>(); }
      template<class F, class S> S accumulate(F, const S& s) const { return s; }
    };
      
    namespace result_of {

      template<class S, class F, class Tag>
      struct transform;

      template<class F>
      struct transform<map0, F, map_tag> {
        typedef fusion::vector<> type;
        static type value(const map0&, F) { return type(); }
      };

      template<class S, class F>
      struct transform<S, F, map_tag> {
        typedef typename fusion::result_of::as_vector<
          typename fusion::result_of::push_back<
            typename transform<typename S::left, F, map_tag>::type,
            typename F::template result<typename S::right>::type
          >::type
        >::type type;
        
        static type value(const S& m, const F& f) {
	        typedef typename S::entry_type entry_type;
          return fusion::as_vector(fusion::push_back(transform<typename S::left, F, map_tag>::value(m.base(), f), f(m.entry_type::value)));
        }
      };

      template<class M, class F, class S, class Tag = typename M::tag>
      struct accumulate;

      template<class F, class S>
      struct accumulate<map0, F, S, map_tag> {
        typedef S type;
        
        static type value(const map0&, F, const S& s) {
          return s;
        }
      };

      template<class M, class F, class S>
      struct accumulate<M, F, S, map_tag> {
        typedef const typename F::template result<
          typename M::right,
          typename accumulate<typename M::left, F, S, map_tag>::type
        >::type type;
        
        static type value(const M& m, const F& f, const S& s) {
          typedef typename M::right right;
          typedef typename M::left left;
          return f(m.right::entry(), accumulate<left, F, S, map_tag>::value(m.base(), f, s));
        }
      };

      template<class Map, class Key>
      struct entry_at_key;

      template<class Map, class Key, class FirstKey>
      struct entry_at_key_impl {
        typedef typename entry_at_key<typename Map::left, Key>::type type;
      };

      template<class Map, class Key>
      struct entry_at_key_impl<Map, Key, Key> {
        typedef typename Map::entry_type type;
      };

      template<class Map, class Key>
      struct entry_at_key {
        typedef typename entry_at_key_impl<Map, Key, typename Map::right_key_type>::type type;
      };

      template<class Map, class Key>
      struct value_at_key {
        typedef typename entry_at_key<Map, Key>::type::value_type type;
      };
      
      template<class Map, class Key>
      struct has_key;

      template<class Map, class Key, class FirstKey>
      struct has_key_impl {
        typedef typename has_key<typename Map::left, Key>::type type;
      };

      template<class Map, class Key>
      struct has_key_impl<Map, Key, Key> {
        typedef mpl::true_ type;
      };

      template<class Map, class Key>
      struct has_key : has_key_impl<Map, Key, typename Map::right_key_type>::type {
      };

      template<class Key>
      struct has_key<map0, Key> : mpl::false_ {
      };
    }

    template<class K, class T>
    struct map_entry {
      map_entry(const T& value) : value(value) { }
      typedef map_entry base;
      typedef T type;
      typedef T value_type;
      type value;
      map_entry& entry() { return *this; }
      const map_entry& entry() const { return *this; }
    };

    //template<class K, class T>
    //inline const T& value_at_key(const map_entry<K, T>& entry) {
    //  return entry.value;
    //}

    template<class K, class T>
    inline const map_entry<K, T>& entry_at_key(const map_entry<K, T>& entry) {
      return entry;
    }

    template<class K, class V, class Base = map0>
    struct map : map_entry<K, V>, Base {

      typedef map_tag tag;
      typedef Base left;
      typedef map_entry<K, V> entry_type;
      typedef entry_type right;
      typedef K right_key_type;
      typedef V right_value_type;
      
      const Base& base() const { return *this; }
      
      explicit map(const V& value) : entry_type(value) { }
      map(const V& value, const Base& base) : entry_type(value), Base(base) { }
      
      template<class F> void for_each(const F& f) { Base::for_each(f); f(entry_type::entry()); }
      template<class F> void for_each(const F& f) const { Base::for_each(f); f(entry_type::entry()); }
     
      template<class K2, class V2>
      struct with {
        typedef map<K2, V2, map> type;
      };
    };
      
    template<class Key, class M>
    const typename result_of::value_at_key<M, Key>::type&
    at_key(const M& m) { return entry_at_key<Key>(m).value; }

    template<class C, class Tag>
    struct for_each_impl;
    
    template<class C>
    struct for_each_impl<C, map_tag> {
      template<class F>
      static void run(const C& c, const F& f) {
	typedef typename C::left left;
	typedef typename C::right right;
        for_each_impl<typename C::left, map_tag>::run(c, f);
        f(c.right::entry());
      }
    };
    
    template<>
    struct for_each_impl<map0, map_tag> {
      template<class F>
      static void run(const map0&, const F&) {
      }
    };
      
    template<class C, class F>
    inline void for_each(const C& c, const F& f) {
      for_each_impl<C, typename C::tag>::run(c, f);
    }

    template<class C, class F>
    inline typename result_of::transform<C, F, typename C::tag>::type
    transform(const C& c, const F& f) {
      return typename result_of::transform<C, F, typename C::tag>::value(c, f);
    }
      
    template<class C, class F, class S>
    typename result_of::accumulate<C, F, const S, typename C::tag>::type
    accumulate(const C& c, const F& f, const S& s) {
      return result_of::accumulate<C, F, const S, typename C::tag>::value(c, f, s);
    }

    template<class K, class T>
    inline std::ostream& operator <<(std::ostream& os, const map_entry<K, T>& wrapper) {
      return os << wrapper.value;
    }
    
    template<class K, class V, class Map>
    inline map<K, V, Map>
    add_key(const Map& m, const V& v) {
      return map<K, V, Map>(v, m);
    }

} } }
