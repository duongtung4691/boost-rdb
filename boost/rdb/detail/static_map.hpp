namespace boost { namespace rdb {

    struct static_map0 {
      template<class F> void for_each(F f) { }
      template<class F> fusion::vector<> transform(F f) { return fusion::vector<>(); }
      template<class F, class S> S accumulate(F, const S& s) const { return s; }
    };
      
    namespace result_of {

      template<class S, class F>
      struct static_map_transform;

      template<class F>
      struct static_map_transform<static_map0, F> {
        typedef fusion::vector<> type;
      };

      template<class S, class F>
      struct static_map_transform {
        typedef typename fusion::result_of::as_vector<
          typename fusion::result_of::push_back<
            typename static_map_transform<typename S::left, F>::type,
            typename F::template result<typename S::right>::type
          >::type
        >::type type;
      };

      template<class M, class F, class S>
      struct static_map_accumulate;

      template<class F, class S>
      struct static_map_accumulate<static_map0, F, S> {
        typedef S type;
      };

      template<class M, class F, class S>
      struct static_map_accumulate {
        typedef const typename F::template result<
          typename M::right,
          typename static_map_accumulate<typename M::left, F, S>::type
        >::type type;
      };

      template<class Key, class Map>
      struct static_map_get;

      template<class Key, class FirstKey, class Map>
      struct static_map_get_impl {
        typedef typename static_map_get<Key, typename Map::left>::type type;
      };

      template<class Key, class Map>
      struct static_map_get_impl<Key, Key, Map> {
        typedef typename Map::right_value_type type;
      };

      template<class Key, class Map>
      struct static_map_get {
        typedef typename static_map_get_impl<Key, typename Map::right_key_type, Map>::type type;
      };

    }

    template<class K, class T>
    struct static_map_entry {
      static_map_entry(const T& value) : value(value) { }
      typedef static_map_entry base;
      typedef T type;
      typedef K key_type;
      typedef T value_type;
      type value;
      static_map_entry& entry() { return *this; }
      const static_map_entry& entry() const { return *this; }
    };

    template<class K, class T>
    inline const T& static_map_get(const static_map_entry<K, T>& entry) {
      return entry.value;
    }

    template<class K, class V, class Base = static_map0>
    struct static_map : static_map_entry<K, V>, Base {

      typedef Base left;
      typedef static_map_entry<K, V> entry_type;
      typedef entry_type right;
      typedef K right_key_type;
      typedef V right_value_type;
      
      explicit static_map(const V& value) : entry_type(value) { }
      static_map(const V& value, const Base& base) : entry_type(value), Base(base) { }
      
      template<class F> void for_each(F f) { Base::for_each(f); f(entry_type::value); }
      
      template<class Key>
      typename result_of::static_map_get<Key, static_map>::type
      get() const { return static_map_get<Key>(*this); }
      
      template<class F>
      typename result_of::static_map_transform<static_map, F>::type
      transform(F f) {
        return fusion::as_vector(fusion::push_back(Base::transform(f), f(entry_type::value)));
      }
      
      template<class F, class S>
      typename result_of::static_map_accumulate<static_map, F, const S>::type
      accumulate(F f, const S& s) const {
        return f(right::entry(), left::accumulate(f, s));
      }
     
      template<class K2, class V2>
      struct with {
        typedef static_map<K2, V2, static_map> type;
      };
    };

    template<class K, class T>
    inline std::ostream& operator <<(std::ostream& os, const static_map_entry<K, T>& wrapper) {
      return os << wrapper.value;
    }
    
    template<class K, class V, class Map>
    inline static_map<K, V, Map>
    static_map_add_key(const V& v, const Map& m) {
      return static_map<K, V, Map>(v, m);
    }

} }