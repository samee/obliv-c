/*** Macros to fixup some things that the parser does not understand ***/


/* Use this in the source to cast an integer what to a pointer in the same 
 * home area as host. Use this guarded by CCURED  */
#ifdef CCURED
#define CASTTOPOINTER(btyp, host, what) \
      (((btyp *)(host)) + (((S32)(what) - (S32)(host)) / ((S32)sizeof(btyp)))) 
#else
#define CASTTOPOINTER(btyp, host, what) ((btyp *)(what))
#endif

#if !defined(__NODE) && !defined(MANUALBOX)
  #define __SAFE
  #define __INDEX
  #define __TAGGED
  #define __FSEQ
  #define __SEQ
  #define __WILD
  #define __SIZED
  #define __RWSTRING
  #define __ROSTRING
  #define __NULLTERM
  #define __SEQN
  #define __FSEQN
  #define __SAFEUNION
  #define __WILDT
  #define __FSEQT
  #define __SEQT
  #define __FSEQNT
  #define __SEQNT
  #define __INDEXT
  #define __NODE
  #define __HEAPIFY
  #define __DUMMYDEFN
  #define __BOXMODEL
  #define __NOBOXBLOCK
  #define __NOBOX
  #define __MODELEDBODY
  #define __BOXVARARG(x)
  #define __BOXFORMAT(x)
#else
  #define __WILD   __attribute__((wild))
  #define __SAFE   __attribute__((safe))
  #define __TAGGED __attribute__((tagged))
  #define __INDEX  __attribute__((index))
  #define __SIZED  __attribute__((sized))
  #define __SEQ    __attribute__((seq))
  #define __FSEQ   __attribute__((fseq))
  #define __SEQN   __attribute__((seqn))
  #define __FSEQN  __attribute__((fseqn))
  #define __NULLTERM   __attribute__((nullterm))
  #define __ROSTRING  __attribute__((rostring))
  #define __RWSTRING  __attribute__((string))
  #define __SAFEUNION __attribute__((safeunion))
  #define __INDEXT   __attribute__((indext))
  #define __WILDT   __attribute__((wildt))
  #define __SEQT   __attribute__((seqt))
  #define __SEQNT   __attribute__((seqnt))
  #define __FSEQT   __attribute__((fseqt))
  #define __FSEQNT   __attribute__((fseqnt))
  #define __NODE
  #ifndef DISABLE_HEAPIFY
    #define __HEAPIFY __attribute__((heapify))
  #else
    #define __HEAPIFY
  #endif
  #define __DUMMYDEFN __attribute__((dummydefn))
  #define __BOXMODEL __attribute__((boxmodel))
  #define __NOBOXBLOCK  __blockattribute__(nobox)
  #define __NOBOX __attribute__((nobox))
  #define __MODELEDBODY
  #define __BOXVARARG(x)
  #define __BOXFORMAT(x)
#endif

//#if ! defined(MANUALBOX) && ! defined(INFERBOX)
//#define calloc_fseq calloc
//#endif

#if !defined(CCURED)
  // if some code calls explicit_gc, but we're not boxing, then
  // we won't link safec{debug,}lib.a either; so let's provide
  // a dummy definition of this fn

  /* But we cannot put it here since this will end up being included several 
   * times (once before patching, and one after cabs2cil) */
  //static __inline int explicit_gc() { return 0; }
  #define explicit_gc() ((void)0)
#endif

#ifdef _MSVC
#pragma warning(disable: 4068) // Unrecognized pragma
#endif



// Now specify some special pragmas
#ifdef CCURED
  #pragma boxpoly("__startof")
  void *__startof(void *ptr); // Get the start of a pointer
  #pragma boxpoly("__endof")
  void *__endof(void *);
  #pragma boxpoly("ccured_kind_of")
  char *  ccured_kind_of(void *);
  #pragma boxalloc("malloc", nozero, sizein(1))
  #pragma boxpoly("free")
  #pragma boxalloc("alloca", nozero, sizein(1))
  #pragma boxalloc("calloc", zero, sizemul(1,2))

  // sm: not sure best way to handle this.. gcc's headers map all
  // these things to __builtin_blah, so we see that after
  // preprocessing..  could attack with patcher, but what if someone
  // writes __buildin_blah in their code?  we'll see how this goes
  #ifdef _GNUCC
    void *__builtin_alloca(unsigned int size);
    #pragma boxalloc("__builtin_alloca", nozero, sizein(1))
    // waiting on rest until need them..
  #endif

  union printf_format {
    int             f_int;
    double          f_double;
    char * __ROSTRING f_string;
  };

  #pragma boxvararg_printf("printf", 1)
  #pragma boxvararg_printf("fprintf", 2)
  #pragma boxvararg_printf("snprintf", 3)
  #pragma boxvararg_printf("syslog", 2)
  #pragma boxvararg_printf("sprintf", 2)
  #pragma boxvararg_printf("vsprintf", 2)
  #pragma boxvararg_printf("vprintf", 1)     // sm: for ftpd
  #pragma boxvararg_printf("vsyslog", 2)     // sm: for ftpd

  // We want to force sprintf to carry a length
  #pragma boxvararg("sprintf_model", sizeof(union printf_format))
  #pragma cilnoremove("sprintf_model")
  static inline
  int sprintf_model(char *buffer, const char *format, ...) {
    // Force buffer to carry a length
    void* e = __endof(buffer); // buffer ++ would do the same
    return 0;
  }
  #pragma boxmodelof("sprintf_model", "sprintf")

  // We want to force vsprintf to carry a length
  #pragma boxvararg("vsprintf_model", sizeof(union printf_format))
  static inline 
  int vsprintf_model(char *buffer, const char *format,
                     struct __ccured_va_list *args) {
    // Force buffer to carry a length
    void* e = __endof(buffer); // buffer ++ would do the same
    return 0;
  }
  #pragma boxmodelof("vsprintf_model", "vsprintf")
  #pragma cilnoremove("vsprintf_model")   


  // sm: taking a stab at strchr's model
  static inline
  char* strchr_model(char* dest, int c)
  {
    return dest;      // just establish the flow
  }
  #pragma cilnoremove("strchr_model")
  #pragma boxmodelof("strchr_model", "strchr")

  #if 0
    // sm: I didn't notice this was here before I wrote the one below..
    static inline char *strpbrk_model(char const *s, char const *accept)
    {
      int someInt = (int)(*accept);   // make sure 'accept' can be read from
      return s;                       // connect s to retval
    }
    #pragma cilnoremove("strpbrk_model")
    #pragma boxmodelof("strpbrk_model", "strpbrk")
  #endif // 0

  static inline char *strdup_model(char const *s)
  {
    char *p;
    __endof(s);                  // need a length
    return p;                    // result is unconstrained new value
  }
  #pragma cilnoremove("strdup_model")
  #pragma boxmodelof("strdup_model", "strdup")

  // sm: I cannot force return value to be fseq if 's' is ...
  static inline char *strpbrk_model(const char *s, const char *accept)
  {
    __endof(s);          // s must be searchable
    __endof(accept);     // also accept
    return s;            // return points into 's'
  }
  #pragma cilnoremove("strpbrk_model")
  #pragma boxmodelof("strpbrk_model", "strpbrk")

  static inline char *strtok_model(char *s, char const *delim)
  {
    __endof(s);
    __endof(delim);
    return s;
  }
  #pragma cilnoremove("strtok_model")
  #pragma boxmodelof("strtok_model", "strtok")

  #pragma boxpoly("memcpy")
  #pragma boxpoly("memset")
  #pragma boxpoly("memmove")
  #pragma boxpoly("memcmp")
  #pragma boxpoly("write")
  #pragma boxpoly("read")
  #pragma boxpoly("fread")
  #pragma boxpoly("fwrite")
  #pragma boxpoly("mmap")      // sm: for ftpd
  #pragma boxpoly("munmap")    // sm: for ftpd

  #pragma boxpoly("memset_seq_model")
  static inline
  void* memset_seq_model(void* dest, int towrite, unsigned int size)
  {
    void *end = __endof(dest); // Make sure it has an end
    return dest;
  }
  #pragma boxmodelof("memset_seq_model", "memset")
  #pragma cilnoremove("memset_seq_model")



  #pragma boxpoly("memcpy_seq_model")
  static inline
  void* memcpy_seq_model(void* dest, void *src, unsigned int size)
  {
    void *end = __endof(dest);
    dest = src; // Make sure they are both have the same type
    return dest;
  }
  #pragma boxmodelof("memcpy_seq_model", "memcpy", "memmove", "__builtin_memcpy")
  #pragma cilnoremove("memcpy_seq_model")



  // like for allocators, we have __builtin_blah for str*...
  #ifdef _GNUCC
    #pragma boxpoly("__builtin_memcpy")
    void *__builtin_memcpy(void *dest, const void *src, unsigned int n);
  #endif

  static inline
  void qsort_seq_model(void *base, unsigned int nmemb, unsigned int size,
          int (*compar)(const void *, const void *))
  {
      void *end = __endof(base);
      return;
  }
  #pragma boxpoly("qsort")
  #pragma boxmodelof("qsort_seq_model", "qsort")
  #pragma cilnoremove("qsort_seq_model")

// Whenever you use MAKE_QSORT(foosort, ...) also add
// #pragma cilnoremove("foosort_seq_model")
// #pragma boxmodelof("foosort_seq_model", foosort)
//
  #define MAKE_QSORT(name,namestr,elt_type) \
  static inline\
  void name (elt_type *base, unsigned int nmemb, unsigned int size,\
          int (*compar)(const elt_type *, const elt_type *));\
  static inline\
  void name ## _seq_model(elt_type *base, unsigned int nmemb, unsigned int size,\
          int (*compar)(const elt_type *, const elt_type *)) \
  {\
      elt_type *end = __endof(base);\
      return;\
  } 


#pragma boxexported("main")
#endif


// ideally we could handle this better..
// hack: 'restrict' is a problem with glibc 2.2
#define __restrict
#define restrict


// sm: I think it's a bad idea to try to match signal's declaration since it's
// such an unusual type; and it doesn't use any types that aren't built-in

// gn: disabled this since everythign in BOX mode fails due to redefin.
#ifdef CCURED
  typedef void (*_box_sig_fn)(int);
  static inline
  _box_sig_fn signal_model(int signum, _box_sig_fn fn)
  {
    // flow argument to result
    return fn;
  }
  #pragma cilnoremove("signal_model")
  #pragma boxmodelof("signal_model", "signal")


#endif // CCURED


#ifndef CCURED
#define __startof(p) p
#define __endof(p) p
#endif

