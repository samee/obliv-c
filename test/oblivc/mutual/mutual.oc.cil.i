# 1 "mutual.oc.cil.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "mutual.oc.cil.c"
# 150 "/usr/lib/gcc/x86_64-linux-gnu/4.6/include/stddef.h"
typedef long ptrdiff_t;
# 212 "/usr/lib/gcc/x86_64-linux-gnu/4.6/include/stddef.h"
typedef unsigned long size_t;
# 324 "/usr/lib/gcc/x86_64-linux-gnu/4.6/include/stddef.h"
typedef int wchar_t;
# 37 "/usr/include/stdint.h"
typedef signed char int8_t;
# 38 "/usr/include/stdint.h"
typedef short int16_t;
# 39 "/usr/include/stdint.h"
typedef int int32_t;
# 41 "/usr/include/stdint.h"
typedef long int64_t;
# 49 "/usr/include/stdint.h"
typedef unsigned char uint8_t;
# 50 "/usr/include/stdint.h"
typedef unsigned short uint16_t;
# 52 "/usr/include/stdint.h"
typedef unsigned int uint32_t;
# 56 "/usr/include/stdint.h"
typedef unsigned long uint64_t;
# 66 "/usr/include/stdint.h"
typedef signed char int_least8_t;
# 67 "/usr/include/stdint.h"
typedef short int_least16_t;
# 68 "/usr/include/stdint.h"
typedef int int_least32_t;
# 70 "/usr/include/stdint.h"
typedef long int_least64_t;
# 77 "/usr/include/stdint.h"
typedef unsigned char uint_least8_t;
# 78 "/usr/include/stdint.h"
typedef unsigned short uint_least16_t;
# 79 "/usr/include/stdint.h"
typedef unsigned int uint_least32_t;
# 81 "/usr/include/stdint.h"
typedef unsigned long uint_least64_t;
# 91 "/usr/include/stdint.h"
typedef signed char int_fast8_t;
# 93 "/usr/include/stdint.h"
typedef long int_fast16_t;
# 94 "/usr/include/stdint.h"
typedef long int_fast32_t;
# 95 "/usr/include/stdint.h"
typedef long int_fast64_t;
# 104 "/usr/include/stdint.h"
typedef unsigned char uint_fast8_t;
# 106 "/usr/include/stdint.h"
typedef unsigned long uint_fast16_t;
# 107 "/usr/include/stdint.h"
typedef unsigned long uint_fast32_t;
# 108 "/usr/include/stdint.h"
typedef unsigned long uint_fast64_t;
# 120 "/usr/include/stdint.h"
typedef long intptr_t;
# 123 "/usr/include/stdint.h"
typedef unsigned long uintptr_t;
# 135 "/usr/include/stdint.h"
typedef long intmax_t;
# 136 "/usr/include/stdint.h"
typedef unsigned long uintmax_t;
# 67 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h"
struct __anonstruct___wait_terminated_1 {
   unsigned int __w_termsig : 7 ;
   unsigned int __w_coredump : 1 ;
   unsigned int __w_retcode : 8 ;
   unsigned int : 16 ;
};
# 67 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h"
struct __anonstruct___wait_stopped_2 {
   unsigned int __w_stopval : 8 ;
   unsigned int __w_stopsig : 8 ;
   unsigned int : 16 ;
};
# 67 "/usr/include/x86_64-linux-gnu/bits/waitstatus.h"
union wait {
   int w_status ;
   struct __anonstruct___wait_terminated_1 __wait_terminated ;
   struct __anonstruct___wait_stopped_2 __wait_stopped ;
};
# 68 "/usr/include/stdlib.h"
union __anonunion___WAIT_STATUS_3 {
   union wait *__uptr ;
   int *__iptr ;
};
# 68 "/usr/include/stdlib.h"
typedef union __anonunion___WAIT_STATUS_3 __attribute__((__transparent_union__)) __WAIT_STATUS;
# 98 "/usr/include/stdlib.h"
struct __anonstruct_div_t_4 {
   int quot ;
   int rem ;
};
# 98 "/usr/include/stdlib.h"
typedef struct __anonstruct_div_t_4 div_t;
# 106 "/usr/include/stdlib.h"
struct __anonstruct_ldiv_t_5 {
   long quot ;
   long rem ;
};
# 106 "/usr/include/stdlib.h"
typedef struct __anonstruct_ldiv_t_5 ldiv_t;
# 118 "/usr/include/stdlib.h"
struct __anonstruct_lldiv_t_6 {
   long long quot ;
   long long rem ;
};
# 118 "/usr/include/stdlib.h"
typedef struct __anonstruct_lldiv_t_6 lldiv_t;
# 31 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned char __u_char;
# 32 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned short __u_short;
# 33 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __u_int;
# 34 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __u_long;
# 37 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef signed char __int8_t;
# 38 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned char __uint8_t;
# 39 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef short __int16_t;
# 40 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned short __uint16_t;
# 41 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef int __int32_t;
# 42 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __uint32_t;
# 44 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __int64_t;
# 45 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __uint64_t;
# 53 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __quad_t;
# 54 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __u_quad_t;
# 134 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __dev_t;
# 135 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __uid_t;
# 136 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __gid_t;
# 137 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __ino_t;
# 138 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __ino64_t;
# 139 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __mode_t;
# 140 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __nlink_t;
# 141 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __off_t;
# 142 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __off64_t;
# 143 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef int __pid_t;
# 144 "/usr/include/x86_64-linux-gnu/bits/types.h"
struct __anonstruct___fsid_t_7 {
   int __val[2] ;
};
# 144 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef struct __anonstruct___fsid_t_7 __fsid_t;
# 145 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __clock_t;
# 146 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __rlim_t;
# 147 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __rlim64_t;
# 148 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __id_t;
# 149 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __time_t;
# 150 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __useconds_t;
# 151 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __suseconds_t;
# 153 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef int __daddr_t;
# 154 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __swblk_t;
# 155 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef int __key_t;
# 158 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef int __clockid_t;
# 161 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef void *__timer_t;
# 164 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __blksize_t;
# 169 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __blkcnt_t;
# 170 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __blkcnt64_t;
# 173 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __fsblkcnt_t;
# 174 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __fsblkcnt64_t;
# 177 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __fsfilcnt_t;
# 178 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned long __fsfilcnt64_t;
# 180 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __ssize_t;
# 184 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef __off64_t __loff_t;
# 185 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef __quad_t *__qaddr_t;
# 186 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef char *__caddr_t;
# 189 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef long __intptr_t;
# 192 "/usr/include/x86_64-linux-gnu/bits/types.h"
typedef unsigned int __socklen_t;
# 34 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __u_char u_char;
# 35 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __u_short u_short;
# 36 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __u_int u_int;
# 37 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __u_long u_long;
# 38 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __quad_t quad_t;
# 39 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __u_quad_t u_quad_t;
# 40 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __fsid_t fsid_t;
# 45 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __loff_t loff_t;
# 49 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __ino_t ino_t;
# 61 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __dev_t dev_t;
# 66 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __gid_t gid_t;
# 71 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __mode_t mode_t;
# 76 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __nlink_t nlink_t;
# 81 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __uid_t uid_t;
# 87 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __off_t off_t;
# 99 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __pid_t pid_t;
# 105 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __id_t id_t;
# 110 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __ssize_t ssize_t;
# 116 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __daddr_t daddr_t;
# 117 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __caddr_t caddr_t;
# 123 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __key_t key_t;
# 60 "/usr/include/time.h"
typedef __clock_t clock_t;
# 76 "/usr/include/time.h"
typedef __time_t time_t;
# 92 "/usr/include/time.h"
typedef __clockid_t clockid_t;
# 104 "/usr/include/time.h"
typedef __timer_t timer_t;
# 151 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned long ulong;
# 152 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned short ushort;
# 153 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned int uint;
# 201 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned char u_int8_t;
# 202 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned short u_int16_t;
# 203 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned int u_int32_t;
# 204 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef unsigned long u_int64_t;
# 206 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef int register_t;
# 24 "/usr/include/x86_64-linux-gnu/bits/sigset.h"
typedef int __sig_atomic_t;
# 29 "/usr/include/x86_64-linux-gnu/bits/sigset.h"
struct __anonstruct___sigset_t_8 {
   unsigned long __val[(unsigned long )1024 / (unsigned long )((unsigned long )8 * (unsigned long )sizeof(unsigned long ))] ;
};
# 29 "/usr/include/x86_64-linux-gnu/bits/sigset.h"
typedef struct __anonstruct___sigset_t_8 __sigset_t;
# 38 "/usr/include/x86_64-linux-gnu/sys/select.h"
typedef __sigset_t sigset_t;
# 120 "/usr/include/time.h"
struct timespec {
   __time_t tv_sec ;
   long tv_nsec ;
};
# 31 "/usr/include/x86_64-linux-gnu/bits/time.h"
struct timeval {
   __time_t tv_sec ;
   __suseconds_t tv_usec ;
};
# 49 "/usr/include/x86_64-linux-gnu/sys/select.h"
typedef __suseconds_t suseconds_t;
# 55 "/usr/include/x86_64-linux-gnu/sys/select.h"
typedef long __fd_mask;
# 65 "/usr/include/x86_64-linux-gnu/sys/select.h"
struct __anonstruct_fd_set_9 {
   __fd_mask __fds_bits[(int )1024 / (int )((int )8 * (int )((int )sizeof(__fd_mask )))] ;
};
# 65 "/usr/include/x86_64-linux-gnu/sys/select.h"
typedef struct __anonstruct_fd_set_9 fd_set;
# 83 "/usr/include/x86_64-linux-gnu/sys/select.h"
typedef __fd_mask fd_mask;
# 229 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __blksize_t blksize_t;
# 236 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __blkcnt_t blkcnt_t;
# 240 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __fsblkcnt_t fsblkcnt_t;
# 244 "/usr/include/x86_64-linux-gnu/sys/types.h"
typedef __fsfilcnt_t fsfilcnt_t;
# 50 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef unsigned long pthread_t;
# 53 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_attr_t_10 {
   char __size[56] ;
   long __align ;
};
# 53 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_attr_t_10 pthread_attr_t;
# 61 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
struct __pthread_internal_list {
   struct __pthread_internal_list *__prev ;
   struct __pthread_internal_list *__next ;
};
# 61 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef struct __pthread_internal_list __pthread_list_t;
# 76 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
struct __pthread_mutex_s {
   int __lock ;
   unsigned int __count ;
   int __owner ;
   unsigned int __nusers ;
   int __kind ;
   int __spins ;
   __pthread_list_t __list ;
};
# 76 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_mutex_t_11 {
   struct __pthread_mutex_s __data ;
   char __size[40] ;
   long __align ;
};
# 76 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_mutex_t_11 pthread_mutex_t;
# 106 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_mutexattr_t_12 {
   char __size[4] ;
   int __align ;
};
# 106 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_mutexattr_t_12 pthread_mutexattr_t;
# 115 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
struct __anonstruct___data_14 {
   int __lock ;
   unsigned int __futex ;
   unsigned long long __total_seq ;
   unsigned long long __wakeup_seq ;
   unsigned long long __woken_seq ;
   void *__mutex ;
   unsigned int __nwaiters ;
   unsigned int __broadcast_seq ;
};
# 115 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_cond_t_13 {
   struct __anonstruct___data_14 __data ;
   char __size[48] ;
   long long __align ;
};
# 115 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_cond_t_13 pthread_cond_t;
# 132 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_condattr_t_15 {
   char __size[4] ;
   int __align ;
};
# 132 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_condattr_t_15 pthread_condattr_t;
# 140 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef unsigned int pthread_key_t;
# 144 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef int pthread_once_t;
# 150 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
struct __anonstruct___data_17 {
   int __lock ;
   unsigned int __nr_readers ;
   unsigned int __readers_wakeup ;
   unsigned int __writer_wakeup ;
   unsigned int __nr_readers_queued ;
   unsigned int __nr_writers_queued ;
   int __writer ;
   int __shared ;
   unsigned long __pad1 ;
   unsigned long __pad2 ;
   unsigned int __flags ;
};
# 150 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_rwlock_t_16 {
   struct __anonstruct___data_17 __data ;
   char __size[56] ;
   long __align ;
};
# 150 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_rwlock_t_16 pthread_rwlock_t;
# 191 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_rwlockattr_t_18 {
   char __size[8] ;
   long __align ;
};
# 191 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_rwlockattr_t_18 pthread_rwlockattr_t;
# 201 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef int volatile pthread_spinlock_t;
# 206 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_barrier_t_19 {
   char __size[32] ;
   long __align ;
};
# 206 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_barrier_t_19 pthread_barrier_t;
# 212 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
union __anonunion_pthread_barrierattr_t_20 {
   char __size[4] ;
   int __align ;
};
# 212 "/usr/include/x86_64-linux-gnu/bits/pthreadtypes.h"
typedef union __anonunion_pthread_barrierattr_t_20 pthread_barrierattr_t;
# 349 "/usr/include/stdlib.h"
struct random_data {
   int32_t *fptr ;
   int32_t *rptr ;
   int32_t *state ;
   int rand_type ;
   int rand_deg ;
   int rand_sep ;
   int32_t *end_ptr ;
};
# 418 "/usr/include/stdlib.h"
struct drand48_data {
   unsigned short __x[3] ;
   unsigned short __old_x[3] ;
   unsigned short __c ;
   unsigned short __init ;
   unsigned long long __a ;
};
# 742 "/usr/include/stdlib.h"
typedef int (*__compar_fn_t)(void const * , void const * );
# 40 "/usr/lib/gcc/x86_64-linux-gnu/4.6/include/stdarg.h"
typedef __builtin_va_list __gnuc_va_list;
# 102 "/usr/lib/gcc/x86_64-linux-gnu/4.6/include/stdarg.h"
typedef __gnuc_va_list va_list;
# 28 "/usr/include/xlocale.h"
struct __locale_data;
# 28 "/usr/include/xlocale.h"
struct __locale_struct {
   struct __locale_data *__locales[13] ;
   unsigned short const *__ctype_b ;
   int const *__ctype_tolower ;
   int const *__ctype_toupper ;
   char const *__names[13] ;
};
# 28 "/usr/include/xlocale.h"
typedef struct __locale_struct *__locale_t;
# 43 "/usr/include/xlocale.h"
typedef __locale_t locale_t;
# 78 "/usr/include/gpg-error.h"
enum __anonenum_gpg_err_source_t_21 {
    GPG_ERR_SOURCE_UNKNOWN = 0,
    GPG_ERR_SOURCE_GCRYPT = 1,
    GPG_ERR_SOURCE_GPG = 2,
    GPG_ERR_SOURCE_GPGSM = 3,
    GPG_ERR_SOURCE_GPGAGENT = 4,
    GPG_ERR_SOURCE_PINENTRY = 5,
    GPG_ERR_SOURCE_SCD = 6,
    GPG_ERR_SOURCE_GPGME = 7,
    GPG_ERR_SOURCE_KEYBOX = 8,
    GPG_ERR_SOURCE_KSBA = 9,
    GPG_ERR_SOURCE_DIRMNGR = 10,
    GPG_ERR_SOURCE_GSTI = 11,
    GPG_ERR_SOURCE_GPA = 12,
    GPG_ERR_SOURCE_KLEO = 13,
    GPG_ERR_SOURCE_G13 = 14,
    GPG_ERR_SOURCE_ANY = 31,
    GPG_ERR_SOURCE_USER_1 = 32,
    GPG_ERR_SOURCE_USER_2 = 33,
    GPG_ERR_SOURCE_USER_3 = 34,
    GPG_ERR_SOURCE_USER_4 = 35,
    GPG_ERR_SOURCE_DIM = 128
} ;
# 78 "/usr/include/gpg-error.h"
typedef enum __anonenum_gpg_err_source_t_21 gpg_err_source_t;
# 110 "/usr/include/gpg-error.h"
enum __anonenum_gpg_err_code_t_22 {
    GPG_ERR_NO_ERROR = 0,
    GPG_ERR_GENERAL = 1,
    GPG_ERR_UNKNOWN_PACKET = 2,
    GPG_ERR_UNKNOWN_VERSION = 3,
    GPG_ERR_PUBKEY_ALGO = 4,
    GPG_ERR_DIGEST_ALGO = 5,
    GPG_ERR_BAD_PUBKEY = 6,
    GPG_ERR_BAD_SECKEY = 7,
    GPG_ERR_BAD_SIGNATURE = 8,
    GPG_ERR_NO_PUBKEY = 9,
    GPG_ERR_CHECKSUM = 10,
    GPG_ERR_BAD_PASSPHRASE = 11,
    GPG_ERR_CIPHER_ALGO = 12,
    GPG_ERR_KEYRING_OPEN = 13,
    GPG_ERR_INV_PACKET = 14,
    GPG_ERR_INV_ARMOR = 15,
    GPG_ERR_NO_USER_ID = 16,
    GPG_ERR_NO_SECKEY = 17,
    GPG_ERR_WRONG_SECKEY = 18,
    GPG_ERR_BAD_KEY = 19,
    GPG_ERR_COMPR_ALGO = 20,
    GPG_ERR_NO_PRIME = 21,
    GPG_ERR_NO_ENCODING_METHOD = 22,
    GPG_ERR_NO_ENCRYPTION_SCHEME = 23,
    GPG_ERR_NO_SIGNATURE_SCHEME = 24,
    GPG_ERR_INV_ATTR = 25,
    GPG_ERR_NO_VALUE = 26,
    GPG_ERR_NOT_FOUND = 27,
    GPG_ERR_VALUE_NOT_FOUND = 28,
    GPG_ERR_SYNTAX = 29,
    GPG_ERR_BAD_MPI = 30,
    GPG_ERR_INV_PASSPHRASE = 31,
    GPG_ERR_SIG_CLASS = 32,
    GPG_ERR_RESOURCE_LIMIT = 33,
    GPG_ERR_INV_KEYRING = 34,
    GPG_ERR_TRUSTDB = 35,
    GPG_ERR_BAD_CERT = 36,
    GPG_ERR_INV_USER_ID = 37,
    GPG_ERR_UNEXPECTED = 38,
    GPG_ERR_TIME_CONFLICT = 39,
    GPG_ERR_KEYSERVER = 40,
    GPG_ERR_WRONG_PUBKEY_ALGO = 41,
    GPG_ERR_TRIBUTE_TO_D_A = 42,
    GPG_ERR_WEAK_KEY = 43,
    GPG_ERR_INV_KEYLEN = 44,
    GPG_ERR_INV_ARG = 45,
    GPG_ERR_BAD_URI = 46,
    GPG_ERR_INV_URI = 47,
    GPG_ERR_NETWORK = 48,
    GPG_ERR_UNKNOWN_HOST = 49,
    GPG_ERR_SELFTEST_FAILED = 50,
    GPG_ERR_NOT_ENCRYPTED = 51,
    GPG_ERR_NOT_PROCESSED = 52,
    GPG_ERR_UNUSABLE_PUBKEY = 53,
    GPG_ERR_UNUSABLE_SECKEY = 54,
    GPG_ERR_INV_VALUE = 55,
    GPG_ERR_BAD_CERT_CHAIN = 56,
    GPG_ERR_MISSING_CERT = 57,
    GPG_ERR_NO_DATA = 58,
    GPG_ERR_BUG = 59,
    GPG_ERR_NOT_SUPPORTED = 60,
    GPG_ERR_INV_OP = 61,
    GPG_ERR_TIMEOUT = 62,
    GPG_ERR_INTERNAL = 63,
    GPG_ERR_EOF_GCRYPT = 64,
    GPG_ERR_INV_OBJ = 65,
    GPG_ERR_TOO_SHORT = 66,
    GPG_ERR_TOO_LARGE = 67,
    GPG_ERR_NO_OBJ = 68,
    GPG_ERR_NOT_IMPLEMENTED = 69,
    GPG_ERR_CONFLICT = 70,
    GPG_ERR_INV_CIPHER_MODE = 71,
    GPG_ERR_INV_FLAG = 72,
    GPG_ERR_INV_HANDLE = 73,
    GPG_ERR_TRUNCATED = 74,
    GPG_ERR_INCOMPLETE_LINE = 75,
    GPG_ERR_INV_RESPONSE = 76,
    GPG_ERR_NO_AGENT = 77,
    GPG_ERR_AGENT = 78,
    GPG_ERR_INV_DATA = 79,
    GPG_ERR_ASSUAN_SERVER_FAULT = 80,
    GPG_ERR_ASSUAN = 81,
    GPG_ERR_INV_SESSION_KEY = 82,
    GPG_ERR_INV_SEXP = 83,
    GPG_ERR_UNSUPPORTED_ALGORITHM = 84,
    GPG_ERR_NO_PIN_ENTRY = 85,
    GPG_ERR_PIN_ENTRY = 86,
    GPG_ERR_BAD_PIN = 87,
    GPG_ERR_INV_NAME = 88,
    GPG_ERR_BAD_DATA = 89,
    GPG_ERR_INV_PARAMETER = 90,
    GPG_ERR_WRONG_CARD = 91,
    GPG_ERR_NO_DIRMNGR = 92,
    GPG_ERR_DIRMNGR = 93,
    GPG_ERR_CERT_REVOKED = 94,
    GPG_ERR_NO_CRL_KNOWN = 95,
    GPG_ERR_CRL_TOO_OLD = 96,
    GPG_ERR_LINE_TOO_LONG = 97,
    GPG_ERR_NOT_TRUSTED = 98,
    GPG_ERR_CANCELED = 99,
    GPG_ERR_BAD_CA_CERT = 100,
    GPG_ERR_CERT_EXPIRED = 101,
    GPG_ERR_CERT_TOO_YOUNG = 102,
    GPG_ERR_UNSUPPORTED_CERT = 103,
    GPG_ERR_UNKNOWN_SEXP = 104,
    GPG_ERR_UNSUPPORTED_PROTECTION = 105,
    GPG_ERR_CORRUPTED_PROTECTION = 106,
    GPG_ERR_AMBIGUOUS_NAME = 107,
    GPG_ERR_CARD = 108,
    GPG_ERR_CARD_RESET = 109,
    GPG_ERR_CARD_REMOVED = 110,
    GPG_ERR_INV_CARD = 111,
    GPG_ERR_CARD_NOT_PRESENT = 112,
    GPG_ERR_NO_PKCS15_APP = 113,
    GPG_ERR_NOT_CONFIRMED = 114,
    GPG_ERR_CONFIGURATION = 115,
    GPG_ERR_NO_POLICY_MATCH = 116,
    GPG_ERR_INV_INDEX = 117,
    GPG_ERR_INV_ID = 118,
    GPG_ERR_NO_SCDAEMON = 119,
    GPG_ERR_SCDAEMON = 120,
    GPG_ERR_UNSUPPORTED_PROTOCOL = 121,
    GPG_ERR_BAD_PIN_METHOD = 122,
    GPG_ERR_CARD_NOT_INITIALIZED = 123,
    GPG_ERR_UNSUPPORTED_OPERATION = 124,
    GPG_ERR_WRONG_KEY_USAGE = 125,
    GPG_ERR_NOTHING_FOUND = 126,
    GPG_ERR_WRONG_BLOB_TYPE = 127,
    GPG_ERR_MISSING_VALUE = 128,
    GPG_ERR_HARDWARE = 129,
    GPG_ERR_PIN_BLOCKED = 130,
    GPG_ERR_USE_CONDITIONS = 131,
    GPG_ERR_PIN_NOT_SYNCED = 132,
    GPG_ERR_INV_CRL = 133,
    GPG_ERR_BAD_BER = 134,
    GPG_ERR_INV_BER = 135,
    GPG_ERR_ELEMENT_NOT_FOUND = 136,
    GPG_ERR_IDENTIFIER_NOT_FOUND = 137,
    GPG_ERR_INV_TAG = 138,
    GPG_ERR_INV_LENGTH = 139,
    GPG_ERR_INV_KEYINFO = 140,
    GPG_ERR_UNEXPECTED_TAG = 141,
    GPG_ERR_NOT_DER_ENCODED = 142,
    GPG_ERR_NO_CMS_OBJ = 143,
    GPG_ERR_INV_CMS_OBJ = 144,
    GPG_ERR_UNKNOWN_CMS_OBJ = 145,
    GPG_ERR_UNSUPPORTED_CMS_OBJ = 146,
    GPG_ERR_UNSUPPORTED_ENCODING = 147,
    GPG_ERR_UNSUPPORTED_CMS_VERSION = 148,
    GPG_ERR_UNKNOWN_ALGORITHM = 149,
    GPG_ERR_INV_ENGINE = 150,
    GPG_ERR_PUBKEY_NOT_TRUSTED = 151,
    GPG_ERR_DECRYPT_FAILED = 152,
    GPG_ERR_KEY_EXPIRED = 153,
    GPG_ERR_SIG_EXPIRED = 154,
    GPG_ERR_ENCODING_PROBLEM = 155,
    GPG_ERR_INV_STATE = 156,
    GPG_ERR_DUP_VALUE = 157,
    GPG_ERR_MISSING_ACTION = 158,
    GPG_ERR_MODULE_NOT_FOUND = 159,
    GPG_ERR_INV_OID_STRING = 160,
    GPG_ERR_INV_TIME = 161,
    GPG_ERR_INV_CRL_OBJ = 162,
    GPG_ERR_UNSUPPORTED_CRL_VERSION = 163,
    GPG_ERR_INV_CERT_OBJ = 164,
    GPG_ERR_UNKNOWN_NAME = 165,
    GPG_ERR_LOCALE_PROBLEM = 166,
    GPG_ERR_NOT_LOCKED = 167,
    GPG_ERR_PROTOCOL_VIOLATION = 168,
    GPG_ERR_INV_MAC = 169,
    GPG_ERR_INV_REQUEST = 170,
    GPG_ERR_UNKNOWN_EXTN = 171,
    GPG_ERR_UNKNOWN_CRIT_EXTN = 172,
    GPG_ERR_LOCKED = 173,
    GPG_ERR_UNKNOWN_OPTION = 174,
    GPG_ERR_UNKNOWN_COMMAND = 175,
    GPG_ERR_NOT_OPERATIONAL = 176,
    GPG_ERR_NO_PASSPHRASE = 177,
    GPG_ERR_NO_PIN = 178,
    GPG_ERR_NOT_ENABLED = 179,
    GPG_ERR_NO_ENGINE = 180,
    GPG_ERR_MISSING_KEY = 181,
    GPG_ERR_TOO_MANY = 182,
    GPG_ERR_LIMIT_REACHED = 183,
    GPG_ERR_NOT_INITIALIZED = 184,
    GPG_ERR_MISSING_ISSUER_CERT = 185,
    GPG_ERR_FULLY_CANCELED = 198,
    GPG_ERR_UNFINISHED = 199,
    GPG_ERR_BUFFER_TOO_SHORT = 200,
    GPG_ERR_SEXP_INV_LEN_SPEC = 201,
    GPG_ERR_SEXP_STRING_TOO_LONG = 202,
    GPG_ERR_SEXP_UNMATCHED_PAREN = 203,
    GPG_ERR_SEXP_NOT_CANONICAL = 204,
    GPG_ERR_SEXP_BAD_CHARACTER = 205,
    GPG_ERR_SEXP_BAD_QUOTATION = 206,
    GPG_ERR_SEXP_ZERO_PREFIX = 207,
    GPG_ERR_SEXP_NESTED_DH = 208,
    GPG_ERR_SEXP_UNMATCHED_DH = 209,
    GPG_ERR_SEXP_UNEXPECTED_PUNC = 210,
    GPG_ERR_SEXP_BAD_HEX_CHAR = 211,
    GPG_ERR_SEXP_ODD_HEX_NUMBERS = 212,
    GPG_ERR_SEXP_BAD_OCT_CHAR = 213,
    GPG_ERR_ASS_GENERAL = 257,
    GPG_ERR_ASS_ACCEPT_FAILED = 258,
    GPG_ERR_ASS_CONNECT_FAILED = 259,
    GPG_ERR_ASS_INV_RESPONSE = 260,
    GPG_ERR_ASS_INV_VALUE = 261,
    GPG_ERR_ASS_INCOMPLETE_LINE = 262,
    GPG_ERR_ASS_LINE_TOO_LONG = 263,
    GPG_ERR_ASS_NESTED_COMMANDS = 264,
    GPG_ERR_ASS_NO_DATA_CB = 265,
    GPG_ERR_ASS_NO_INQUIRE_CB = 266,
    GPG_ERR_ASS_NOT_A_SERVER = 267,
    GPG_ERR_ASS_NOT_A_CLIENT = 268,
    GPG_ERR_ASS_SERVER_START = 269,
    GPG_ERR_ASS_READ_ERROR = 270,
    GPG_ERR_ASS_WRITE_ERROR = 271,
    GPG_ERR_ASS_TOO_MUCH_DATA = 273,
    GPG_ERR_ASS_UNEXPECTED_CMD = 274,
    GPG_ERR_ASS_UNKNOWN_CMD = 275,
    GPG_ERR_ASS_SYNTAX = 276,
    GPG_ERR_ASS_CANCELED = 277,
    GPG_ERR_ASS_NO_INPUT = 278,
    GPG_ERR_ASS_NO_OUTPUT = 279,
    GPG_ERR_ASS_PARAMETER = 280,
    GPG_ERR_ASS_UNKNOWN_INQUIRE = 281,
    GPG_ERR_USER_1 = 1024,
    GPG_ERR_USER_2 = 1025,
    GPG_ERR_USER_3 = 1026,
    GPG_ERR_USER_4 = 1027,
    GPG_ERR_USER_5 = 1028,
    GPG_ERR_USER_6 = 1029,
    GPG_ERR_USER_7 = 1030,
    GPG_ERR_USER_8 = 1031,
    GPG_ERR_USER_9 = 1032,
    GPG_ERR_USER_10 = 1033,
    GPG_ERR_USER_11 = 1034,
    GPG_ERR_USER_12 = 1035,
    GPG_ERR_USER_13 = 1036,
    GPG_ERR_USER_14 = 1037,
    GPG_ERR_USER_15 = 1038,
    GPG_ERR_USER_16 = 1039,
    GPG_ERR_MISSING_ERRNO = 16381,
    GPG_ERR_UNKNOWN_ERRNO = 16382,
    GPG_ERR_EOF = 16383,
    GPG_ERR_E2BIG = 32768,
    GPG_ERR_EACCES = 32769,
    GPG_ERR_EADDRINUSE = 32770,
    GPG_ERR_EADDRNOTAVAIL = 32771,
    GPG_ERR_EADV = 32772,
    GPG_ERR_EAFNOSUPPORT = 32773,
    GPG_ERR_EAGAIN = 32774,
    GPG_ERR_EALREADY = 32775,
    GPG_ERR_EAUTH = 32776,
    GPG_ERR_EBACKGROUND = 32777,
    GPG_ERR_EBADE = 32778,
    GPG_ERR_EBADF = 32779,
    GPG_ERR_EBADFD = 32780,
    GPG_ERR_EBADMSG = 32781,
    GPG_ERR_EBADR = 32782,
    GPG_ERR_EBADRPC = 32783,
    GPG_ERR_EBADRQC = 32784,
    GPG_ERR_EBADSLT = 32785,
    GPG_ERR_EBFONT = 32786,
    GPG_ERR_EBUSY = 32787,
    GPG_ERR_ECANCELED = 32788,
    GPG_ERR_ECHILD = 32789,
    GPG_ERR_ECHRNG = 32790,
    GPG_ERR_ECOMM = 32791,
    GPG_ERR_ECONNABORTED = 32792,
    GPG_ERR_ECONNREFUSED = 32793,
    GPG_ERR_ECONNRESET = 32794,
    GPG_ERR_ED = 32795,
    GPG_ERR_EDEADLK = 32796,
    GPG_ERR_EDEADLOCK = 32797,
    GPG_ERR_EDESTADDRREQ = 32798,
    GPG_ERR_EDIED = 32799,
    GPG_ERR_EDOM = 32800,
    GPG_ERR_EDOTDOT = 32801,
    GPG_ERR_EDQUOT = 32802,
    GPG_ERR_EEXIST = 32803,
    GPG_ERR_EFAULT = 32804,
    GPG_ERR_EFBIG = 32805,
    GPG_ERR_EFTYPE = 32806,
    GPG_ERR_EGRATUITOUS = 32807,
    GPG_ERR_EGREGIOUS = 32808,
    GPG_ERR_EHOSTDOWN = 32809,
    GPG_ERR_EHOSTUNREACH = 32810,
    GPG_ERR_EIDRM = 32811,
    GPG_ERR_EIEIO = 32812,
    GPG_ERR_EILSEQ = 32813,
    GPG_ERR_EINPROGRESS = 32814,
    GPG_ERR_EINTR = 32815,
    GPG_ERR_EINVAL = 32816,
    GPG_ERR_EIO = 32817,
    GPG_ERR_EISCONN = 32818,
    GPG_ERR_EISDIR = 32819,
    GPG_ERR_EISNAM = 32820,
    GPG_ERR_EL2HLT = 32821,
    GPG_ERR_EL2NSYNC = 32822,
    GPG_ERR_EL3HLT = 32823,
    GPG_ERR_EL3RST = 32824,
    GPG_ERR_ELIBACC = 32825,
    GPG_ERR_ELIBBAD = 32826,
    GPG_ERR_ELIBEXEC = 32827,
    GPG_ERR_ELIBMAX = 32828,
    GPG_ERR_ELIBSCN = 32829,
    GPG_ERR_ELNRNG = 32830,
    GPG_ERR_ELOOP = 32831,
    GPG_ERR_EMEDIUMTYPE = 32832,
    GPG_ERR_EMFILE = 32833,
    GPG_ERR_EMLINK = 32834,
    GPG_ERR_EMSGSIZE = 32835,
    GPG_ERR_EMULTIHOP = 32836,
    GPG_ERR_ENAMETOOLONG = 32837,
    GPG_ERR_ENAVAIL = 32838,
    GPG_ERR_ENEEDAUTH = 32839,
    GPG_ERR_ENETDOWN = 32840,
    GPG_ERR_ENETRESET = 32841,
    GPG_ERR_ENETUNREACH = 32842,
    GPG_ERR_ENFILE = 32843,
    GPG_ERR_ENOANO = 32844,
    GPG_ERR_ENOBUFS = 32845,
    GPG_ERR_ENOCSI = 32846,
    GPG_ERR_ENODATA = 32847,
    GPG_ERR_ENODEV = 32848,
    GPG_ERR_ENOENT = 32849,
    GPG_ERR_ENOEXEC = 32850,
    GPG_ERR_ENOLCK = 32851,
    GPG_ERR_ENOLINK = 32852,
    GPG_ERR_ENOMEDIUM = 32853,
    GPG_ERR_ENOMEM = 32854,
    GPG_ERR_ENOMSG = 32855,
    GPG_ERR_ENONET = 32856,
    GPG_ERR_ENOPKG = 32857,
    GPG_ERR_ENOPROTOOPT = 32858,
    GPG_ERR_ENOSPC = 32859,
    GPG_ERR_ENOSR = 32860,
    GPG_ERR_ENOSTR = 32861,
    GPG_ERR_ENOSYS = 32862,
    GPG_ERR_ENOTBLK = 32863,
    GPG_ERR_ENOTCONN = 32864,
    GPG_ERR_ENOTDIR = 32865,
    GPG_ERR_ENOTEMPTY = 32866,
    GPG_ERR_ENOTNAM = 32867,
    GPG_ERR_ENOTSOCK = 32868,
    GPG_ERR_ENOTSUP = 32869,
    GPG_ERR_ENOTTY = 32870,
    GPG_ERR_ENOTUNIQ = 32871,
    GPG_ERR_ENXIO = 32872,
    GPG_ERR_EOPNOTSUPP = 32873,
    GPG_ERR_EOVERFLOW = 32874,
    GPG_ERR_EPERM = 32875,
    GPG_ERR_EPFNOSUPPORT = 32876,
    GPG_ERR_EPIPE = 32877,
    GPG_ERR_EPROCLIM = 32878,
    GPG_ERR_EPROCUNAVAIL = 32879,
    GPG_ERR_EPROGMISMATCH = 32880,
    GPG_ERR_EPROGUNAVAIL = 32881,
    GPG_ERR_EPROTO = 32882,
    GPG_ERR_EPROTONOSUPPORT = 32883,
    GPG_ERR_EPROTOTYPE = 32884,
    GPG_ERR_ERANGE = 32885,
    GPG_ERR_EREMCHG = 32886,
    GPG_ERR_EREMOTE = 32887,
    GPG_ERR_EREMOTEIO = 32888,
    GPG_ERR_ERESTART = 32889,
    GPG_ERR_EROFS = 32890,
    GPG_ERR_ERPCMISMATCH = 32891,
    GPG_ERR_ESHUTDOWN = 32892,
    GPG_ERR_ESOCKTNOSUPPORT = 32893,
    GPG_ERR_ESPIPE = 32894,
    GPG_ERR_ESRCH = 32895,
    GPG_ERR_ESRMNT = 32896,
    GPG_ERR_ESTALE = 32897,
    GPG_ERR_ESTRPIPE = 32898,
    GPG_ERR_ETIME = 32899,
    GPG_ERR_ETIMEDOUT = 32900,
    GPG_ERR_ETOOMANYREFS = 32901,
    GPG_ERR_ETXTBSY = 32902,
    GPG_ERR_EUCLEAN = 32903,
    GPG_ERR_EUNATCH = 32904,
    GPG_ERR_EUSERS = 32905,
    GPG_ERR_EWOULDBLOCK = 32906,
    GPG_ERR_EXDEV = 32907,
    GPG_ERR_EXFULL = 32908,
    GPG_ERR_CODE_DIM = 65536
} ;
# 110 "/usr/include/gpg-error.h"
typedef enum __anonenum_gpg_err_code_t_22 gpg_err_code_t;
# 513 "/usr/include/gpg-error.h"
typedef unsigned int gpg_error_t;
# 44 "/usr/include/x86_64-linux-gnu/bits/uio.h"
struct iovec {
   void *iov_base ;
   size_t iov_len ;
};
# 35 "/usr/include/x86_64-linux-gnu/bits/socket.h"
typedef __socklen_t socklen_t;
# 40 "/usr/include/x86_64-linux-gnu/bits/socket.h"
enum __socket_type {
    SOCK_STREAM = 1,
    SOCK_DGRAM = 2,
    SOCK_RAW = 3,
    SOCK_RDM = 4,
    SOCK_SEQPACKET = 5,
    SOCK_DCCP = 6,
    SOCK_PACKET = 10,
    SOCK_CLOEXEC = 524288,
    SOCK_NONBLOCK = 2048
} ;
# 29 "/usr/include/x86_64-linux-gnu/bits/sockaddr.h"
typedef unsigned short sa_family_t;
# 180 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct sockaddr {
   sa_family_t sa_family ;
   char sa_data[14] ;
};
# 193 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct sockaddr_storage {
   sa_family_t ss_family ;
   unsigned long __ss_align ;
   char __ss_padding[(unsigned long )128 - (unsigned long )((unsigned long )2 * (unsigned long )sizeof(unsigned long ))] ;
};
# 202 "/usr/include/x86_64-linux-gnu/bits/socket.h"
enum __anonenum_23 {
    MSG_OOB = 1,
    MSG_PEEK = 2,
    MSG_DONTROUTE = 4,
    MSG_CTRUNC = 8,
    MSG_PROXY = 16,
    MSG_TRUNC = 32,
    MSG_DONTWAIT = 64,
    MSG_EOR = 128,
    MSG_WAITALL = 256,
    MSG_FIN = 512,
    MSG_SYN = 1024,
    MSG_CONFIRM = 2048,
    MSG_RST = 4096,
    MSG_ERRQUEUE = 8192,
    MSG_NOSIGNAL = 16384,
    MSG_MORE = 32768,
    MSG_WAITFORONE = 65536,
    MSG_CMSG_CLOEXEC = 1073741824
} ;
# 253 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct msghdr {
   void *msg_name ;
   socklen_t msg_namelen ;
   struct iovec *msg_iov ;
   size_t msg_iovlen ;
   void *msg_control ;
   size_t msg_controllen ;
   int msg_flags ;
};
# 280 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct cmsghdr {
   size_t cmsg_len ;
   int cmsg_level ;
   int cmsg_type ;
   unsigned char __cmsg_data[] ;
};
# 337 "/usr/include/x86_64-linux-gnu/bits/socket.h"
enum __anonenum_24 {
    SCM_RIGHTS = 1
} ;
# 417 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct linger {
   int l_onoff ;
   int l_linger ;
};
# 431 "/usr/include/x86_64-linux-gnu/bits/socket.h"
struct mmsghdr;
# 45 "/usr/include/x86_64-linux-gnu/sys/socket.h"
struct osockaddr {
   unsigned short sa_family ;
   unsigned char sa_data[14] ;
};
# 54 "/usr/include/x86_64-linux-gnu/sys/socket.h"
enum __anonenum_25 {
    SHUT_RD = 0,
    SHUT_WR = 1,
    SHUT_RDWR = 2
} ;
# 57 "/usr/include/x86_64-linux-gnu/sys/time.h"
struct timezone {
   int tz_minuteswest ;
   int tz_dsttime ;
};
# 63 "/usr/include/x86_64-linux-gnu/sys/time.h"
typedef struct timezone * __restrict __timezone_ptr_t;
# 93 "/usr/include/x86_64-linux-gnu/sys/time.h"
enum __itimer_which {
    ITIMER_REAL = 0,
    ITIMER_VIRTUAL = 1,
    ITIMER_PROF = 2
} ;
# 109 "/usr/include/x86_64-linux-gnu/sys/time.h"
struct itimerval {
   struct timeval it_interval ;
   struct timeval it_value ;
};
# 122 "/usr/include/x86_64-linux-gnu/sys/time.h"
typedef int __itimer_which_t;
# 47 "/usr/include/gcrypt.h"
typedef socklen_t gcry_socklen_t;
# 117 "/usr/include/gcrypt.h"
typedef gpg_error_t gcry_error_t;
# 118 "/usr/include/gcrypt.h"
typedef gpg_err_code_t gcry_err_code_t;
# 119 "/usr/include/gcrypt.h"
typedef gpg_err_source_t gcry_err_source_t;
# 179 "/usr/include/gcrypt.h"
enum gcry_thread_option {
    _GCRY_THREAD_OPTION_DUMMY = 0
} __attribute__((__deprecated__)) ;
# 197 "/usr/include/gcrypt.h"
struct gcry_thread_cbs {
   unsigned int option ;
   int (*init)(void) ;
   int (*mutex_init)(void **priv ) ;
   int (*mutex_destroy)(void **priv ) ;
   int (*mutex_lock)(void **priv ) ;
   int (*mutex_unlock)(void **priv ) ;
   ssize_t (*read)(int fd , void *buf , size_t nbytes ) ;
   ssize_t (*write)(int fd , void const *buf , size_t nbytes ) ;
   ssize_t (*select)(int nfd , fd_set *rset , fd_set *wset , fd_set *eset , struct timeval *timeout ) ;
   ssize_t (*waitpid)(pid_t pid , int *status , int options ) ;
   int (*accept)(int s , struct sockaddr *addr , gcry_socklen_t *length_ptr ) ;
   int (*connect)(int s , struct sockaddr *addr , gcry_socklen_t length ) ;
   int (*sendmsg)(int s , struct msghdr const *msg , int flags ) ;
   int (*recvmsg)(int s , struct msghdr *msg , int flags ) ;
};
# 343 "/usr/include/gcrypt.h"
struct gcry_mpi;
# 343 "/usr/include/gcrypt.h"
struct gcry_mpi;
# 344 "/usr/include/gcrypt.h"
typedef struct gcry_mpi *gcry_mpi_t;
# 347 "/usr/include/gcrypt.h"
typedef struct gcry_mpi * __attribute__((__deprecated__)) GCRY_MPI;
# 348 "/usr/include/gcrypt.h"
typedef struct gcry_mpi * __attribute__((__deprecated__)) GcryMPI;
# 359 "/usr/include/gcrypt.h"
enum gcry_ctl_cmds {
    GCRYCTL_SET_KEY = 1,
    GCRYCTL_SET_IV = 2,
    GCRYCTL_CFB_SYNC = 3,
    GCRYCTL_RESET = 4,
    GCRYCTL_FINALIZE = 5,
    GCRYCTL_GET_KEYLEN = 6,
    GCRYCTL_GET_BLKLEN = 7,
    GCRYCTL_TEST_ALGO = 8,
    GCRYCTL_IS_SECURE = 9,
    GCRYCTL_GET_ASNOID = 10,
    GCRYCTL_ENABLE_ALGO = 11,
    GCRYCTL_DISABLE_ALGO = 12,
    GCRYCTL_DUMP_RANDOM_STATS = 13,
    GCRYCTL_DUMP_SECMEM_STATS = 14,
    GCRYCTL_GET_ALGO_NPKEY = 15,
    GCRYCTL_GET_ALGO_NSKEY = 16,
    GCRYCTL_GET_ALGO_NSIGN = 17,
    GCRYCTL_GET_ALGO_NENCR = 18,
    GCRYCTL_SET_VERBOSITY = 19,
    GCRYCTL_SET_DEBUG_FLAGS = 20,
    GCRYCTL_CLEAR_DEBUG_FLAGS = 21,
    GCRYCTL_USE_SECURE_RNDPOOL = 22,
    GCRYCTL_DUMP_MEMORY_STATS = 23,
    GCRYCTL_INIT_SECMEM = 24,
    GCRYCTL_TERM_SECMEM = 25,
    GCRYCTL_DISABLE_SECMEM_WARN = 27,
    GCRYCTL_SUSPEND_SECMEM_WARN = 28,
    GCRYCTL_RESUME_SECMEM_WARN = 29,
    GCRYCTL_DROP_PRIVS = 30,
    GCRYCTL_ENABLE_M_GUARD = 31,
    GCRYCTL_START_DUMP = 32,
    GCRYCTL_STOP_DUMP = 33,
    GCRYCTL_GET_ALGO_USAGE = 34,
    GCRYCTL_IS_ALGO_ENABLED = 35,
    GCRYCTL_DISABLE_INTERNAL_LOCKING = 36,
    GCRYCTL_DISABLE_SECMEM = 37,
    GCRYCTL_INITIALIZATION_FINISHED = 38,
    GCRYCTL_INITIALIZATION_FINISHED_P = 39,
    GCRYCTL_ANY_INITIALIZATION_P = 40,
    GCRYCTL_SET_CBC_CTS = 41,
    GCRYCTL_SET_CBC_MAC = 42,
    GCRYCTL_SET_CTR = 43,
    GCRYCTL_ENABLE_QUICK_RANDOM = 44,
    GCRYCTL_SET_RANDOM_SEED_FILE = 45,
    GCRYCTL_UPDATE_RANDOM_SEED_FILE = 46,
    GCRYCTL_SET_THREAD_CBS = 47,
    GCRYCTL_FAST_POLL = 48,
    GCRYCTL_SET_RANDOM_DAEMON_SOCKET = 49,
    GCRYCTL_USE_RANDOM_DAEMON = 50,
    GCRYCTL_FAKED_RANDOM_P = 51,
    GCRYCTL_SET_RNDEGD_SOCKET = 52,
    GCRYCTL_PRINT_CONFIG = 53,
    GCRYCTL_OPERATIONAL_P = 54,
    GCRYCTL_FIPS_MODE_P = 55,
    GCRYCTL_FORCE_FIPS_MODE = 56,
    GCRYCTL_SELFTEST = 57,
    GCRYCTL_DISABLE_HWF = 63
} ;
# 429 "/usr/include/gcrypt.h"
struct gcry_sexp;
# 429 "/usr/include/gcrypt.h"
struct gcry_sexp;
# 430 "/usr/include/gcrypt.h"
typedef struct gcry_sexp *gcry_sexp_t;
# 433 "/usr/include/gcrypt.h"
typedef struct gcry_sexp * __attribute__((__deprecated__)) GCRY_SEXP;
# 434 "/usr/include/gcrypt.h"
typedef struct gcry_sexp * __attribute__((__deprecated__)) GcrySexp;
# 438 "/usr/include/gcrypt.h"
enum gcry_sexp_format {
    GCRYSEXP_FMT_DEFAULT = 0,
    GCRYSEXP_FMT_CANON = 1,
    GCRYSEXP_FMT_BASE64 = 2,
    GCRYSEXP_FMT_ADVANCED = 3
} ;
# 561 "/usr/include/gcrypt.h"
enum gcry_mpi_format {
    GCRYMPI_FMT_NONE = 0,
    GCRYMPI_FMT_STD = 1,
    GCRYMPI_FMT_PGP = 2,
    GCRYMPI_FMT_SSH = 3,
    GCRYMPI_FMT_HEX = 4,
    GCRYMPI_FMT_USG = 5
} ;
# 572 "/usr/include/gcrypt.h"
enum gcry_mpi_flag {
    GCRYMPI_FLAG_SECURE = 1,
    GCRYMPI_FLAG_OPAQUE = 2
} ;
# 801 "/usr/include/gcrypt.h"
struct gcry_cipher_handle;
# 801 "/usr/include/gcrypt.h"
struct gcry_cipher_handle;
# 802 "/usr/include/gcrypt.h"
typedef struct gcry_cipher_handle *gcry_cipher_hd_t;
# 805 "/usr/include/gcrypt.h"
typedef struct gcry_cipher_handle * __attribute__((__deprecated__)) GCRY_CIPHER_HD;
# 806 "/usr/include/gcrypt.h"
typedef struct gcry_cipher_handle * __attribute__((__deprecated__)) GcryCipherHd;
# 811 "/usr/include/gcrypt.h"
enum gcry_cipher_algos {
    GCRY_CIPHER_NONE = 0,
    GCRY_CIPHER_IDEA = 1,
    GCRY_CIPHER_3DES = 2,
    GCRY_CIPHER_CAST5 = 3,
    GCRY_CIPHER_BLOWFISH = 4,
    GCRY_CIPHER_SAFER_SK128 = 5,
    GCRY_CIPHER_DES_SK = 6,
    GCRY_CIPHER_AES = 7,
    GCRY_CIPHER_AES192 = 8,
    GCRY_CIPHER_AES256 = 9,
    GCRY_CIPHER_TWOFISH = 10,
    GCRY_CIPHER_ARCFOUR = 301,
    GCRY_CIPHER_DES = 302,
    GCRY_CIPHER_TWOFISH128 = 303,
    GCRY_CIPHER_SERPENT128 = 304,
    GCRY_CIPHER_SERPENT192 = 305,
    GCRY_CIPHER_SERPENT256 = 306,
    GCRY_CIPHER_RFC2268_40 = 307,
    GCRY_CIPHER_RFC2268_128 = 308,
    GCRY_CIPHER_SEED = 309,
    GCRY_CIPHER_CAMELLIA128 = 310,
    GCRY_CIPHER_CAMELLIA192 = 311,
    GCRY_CIPHER_CAMELLIA256 = 312
} ;
# 849 "/usr/include/gcrypt.h"
enum gcry_cipher_modes {
    GCRY_CIPHER_MODE_NONE = 0,
    GCRY_CIPHER_MODE_ECB = 1,
    GCRY_CIPHER_MODE_CFB = 2,
    GCRY_CIPHER_MODE_CBC = 3,
    GCRY_CIPHER_MODE_STREAM = 4,
    GCRY_CIPHER_MODE_OFB = 5,
    GCRY_CIPHER_MODE_CTR = 6,
    GCRY_CIPHER_MODE_AESWRAP = 7
} ;
# 862 "/usr/include/gcrypt.h"
enum gcry_cipher_flags {
    GCRY_CIPHER_SECURE = 1,
    GCRY_CIPHER_ENABLE_SYNC = 2,
    GCRY_CIPHER_CBC_CTS = 4,
    GCRY_CIPHER_CBC_MAC = 8
} ;
# 970 "/usr/include/gcrypt.h"
enum gcry_pk_algos {
    GCRY_PK_RSA = 1,
    GCRY_PK_RSA_E = 2,
    GCRY_PK_RSA_S = 3,
    GCRY_PK_ELG_E = 16,
    GCRY_PK_DSA = 17,
    GCRY_PK_ELG = 20,
    GCRY_PK_ECDSA = 301,
    GCRY_PK_ECDH = 302
} ;
# 1070 "/usr/include/gcrypt.h"
enum gcry_md_algos {
    GCRY_MD_NONE = 0,
    GCRY_MD_MD5 = 1,
    GCRY_MD_SHA1 = 2,
    GCRY_MD_RMD160 = 3,
    GCRY_MD_MD2 = 5,
    GCRY_MD_TIGER = 6,
    GCRY_MD_HAVAL = 7,
    GCRY_MD_SHA256 = 8,
    GCRY_MD_SHA384 = 9,
    GCRY_MD_SHA512 = 10,
    GCRY_MD_SHA224 = 11,
    GCRY_MD_MD4 = 301,
    GCRY_MD_CRC32 = 302,
    GCRY_MD_CRC32_RFC1510 = 303,
    GCRY_MD_CRC24_RFC2440 = 304,
    GCRY_MD_WHIRLPOOL = 305,
    GCRY_MD_TIGER1 = 306,
    GCRY_MD_TIGER2 = 307
} ;
# 1093 "/usr/include/gcrypt.h"
enum gcry_md_flags {
    GCRY_MD_FLAG_SECURE = 1,
    GCRY_MD_FLAG_HMAC = 2
} ;
# 1100 "/usr/include/gcrypt.h"
struct gcry_md_context;
# 1100 "/usr/include/gcrypt.h"
struct gcry_md_context;
# 1105 "/usr/include/gcrypt.h"
struct gcry_md_handle {
   struct gcry_md_context *ctx ;
   int bufpos ;
   int bufsize ;
   unsigned char buf[1] ;
};
# 1105 "/usr/include/gcrypt.h"
typedef struct gcry_md_handle *gcry_md_hd_t;
# 1118 "/usr/include/gcrypt.h"
typedef struct gcry_md_handle * __attribute__((__deprecated__)) GCRY_MD_HD;
# 1119 "/usr/include/gcrypt.h"
typedef struct gcry_md_handle * __attribute__((__deprecated__)) GcryMDHd;
# 1256 "/usr/include/gcrypt.h"
enum gcry_ac_id {
    GCRY_AC_RSA = 1,
    GCRY_AC_DSA = 17,
    GCRY_AC_ELG = 20,
    GCRY_AC_ELG_E = 16
} ;
# 1256 "/usr/include/gcrypt.h"
typedef enum gcry_ac_id __attribute__((__deprecated__)) gcry_ac_id_t;
# 1266 "/usr/include/gcrypt.h"
enum gcry_ac_key_type {
    GCRY_AC_KEY_SECRET = 0,
    GCRY_AC_KEY_PUBLIC = 1
} ;
# 1266 "/usr/include/gcrypt.h"
typedef enum gcry_ac_key_type __attribute__((__deprecated__)) gcry_ac_key_type_t;
# 1274 "/usr/include/gcrypt.h"
enum gcry_ac_em {
    GCRY_AC_EME_PKCS_V1_5 = 0,
    GCRY_AC_EMSA_PKCS_V1_5 = 1
} ;
# 1274 "/usr/include/gcrypt.h"
typedef enum gcry_ac_em __attribute__((__deprecated__)) gcry_ac_em_t;
# 1282 "/usr/include/gcrypt.h"
enum gcry_ac_scheme {
    GCRY_AC_ES_PKCS_V1_5 = 0,
    GCRY_AC_SSA_PKCS_V1_5 = 1
} ;
# 1282 "/usr/include/gcrypt.h"
typedef enum gcry_ac_scheme __attribute__((__deprecated__)) gcry_ac_scheme_t;
# 1295 "/usr/include/gcrypt.h"
struct gcry_ac_data;
# 1295 "/usr/include/gcrypt.h"
typedef struct gcry_ac_data * __attribute__((__deprecated__)) gcry_ac_data_t;
# 1299 "/usr/include/gcrypt.h"
struct gcry_ac_key;
# 1299 "/usr/include/gcrypt.h"
typedef struct gcry_ac_key * __attribute__((__deprecated__)) gcry_ac_key_t;
# 1303 "/usr/include/gcrypt.h"
struct gcry_ac_key_pair;
# 1303 "/usr/include/gcrypt.h"
typedef struct gcry_ac_key_pair * __attribute__((__deprecated__)) gcry_ac_key_pair_t;
# 1307 "/usr/include/gcrypt.h"
struct gcry_ac_handle;
# 1307 "/usr/include/gcrypt.h"
typedef struct gcry_ac_handle * __attribute__((__deprecated__)) gcry_ac_handle_t;
# 1309 "/usr/include/gcrypt.h"
typedef gpg_error_t (* __attribute__((__deprecated__)) gcry_ac_data_read_cb_t)(void *opaque ,
                                                                               unsigned char *buffer ,
                                                                               size_t *buffer_n );
# 1314 "/usr/include/gcrypt.h"
typedef gpg_error_t (* __attribute__((__deprecated__)) gcry_ac_data_write_cb_t)(void *opaque ,
                                                                                unsigned char *buffer ,
                                                                                size_t buffer_n );
# 1319 "/usr/include/gcrypt.h"
enum __anonenum_gcry_ac_io_mode_t_26 {
    GCRY_AC_IO_READABLE = 0,
    GCRY_AC_IO_WRITABLE = 1
} ;
# 1319 "/usr/include/gcrypt.h"
typedef enum __anonenum_gcry_ac_io_mode_t_26 __attribute__((__deprecated__)) gcry_ac_io_mode_t;
# 1326 "/usr/include/gcrypt.h"
enum __anonenum_gcry_ac_io_type_t_27 {
    GCRY_AC_IO_STRING = 0,
    GCRY_AC_IO_CALLBACK = 1
} ;
# 1326 "/usr/include/gcrypt.h"
typedef enum __anonenum_gcry_ac_io_type_t_27 __attribute__((__deprecated__)) gcry_ac_io_type_t;
# 1333 "/usr/include/gcrypt.h"
struct __anonstruct_callback_30 {
   gpg_error_t (* __attribute__((__deprecated__)) cb)(void *opaque , unsigned char *buffer ,
                                                      size_t *buffer_n ) ;
   void *opaque ;
};
# 1333 "/usr/include/gcrypt.h"
struct __anonstruct_string_31 {
   unsigned char *data ;
   size_t data_n ;
};
# 1333 "/usr/include/gcrypt.h"
union __anonunion_readable_29 {
   struct __anonstruct_callback_30 callback ;
   struct __anonstruct_string_31 string ;
   void *opaque ;
};
# 1333 "/usr/include/gcrypt.h"
struct __anonstruct_callback_33 {
   gpg_error_t (* __attribute__((__deprecated__)) cb)(void *opaque , unsigned char *buffer ,
                                                      size_t buffer_n ) ;
   void *opaque ;
};
# 1333 "/usr/include/gcrypt.h"
struct __anonstruct_string_34 {
   unsigned char **data ;
   size_t *data_n ;
};
# 1333 "/usr/include/gcrypt.h"
union __anonunion_writable_32 {
   struct __anonstruct_callback_33 callback ;
   struct __anonstruct_string_34 string ;
   void *opaque ;
};
# 1333 "/usr/include/gcrypt.h"
union __anonunion_io_28 {
   union __anonunion_readable_29 readable ;
   union __anonunion_writable_32 writable ;
};
# 1333 "/usr/include/gcrypt.h"
struct gcry_ac_io {
   gcry_ac_io_mode_t mode __attribute__((__deprecated__)) ;
   gcry_ac_io_type_t type __attribute__((__deprecated__)) ;
   union __anonunion_io_28 io __attribute__((__deprecated__)) ;
};
# 1333 "/usr/include/gcrypt.h"
typedef struct gcry_ac_io __attribute__((__deprecated__)) gcry_ac_io_t;
# 1375 "/usr/include/gcrypt.h"
struct gcry_ac_key_spec_rsa {
   gcry_mpi_t e ;
};
# 1375 "/usr/include/gcrypt.h"
typedef struct gcry_ac_key_spec_rsa __attribute__((__deprecated__)) gcry_ac_key_spec_rsa_t;
# 1382 "/usr/include/gcrypt.h"
struct gcry_ac_eme_pkcs_v1_5 {
   size_t key_size ;
};
# 1382 "/usr/include/gcrypt.h"
typedef struct gcry_ac_eme_pkcs_v1_5 __attribute__((__deprecated__)) gcry_ac_eme_pkcs_v1_5_t;
# 1387 "/usr/include/gcrypt.h"
typedef enum gcry_md_algos __attribute__((__deprecated__)) gcry_md_algo_t;
# 1391 "/usr/include/gcrypt.h"
struct gcry_ac_emsa_pkcs_v1_5 {
   gcry_md_algo_t md ;
   size_t em_n ;
};
# 1391 "/usr/include/gcrypt.h"
typedef struct gcry_ac_emsa_pkcs_v1_5 __attribute__((__deprecated__)) gcry_ac_emsa_pkcs_v1_5_t;
# 1399 "/usr/include/gcrypt.h"
struct gcry_ac_ssa_pkcs_v1_5 {
   gcry_md_algo_t md ;
};
# 1399 "/usr/include/gcrypt.h"
typedef struct gcry_ac_ssa_pkcs_v1_5 __attribute__((__deprecated__)) gcry_ac_ssa_pkcs_v1_5_t;
# 1668 "/usr/include/gcrypt.h"
enum gcry_kdf_algos {
    GCRY_KDF_NONE = 0,
    GCRY_KDF_SIMPLE_S2K = 16,
    GCRY_KDF_SALTED_S2K = 17,
    GCRY_KDF_ITERSALTED_S2K = 19,
    GCRY_KDF_PBKDF1 = 33,
    GCRY_KDF_PBKDF2 = 34
} ;
# 1698 "/usr/include/gcrypt.h"
enum gcry_random_level {
    GCRY_WEAK_RANDOM = 0,
    GCRY_STRONG_RANDOM = 1,
    GCRY_VERY_STRONG_RANDOM = 2
} ;
# 1698 "/usr/include/gcrypt.h"
typedef enum gcry_random_level gcry_random_level_t;
# 1762 "/usr/include/gcrypt.h"
typedef int (*gcry_prime_check_func_t)(void *arg , int mode , gcry_mpi_t candidate );
# 1815 "/usr/include/gcrypt.h"
enum gcry_log_levels {
    GCRY_LOG_CONT = 0,
    GCRY_LOG_INFO = 10,
    GCRY_LOG_WARN = 20,
    GCRY_LOG_ERROR = 30,
    GCRY_LOG_FATAL = 40,
    GCRY_LOG_BUG = 50,
    GCRY_LOG_DEBUG = 100
} ;
# 1827 "/usr/include/gcrypt.h"
typedef void (*gcry_handler_progress_t)(void * , char const * , int , int , int );
# 1830 "/usr/include/gcrypt.h"
typedef void *(*gcry_handler_alloc_t)(size_t n );
# 1833 "/usr/include/gcrypt.h"
typedef int (*gcry_handler_secure_check_t)(void const * );
# 1836 "/usr/include/gcrypt.h"
typedef void *(*gcry_handler_realloc_t)(void *p , size_t n );
# 1839 "/usr/include/gcrypt.h"
typedef void (*gcry_handler_free_t)(void * );
# 1842 "/usr/include/gcrypt.h"
typedef int (*gcry_handler_no_mem_t)(void * , size_t , unsigned int );
# 1845 "/usr/include/gcrypt.h"
typedef void (*gcry_handler_error_t)(void * , int , char const * );
# 1848 "/usr/include/gcrypt.h"
typedef void (*gcry_handler_log_t)(void * , int , char const * , va_list );
# 43 "/usr/include/gcrypt-module.h"
struct gcry_module;
# 43 "/usr/include/gcrypt-module.h"
typedef struct gcry_module *gcry_module_t;
# 48 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_cipher_setkey_t)(void *c , unsigned char const *key ,
                                                unsigned int keylen );
# 53 "/usr/include/gcrypt-module.h"
typedef void (*gcry_cipher_encrypt_t)(void *c , unsigned char *outbuf , unsigned char const *inbuf );
# 58 "/usr/include/gcrypt-module.h"
typedef void (*gcry_cipher_decrypt_t)(void *c , unsigned char *outbuf , unsigned char const *inbuf );
# 63 "/usr/include/gcrypt-module.h"
typedef void (*gcry_cipher_stencrypt_t)(void *c , unsigned char *outbuf , unsigned char const *inbuf ,
                                        unsigned int n );
# 69 "/usr/include/gcrypt-module.h"
typedef void (*gcry_cipher_stdecrypt_t)(void *c , unsigned char *outbuf , unsigned char const *inbuf ,
                                        unsigned int n );
# 74 "/usr/include/gcrypt-module.h"
struct gcry_cipher_oid_spec {
   char const *oid ;
   int mode ;
};
# 74 "/usr/include/gcrypt-module.h"
typedef struct gcry_cipher_oid_spec gcry_cipher_oid_spec_t;
# 81 "/usr/include/gcrypt-module.h"
struct gcry_cipher_spec {
   char const *name ;
   char const **aliases ;
   gcry_cipher_oid_spec_t *oids ;
   size_t blocksize ;
   size_t keylen ;
   size_t contextsize ;
   gcry_err_code_t (*setkey)(void *c , unsigned char const *key , unsigned int keylen ) ;
   void (*encrypt)(void *c , unsigned char *outbuf , unsigned char const *inbuf ) ;
   void (*decrypt)(void *c , unsigned char *outbuf , unsigned char const *inbuf ) ;
   void (*stencrypt)(void *c , unsigned char *outbuf , unsigned char const *inbuf ,
                     unsigned int n ) ;
   void (*stdecrypt)(void *c , unsigned char *outbuf , unsigned char const *inbuf ,
                     unsigned int n ) ;
};
# 81 "/usr/include/gcrypt-module.h"
typedef struct gcry_cipher_spec gcry_cipher_spec_t;
# 113 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_generate_t)(int algo , unsigned int nbits , unsigned long use_e ,
                                              gcry_mpi_t *skey , gcry_mpi_t **retfactors );
# 120 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_check_secret_key_t)(int algo , gcry_mpi_t *skey );
# 124 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_encrypt_t)(int algo , gcry_mpi_t *resarr , gcry_mpi_t data ,
                                             gcry_mpi_t *pkey , int flags );
# 131 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_decrypt_t)(int algo , gcry_mpi_t *result , gcry_mpi_t *data ,
                                             gcry_mpi_t *skey , int flags );
# 138 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_sign_t)(int algo , gcry_mpi_t *resarr , gcry_mpi_t data ,
                                          gcry_mpi_t *skey );
# 144 "/usr/include/gcrypt-module.h"
typedef gcry_err_code_t (*gcry_pk_verify_t)(int algo , gcry_mpi_t hash , gcry_mpi_t *data ,
                                            gcry_mpi_t *pkey , int (*cmp)(void * ,
                                                                          gcry_mpi_t ) ,
                                            void *opaquev );
# 152 "/usr/include/gcrypt-module.h"
typedef unsigned int (*gcry_pk_get_nbits_t)(int algo , gcry_mpi_t *pkey );
# 155 "/usr/include/gcrypt-module.h"
struct gcry_pk_spec {
   char const *name ;
   char const **aliases ;
   char const *elements_pkey ;
   char const *elements_skey ;
   char const *elements_enc ;
   char const *elements_sig ;
   char const *elements_grip ;
   int use ;
   gcry_err_code_t (*generate)(int algo , unsigned int nbits , unsigned long use_e ,
                               gcry_mpi_t *skey , gcry_mpi_t **retfactors ) ;
   gcry_err_code_t (*check_secret_key)(int algo , gcry_mpi_t *skey ) ;
   gcry_err_code_t (*encrypt)(int algo , gcry_mpi_t *resarr , gcry_mpi_t data , gcry_mpi_t *pkey ,
                              int flags ) ;
   gcry_err_code_t (*decrypt)(int algo , gcry_mpi_t *result , gcry_mpi_t *data , gcry_mpi_t *skey ,
                              int flags ) ;
   gcry_err_code_t (*sign)(int algo , gcry_mpi_t *resarr , gcry_mpi_t data , gcry_mpi_t *skey ) ;
   gcry_err_code_t (*verify)(int algo , gcry_mpi_t hash , gcry_mpi_t *data , gcry_mpi_t *pkey ,
                             int (*cmp)(void * , gcry_mpi_t ) , void *opaquev ) ;
   unsigned int (*get_nbits)(int algo , gcry_mpi_t *pkey ) ;
};
# 155 "/usr/include/gcrypt-module.h"
typedef struct gcry_pk_spec gcry_pk_spec_t;
# 190 "/usr/include/gcrypt-module.h"
typedef void (*gcry_md_init_t)(void *c );
# 193 "/usr/include/gcrypt-module.h"
typedef void (*gcry_md_write_t)(void *c , void const *buf , size_t nbytes );
# 196 "/usr/include/gcrypt-module.h"
typedef void (*gcry_md_final_t)(void *c );
# 199 "/usr/include/gcrypt-module.h"
typedef unsigned char *(*gcry_md_read_t)(void *c );
# 201 "/usr/include/gcrypt-module.h"
struct gcry_md_oid_spec {
   char const *oidstring ;
};
# 201 "/usr/include/gcrypt-module.h"
typedef struct gcry_md_oid_spec gcry_md_oid_spec_t;
# 207 "/usr/include/gcrypt-module.h"
struct gcry_md_spec {
   char const *name ;
   unsigned char *asnoid ;
   int asnlen ;
   gcry_md_oid_spec_t *oids ;
   int mdlen ;
   void (*init)(void *c ) ;
   void (*write)(void *c , void const *buf , size_t nbytes ) ;
   void (*final)(void *c ) ;
   unsigned char *(*read)(void *c ) ;
   size_t contextsize ;
};
# 207 "/usr/include/gcrypt-module.h"
typedef struct gcry_md_spec gcry_md_spec_t;
# 9 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef long long widest_t;
# 17 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct ProtocolDesc;
# 17 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct ProtocolDesc ProtocolDesc;
# 18 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct OblivInputs;
# 18 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct OblivInputs OblivInputs;
# 19 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct OblivBit;
# 19 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct OblivBit OblivBit;
# 28 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef char yao_key_t[11];
# 30 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct ProtocolTransport;
# 30 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct __anonstruct_debug_36 {
   unsigned int mulCount ;
   unsigned int xorCount ;
};
# 30 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
union __anonunion____missing_field_name_35 {
   struct __anonstruct_debug_36 debug ;
};
# 30 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct ProtocolDesc {
   int partyCount ;
   int thisParty ;
   struct ProtocolTransport *trans ;
   union __anonunion____missing_field_name_35 __annonCompField1 ;
   void (*feedOblivInputs)(ProtocolDesc * , OblivInputs * , size_t , int ) ;
   _Bool (*revealOblivBits)(ProtocolDesc * , widest_t * , OblivBit const * , size_t ,
                            int ) ;
   void (*setBitAnd)(ProtocolDesc * , OblivBit * , OblivBit const * , OblivBit const * ) ;
   void (*setBitOr)(ProtocolDesc * , OblivBit * , OblivBit const * , OblivBit const * ) ;
   void (*setBitXor)(ProtocolDesc * , OblivBit * , OblivBit const * , OblivBit const * ) ;
   void (*setBitNot)(ProtocolDesc * , OblivBit * , OblivBit const * ) ;
   void (*flipBit)(ProtocolDesc * , OblivBit * ) ;
   void *extra ;
};
# 52 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct __anonstruct_OTsender_37 {
   void *sender ;
   void (*send)(void * , char const * , char const * , int n , int len ) ;
   void (*release)(void * ) ;
};
# 52 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct __anonstruct_OTsender_37 OTsender;
# 58 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct __anonstruct_OTrecver_38 {
   void *recver ;
   void (*recv)(void * , char * , _Bool const * , int n , int len ) ;
   void (*release)(void * ) ;
};
# 58 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct __anonstruct_OTrecver_38 OTrecver;
# 64 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
union __anonunion____missing_field_name_39 {
   OTsender sender ;
   OTrecver recver ;
};
# 64 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct YaoProtocolDesc {
   yao_key_t R ;
   yao_key_t I ;
   uint64_t gcount ;
   unsigned int icount ;
   unsigned int ocount ;
   void (*nonFreeGate)(struct ProtocolDesc * , OblivBit * , char , OblivBit const * ,
                       OblivBit const * ) ;
   union __anonunion____missing_field_name_39 __annonCompField2 ;
};
# 64 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct YaoProtocolDesc YaoProtocolDesc;
# 73 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef struct ProtocolTransport ProtocolTransport;
# 77 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct ProtocolTransport {
   int maxParties ;
   int maxChannels ;
   int curChannel ;
   ProtocolTransport *(*subtransport)(ProtocolTransport * , int ) ;
   int (*send)(ProtocolTransport * , int , void const * , size_t ) ;
   int (*recv)(ProtocolTransport * , int , void * , size_t ) ;
   void (*cleanup)(ProtocolTransport * ) ;
};
# 85 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
struct OblivInputs {
   unsigned long long src ;
   struct OblivBit *dest ;
   size_t size ;
};
# 92 "/home/student/obliv-c/src/ext/oblivc/obliv_types.h"
typedef void (*protocol_run)(void * );
# 12 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct_yao_41 {
   yao_key_t w ;
   _Bool inverted ;
};
# 12 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
union __anonunion____missing_field_name_40 {
   _Bool knownValue ;
   struct __anonstruct_yao_41 yao ;
};
# 12 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct OblivBit {
   _Bool unknown ;
   union __anonunion____missing_field_name_40 __annonCompField3 ;
};
# 12 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct OblivBit OblivBit___0;
# 33 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__bool_42 {
   OblivBit___0 bits[1] ;
};
# 33 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__bool_42 __obliv_c__bool;
# 34 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__char_43 {
   OblivBit___0 bits[(unsigned long )8 * (unsigned long )sizeof(char )] ;
};
# 34 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__char_43 __obliv_c__char;
# 35 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__int_44 {
   OblivBit___0 bits[(unsigned long )8 * (unsigned long )sizeof(int )] ;
};
# 35 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__int_44 __obliv_c__int;
# 36 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__short_45 {
   OblivBit___0 bits[(unsigned long )8 * (unsigned long )sizeof(short )] ;
};
# 36 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__short_45 __obliv_c__short;
# 37 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__long_46 {
   OblivBit___0 bits[(unsigned long )8 * (unsigned long )sizeof(long )] ;
};
# 37 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__long_46 __obliv_c__long;
# 38 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
struct __anonstruct___obliv_c__lLong_47 {
   OblivBit___0 bits[(unsigned long )8 * (unsigned long )sizeof(long long )] ;
};
# 38 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
typedef struct __anonstruct___obliv_c__lLong_47 __obliv_c__lLong;
# 45 "/usr/include/stdio.h"
struct _IO_FILE;
# 45 "/usr/include/stdio.h"
struct _IO_FILE;
# 49 "/usr/include/stdio.h"
typedef struct _IO_FILE FILE;
# 65 "/usr/include/stdio.h"
typedef struct _IO_FILE __FILE;
# 83 "/usr/include/wchar.h"
union __anonunion___value_49 {
   unsigned int __wch ;
   char __wchb[4] ;
};
# 83 "/usr/include/wchar.h"
struct __anonstruct___mbstate_t_48 {
   int __count ;
   union __anonunion___value_49 __value ;
};
# 83 "/usr/include/wchar.h"
typedef struct __anonstruct___mbstate_t_48 __mbstate_t;
# 22 "/usr/include/_G_config.h"
struct __anonstruct__G_fpos_t_50 {
   __off_t __pos ;
   __mbstate_t __state ;
};
# 22 "/usr/include/_G_config.h"
typedef struct __anonstruct__G_fpos_t_50 _G_fpos_t;
# 27 "/usr/include/_G_config.h"
struct __anonstruct__G_fpos64_t_51 {
   __off64_t __pos ;
   __mbstate_t __state ;
};
# 27 "/usr/include/_G_config.h"
typedef struct __anonstruct__G_fpos64_t_51 _G_fpos64_t;
# 53 "/usr/include/_G_config.h"
typedef short _G_int16_t;
# 54 "/usr/include/_G_config.h"
typedef int _G_int32_t;
# 55 "/usr/include/_G_config.h"
typedef unsigned short _G_uint16_t;
# 56 "/usr/include/_G_config.h"
typedef unsigned int _G_uint32_t;
# 172 "/usr/include/libio.h"
struct _IO_jump_t;
# 172 "/usr/include/libio.h"
struct _IO_jump_t;
# 172 "/usr/include/libio.h"
struct _IO_FILE;
# 182 "/usr/include/libio.h"
typedef void _IO_lock_t;
# 188 "/usr/include/libio.h"
struct _IO_marker {
   struct _IO_marker *_next ;
   struct _IO_FILE *_sbuf ;
   int _pos ;
};
# 208 "/usr/include/libio.h"
enum __codecvt_result {
    __codecvt_ok = 0,
    __codecvt_partial = 1,
    __codecvt_error = 2,
    __codecvt_noconv = 3
} ;
# 273 "/usr/include/libio.h"
struct _IO_FILE {
   int _flags ;
   char *_IO_read_ptr ;
   char *_IO_read_end ;
   char *_IO_read_base ;
   char *_IO_write_base ;
   char *_IO_write_ptr ;
   char *_IO_write_end ;
   char *_IO_buf_base ;
   char *_IO_buf_end ;
   char *_IO_save_base ;
   char *_IO_backup_base ;
   char *_IO_save_end ;
   struct _IO_marker *_markers ;
   struct _IO_FILE *_chain ;
   int _fileno ;
   int _flags2 ;
   __off_t _old_offset ;
   unsigned short _cur_column ;
   signed char _vtable_offset ;
   char _shortbuf[1] ;
   _IO_lock_t *_lock ;
   __off64_t _offset ;
   void *__pad1 ;
   void *__pad2 ;
   void *__pad3 ;
   void *__pad4 ;
   size_t __pad5 ;
   int _mode ;
   char _unused2[(unsigned long )((unsigned long )((unsigned long )15 * (unsigned long )sizeof(int )) - (unsigned long )((unsigned long )4 * (unsigned long )sizeof(void *))) - (unsigned long )sizeof(size_t )] ;
};
# 343 "/usr/include/libio.h"
typedef struct _IO_FILE _IO_FILE;
# 346 "/usr/include/libio.h"
struct _IO_FILE_plus;
# 346 "/usr/include/libio.h"
struct _IO_FILE_plus;
# 366 "/usr/include/libio.h"
typedef __ssize_t __io_read_fn(void *__cookie , char *__buf , size_t __nbytes );
# 374 "/usr/include/libio.h"
typedef __ssize_t __io_write_fn(void *__cookie , char const *__buf , size_t __n );
# 383 "/usr/include/libio.h"
typedef int __io_seek_fn(void *__cookie , __off64_t *__pos , int __w );
# 386 "/usr/include/libio.h"
typedef int __io_close_fn(void *__cookie );
# 111 "/usr/include/stdio.h"
typedef _G_fpos_t fpos_t;
# 7 "./mutual.h"
typedef char *string;
# 8 "./mutual.h"
struct protocolIO {
   char mine[45][10] ;
   int size ;
   char common[45][10] ;
   int commonSize ;
};
# 8 "./mutual.h"
typedef struct protocolIO protocolIO;
# 7 "mutual.oc"
typedef __obliv_c__bool obool;
# 3 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__nonnull__(1), __leaf__)) memset)(void *__s ,
                                                                                               int __c ,
                                                                                               unsigned long __n ) ;
# 140 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__leaf__)) __ctype_get_mb_cur_max)(void) ;
# 145 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) double ( __attribute__((__nonnull__(1), __leaf__)) atof)(char const *__nptr ) __attribute__((__pure__)) ;
# 148 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) atoi)(char const *__nptr ) __attribute__((__pure__)) ;
# 151 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__nonnull__(1), __leaf__)) atol)(char const *__nptr ) __attribute__((__pure__)) ;
# 158 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long long ( __attribute__((__nonnull__(1), __leaf__)) atoll)(char const *__nptr ) __attribute__((__pure__)) ;
# 165 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) double ( __attribute__((__nonnull__(1), __leaf__)) strtod)(char const * __restrict __nptr ,
                                                                                                char ** __restrict __endptr ) ;
# 173 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) float ( __attribute__((__nonnull__(1), __leaf__)) strtof)(char const * __restrict __nptr ,
                                                                                               char ** __restrict __endptr ) ;
# 176 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long double ( __attribute__((__nonnull__(1),
__leaf__)) strtold)(char const * __restrict __nptr , char ** __restrict __endptr ) ;
# 184 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__nonnull__(1), __leaf__)) strtol)(char const * __restrict __nptr ,
                                                                                              char ** __restrict __endptr ,
                                                                                              int __base ) ;
# 188 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) unsigned long ( __attribute__((__nonnull__(1),
__leaf__)) strtoul)(char const * __restrict __nptr , char ** __restrict __endptr ,
                    int __base ) ;
# 196 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long long ( __attribute__((__nonnull__(1), __leaf__)) strtoq)(char const * __restrict __nptr ,
                                                                                                   char ** __restrict __endptr ,
                                                                                                   int __base ) ;
# 201 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) unsigned long long ( __attribute__((__nonnull__(1),
__leaf__)) strtouq)(char const * __restrict __nptr , char ** __restrict __endptr ,
                    int __base ) ;
# 210 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long long ( __attribute__((__nonnull__(1), __leaf__)) strtoll)(char const * __restrict __nptr ,
                                                                                                    char ** __restrict __endptr ,
                                                                                                    int __base ) ;
# 215 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) unsigned long long ( __attribute__((__nonnull__(1),
__leaf__)) strtoull)(char const * __restrict __nptr , char ** __restrict __endptr ,
                     int __base ) ;
# 311 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) l64a)(long __n ) ;
# 314 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__nonnull__(1), __leaf__)) a64l)(char const *__s ) __attribute__((__pure__)) ;
# 107 "/usr/include/x86_64-linux-gnu/sys/select.h"
extern int select(int __nfds , fd_set * __restrict __readfds , fd_set * __restrict __writefds ,
                  fd_set * __restrict __exceptfds , struct timeval * __restrict __timeout ) ;
# 119 "/usr/include/x86_64-linux-gnu/sys/select.h"
extern int pselect(int __nfds , fd_set * __restrict __readfds , fd_set * __restrict __writefds ,
                   fd_set * __restrict __exceptfds , struct timespec const * __restrict __timeout ,
                   __sigset_t const * __restrict __sigmask ) ;
# 33 "/usr/include/x86_64-linux-gnu/sys/sysmacros.h"
extern __attribute__((__nothrow__)) unsigned int ( __attribute__((__leaf__)) gnu_dev_major)(unsigned long long __dev ) __attribute__((__const__)) ;
# 36 "/usr/include/x86_64-linux-gnu/sys/sysmacros.h"
extern __attribute__((__nothrow__)) unsigned int ( __attribute__((__leaf__)) gnu_dev_minor)(unsigned long long __dev ) __attribute__((__const__)) ;
# 39 "/usr/include/x86_64-linux-gnu/sys/sysmacros.h"
extern __attribute__((__nothrow__)) unsigned long long ( __attribute__((__leaf__)) gnu_dev_makedev)(unsigned int __major ,
                                                                                                     unsigned int __minor ) __attribute__((__const__)) ;
# 327 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__leaf__)) random)(void) ;
# 330 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) srandom)(unsigned int __seed ) ;
# 336 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(2), __leaf__)) initstate)(unsigned int __seed ,
                                                                                                  char *__statebuf ,
                                                                                                  size_t __statelen ) ;
# 341 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) setstate)(char *__statebuf ) ;
# 360 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) random_r)(struct random_data * __restrict __buf ,
                                                                                                 int32_t * __restrict __result ) ;
# 363 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(2), __leaf__)) srandom_r)(unsigned int __seed ,
                                                                                                struct random_data *__buf ) ;
# 366 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(2,4), __leaf__)) initstate_r)(unsigned int __seed ,
                                                                                                    char * __restrict __statebuf ,
                                                                                                    size_t __statelen ,
                                                                                                    struct random_data * __restrict __buf ) ;
# 371 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) setstate_r)(char * __restrict __statebuf ,
                                                                                                   struct random_data * __restrict __buf ) ;
# 380 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) rand)(void) ;
# 382 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) srand)(unsigned int __seed ) ;
# 387 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) rand_r)(unsigned int *__seed ) ;
# 395 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) double ( __attribute__((__leaf__)) drand48)(void) ;
# 396 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) double ( __attribute__((__nonnull__(1), __leaf__)) erand48)(unsigned short *__xsubi ) ;
# 399 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__leaf__)) lrand48)(void) ;
# 400 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__nonnull__(1), __leaf__)) nrand48)(unsigned short *__xsubi ) ;
# 404 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__leaf__)) mrand48)(void) ;
# 405 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__nonnull__(1), __leaf__)) jrand48)(unsigned short *__xsubi ) ;
# 409 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) srand48)(long __seedval ) ;
# 410 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) unsigned short *( __attribute__((__nonnull__(1),
__leaf__)) seed48)(unsigned short *__seed16v ) ;
# 412 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__nonnull__(1), __leaf__)) lcong48)(unsigned short *__param ) ;
# 428 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) drand48_r)(struct drand48_data * __restrict __buffer ,
                                                                                                  double * __restrict __result ) ;
# 430 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) erand48_r)(unsigned short *__xsubi ,
                                                                                                  struct drand48_data * __restrict __buffer ,
                                                                                                  double * __restrict __result ) ;
# 435 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) lrand48_r)(struct drand48_data * __restrict __buffer ,
                                                                                                  long * __restrict __result ) ;
# 438 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) nrand48_r)(unsigned short *__xsubi ,
                                                                                                  struct drand48_data * __restrict __buffer ,
                                                                                                  long * __restrict __result ) ;
# 444 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) mrand48_r)(struct drand48_data * __restrict __buffer ,
                                                                                                  long * __restrict __result ) ;
# 447 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) jrand48_r)(unsigned short *__xsubi ,
                                                                                                  struct drand48_data * __restrict __buffer ,
                                                                                                  long * __restrict __result ) ;
# 453 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(2), __leaf__)) srand48_r)(long __seedval ,
                                                                                                struct drand48_data *__buffer ) ;
# 456 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) seed48_r)(unsigned short *__seed16v ,
                                                                                                 struct drand48_data *__buffer ) ;
# 459 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) lcong48_r)(unsigned short *__param ,
                                                                                                  struct drand48_data *__buffer ) ;
# 471 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__leaf__)) malloc)(size_t __size ) __attribute__((__malloc__)) ;
# 473 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__leaf__)) calloc)(size_t __nmemb ,
                                                                               size_t __size ) __attribute__((__malloc__)) ;
# 485 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__warn_unused_result__,
__leaf__)) realloc)(void *__ptr , size_t __size ) ;
# 488 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) free)(void *__ptr ) ;
# 493 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) cfree)(void *__ptr ) ;
# 33 "/usr/include/alloca.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__leaf__)) alloca)(size_t __size ) ;
# 503 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__leaf__)) valloc)(size_t __size ) __attribute__((__malloc__)) ;
# 508 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) posix_memalign)(void **__memptr ,
                                                                                                     size_t __alignment ,
                                                                                                     size_t __size ) ;
# 514 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__, __noreturn__)) void ( __attribute__((__leaf__)) abort)(void) ;
# 518 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) atexit)(void (*__func)(void) ) ;
# 536 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) on_exit)(void (*__func)(int __status ,
                                                                                                             void *__arg ) ,
                                                                                              void *__arg ) ;
# 544 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__, __noreturn__)) void ( __attribute__((__leaf__)) exit)(int __status ) ;
# 560 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__, __noreturn__)) void ( __attribute__((__leaf__)) _Exit)(int __status ) ;
# 567 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) getenv)(char const *__name ) ;
# 572 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) __secure_getenv)(char const *__name ) ;
# 579 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) putenv)(char *__string ) ;
# 585 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(2), __leaf__)) setenv)(char const *__name ,
                                                                                             char const *__value ,
                                                                                             int __replace ) ;
# 589 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) unsetenv)(char const *__name ) ;
# 596 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) clearenv)(void) ;
# 606 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) mktemp)(char *__template ) ;
# 620 "/usr/include/stdlib.h"
extern int ( __attribute__((__nonnull__(1))) mkstemp)(char *__template ) ;
# 642 "/usr/include/stdlib.h"
extern int ( __attribute__((__nonnull__(1))) mkstemps)(char *__template , int __suffixlen ) ;
# 663 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) mkdtemp)(char *__template ) ;
# 717 "/usr/include/stdlib.h"
extern int system(char const *__command ) ;
# 734 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) realpath)(char const * __restrict __name ,
                                                                                 char * __restrict __resolved ) ;
# 755 "/usr/include/stdlib.h"
extern void *( __attribute__((__nonnull__(1,2,5))) bsearch)(void const *__key ,
                                                            void const *__base ,
                                                            size_t __nmemb , size_t __size ,
                                                            int (*__compar)(void const * ,
                                                                            void const * ) ) ;
# 761 "/usr/include/stdlib.h"
extern void ( __attribute__((__nonnull__(1,4))) qsort)(void *__base , size_t __nmemb ,
                                                       size_t __size , int (*__compar)(void const * ,
                                                                                       void const * ) ) ;
# 771 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) abs)(int __x ) __attribute__((__const__)) ;
# 772 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long ( __attribute__((__leaf__)) labs)(long __x ) __attribute__((__const__)) ;
# 776 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) long long ( __attribute__((__leaf__)) llabs)(long long __x ) __attribute__((__const__)) ;
# 785 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) div_t ( __attribute__((__leaf__)) div)(int __numer ,
                                                                            int __denom ) __attribute__((__const__)) ;
# 787 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) ldiv_t ( __attribute__((__leaf__)) ldiv)(long __numer ,
                                                                              long __denom ) __attribute__((__const__)) ;
# 793 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) lldiv_t ( __attribute__((__leaf__)) lldiv)(long long __numer ,
                                                                                long long __denom ) __attribute__((__const__)) ;
# 808 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3,4), __leaf__)) ecvt)(double __value ,
                                                                                               int __ndigit ,
                                                                                               int * __restrict __decpt ,
                                                                                               int * __restrict __sign ) ;
# 814 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3,4), __leaf__)) fcvt)(double __value ,
                                                                                               int __ndigit ,
                                                                                               int * __restrict __decpt ,
                                                                                               int * __restrict __sign ) ;
# 820 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3), __leaf__)) gcvt)(double __value ,
                                                                                             int __ndigit ,
                                                                                             char *__buf ) ;
# 826 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3,4), __leaf__)) qecvt)(long double __value ,
                                                                                                int __ndigit ,
                                                                                                int * __restrict __decpt ,
                                                                                                int * __restrict __sign ) ;
# 829 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3,4), __leaf__)) qfcvt)(long double __value ,
                                                                                                int __ndigit ,
                                                                                                int * __restrict __decpt ,
                                                                                                int * __restrict __sign ) ;
# 832 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(3), __leaf__)) qgcvt)(long double __value ,
                                                                                              int __ndigit ,
                                                                                              char *__buf ) ;
# 838 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(3,4,5), __leaf__)) ecvt_r)(double __value ,
                                                                                                 int __ndigit ,
                                                                                                 int * __restrict __decpt ,
                                                                                                 int * __restrict __sign ,
                                                                                                 char * __restrict __buf ,
                                                                                                 size_t __len ) ;
# 841 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(3,4,5), __leaf__)) fcvt_r)(double __value ,
                                                                                                 int __ndigit ,
                                                                                                 int * __restrict __decpt ,
                                                                                                 int * __restrict __sign ,
                                                                                                 char * __restrict __buf ,
                                                                                                 size_t __len ) ;
# 845 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(3,4,5), __leaf__)) qecvt_r)(long double __value ,
                                                                                                  int __ndigit ,
                                                                                                  int * __restrict __decpt ,
                                                                                                  int * __restrict __sign ,
                                                                                                  char * __restrict __buf ,
                                                                                                  size_t __len ) ;
# 849 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(3,4,5), __leaf__)) qfcvt_r)(long double __value ,
                                                                                                  int __ndigit ,
                                                                                                  int * __restrict __decpt ,
                                                                                                  int * __restrict __sign ,
                                                                                                  char * __restrict __buf ,
                                                                                                  size_t __len ) ;
# 860 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) mblen)(char const *__s ,
                                                                            size_t __n ) ;
# 863 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) mbtowc)(wchar_t * __restrict __pwc ,
                                                                             char const * __restrict __s ,
                                                                             size_t __n ) ;
# 867 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) wctomb)(char *__s ,
                                                                             wchar_t __wchar ) ;
# 871 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__leaf__)) mbstowcs)(wchar_t * __restrict __pwcs ,
                                                                                  char const * __restrict __s ,
                                                                                  size_t __n ) ;
# 874 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__leaf__)) wcstombs)(char * __restrict __s ,
                                                                                  wchar_t const * __restrict __pwcs ,
                                                                                  size_t __n ) ;
# 885 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) rpmatch)(char const *__response ) ;
# 896 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2,3), __leaf__)) getsubopt)(char ** __restrict __optionp ,
                                                                                                    char * const * __restrict __tokens ,
                                                                                                    char ** __restrict __valuep ) ;
# 948 "/usr/include/stdlib.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) getloadavg)(double *__loadavg ,
                                                                                                 int __nelem ) ;
# 44 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__nonnull__(1,2), __leaf__)) memcpy)(void * __restrict __dest ,
                                                                                                 void const * __restrict __src ,
                                                                                                 size_t __n ) ;
# 49 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__nonnull__(1,2), __leaf__)) memmove)(void *__dest ,
                                                                                                  void const *__src ,
                                                                                                  size_t __n ) ;
# 57 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__nonnull__(1,2), __leaf__)) memccpy)(void * __restrict __dest ,
                                                                                                  void const * __restrict __src ,
                                                                                                  int __c ,
                                                                                                  size_t __n ) ;
# 68 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) memcmp)(void const *__s1 ,
                                                                                               void const *__s2 ,
                                                                                               size_t __n ) __attribute__((__pure__)) ;
# 95 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void *( __attribute__((__nonnull__(1), __leaf__)) memchr)(void const *__s ,
                                                                                               int __c ,
                                                                                               size_t __n ) __attribute__((__pure__)) ;
# 128 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strcpy)(char * __restrict __dest ,
                                                                                                 char const * __restrict __src ) ;
# 131 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strncpy)(char * __restrict __dest ,
                                                                                                  char const * __restrict __src ,
                                                                                                  size_t __n ) ;
# 136 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strcat)(char * __restrict __dest ,
                                                                                                 char const * __restrict __src ) ;
# 139 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strncat)(char * __restrict __dest ,
                                                                                                  char const * __restrict __src ,
                                                                                                  size_t __n ) ;
# 143 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) strcmp)(char const *__s1 ,
                                                                                               char const *__s2 ) __attribute__((__pure__)) ;
# 146 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) strncmp)(char const *__s1 ,
                                                                                                char const *__s2 ,
                                                                                                size_t __n ) __attribute__((__pure__)) ;
# 150 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) strcoll)(char const *__s1 ,
                                                                                                char const *__s2 ) __attribute__((__pure__)) ;
# 153 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(2), __leaf__)) strxfrm)(char * __restrict __dest ,
                                                                                                 char const * __restrict __src ,
                                                                                                 size_t __n ) ;
# 165 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2,3), __leaf__)) strcoll_l)(char const *__s1 ,
                                                                                                    char const *__s2 ,
                                                                                                    __locale_t __l ) __attribute__((__pure__)) ;
# 168 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(2,4), __leaf__)) strxfrm_l)(char *__dest ,
                                                                                                     char const *__src ,
                                                                                                     size_t __n ,
                                                                                                     __locale_t __l ) ;
# 175 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) strdup)(char const *__s ) __attribute__((__malloc__)) ;
# 183 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) strndup)(char const *__string ,
                                                                                                size_t __n ) __attribute__((__malloc__)) ;
# 235 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) strchr)(char const *__s ,
                                                                                               int __c ) __attribute__((__pure__)) ;
# 262 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) strrchr)(char const *__s ,
                                                                                                int __c ) __attribute__((__pure__)) ;
# 284 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(1,2), __leaf__)) strcspn)(char const *__s ,
                                                                                                   char const *__reject ) __attribute__((__pure__)) ;
# 288 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(1,2), __leaf__)) strspn)(char const *__s ,
                                                                                                  char const *__accept ) __attribute__((__pure__)) ;
# 314 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strpbrk)(char const *__s ,
                                                                                                  char const *__accept ) __attribute__((__pure__)) ;
# 342 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strstr)(char const *__haystack ,
                                                                                                 char const *__needle ) __attribute__((__pure__)) ;
# 348 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(2), __leaf__)) strtok)(char * __restrict __s ,
                                                                                               char const * __restrict __delim ) ;
# 354 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(2,3), __leaf__)) __strtok_r)(char * __restrict __s ,
                                                                                                     char const * __restrict __delim ,
                                                                                                     char ** __restrict __save_ptr ) ;
# 359 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(2,3), __leaf__)) strtok_r)(char * __restrict __s ,
                                                                                                   char const * __restrict __delim ,
                                                                                                   char ** __restrict __save_ptr ) ;
# 399 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(1), __leaf__)) strlen)(char const *__s ) __attribute__((__pure__)) ;
# 406 "/usr/include/string.h"
extern __attribute__((__nothrow__)) size_t ( __attribute__((__nonnull__(1), __leaf__)) strnlen)(char const *__string ,
                                                                                                 size_t __maxlen ) __attribute__((__pure__)) ;
# 413 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) strerror)(int __errnum ) ;
# 427 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(2), __leaf__)) strerror_r)(int __errnum ,
                                                                                                 char *__buf ,
                                                                                                 size_t __buflen ) __asm__("__xpg_strerror_r") ;
# 445 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) strerror_l)(int __errnum ,
                                                                                   __locale_t __l ) ;
# 451 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__nonnull__(1), __leaf__)) __bzero)(void *__s ,
                                                                                               size_t __n ) ;
# 455 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__nonnull__(1,2), __leaf__)) bcopy)(void const *__src ,
                                                                                               void *__dest ,
                                                                                               size_t __n ) ;
# 459 "/usr/include/string.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__nonnull__(1), __leaf__)) bzero)(void *__s ,
                                                                                             size_t __n ) ;
# 462 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) bcmp)(void const *__s1 ,
                                                                                             void const *__s2 ,
                                                                                             size_t __n ) __attribute__((__pure__)) ;
# 489 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) index)(char const *__s ,
                                                                                              int __c ) __attribute__((__pure__)) ;
# 517 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1), __leaf__)) rindex)(char const *__s ,
                                                                                               int __c ) __attribute__((__pure__)) ;
# 523 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) ffs)(int __i ) __attribute__((__const__)) ;
# 536 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) strcasecmp)(char const *__s1 ,
                                                                                                   char const *__s2 ) __attribute__((__pure__)) ;
# 540 "/usr/include/string.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1,2), __leaf__)) strncasecmp)(char const *__s1 ,
                                                                                                    char const *__s2 ,
                                                                                                    size_t __n ) __attribute__((__pure__)) ;
# 559 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) strsep)(char ** __restrict __stringp ,
                                                                                                 char const * __restrict __delim ) ;
# 566 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) strsignal)(int __sig ) ;
# 569 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) __stpcpy)(char * __restrict __dest ,
                                                                                                   char const * __restrict __src ) ;
# 571 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) stpcpy)(char * __restrict __dest ,
                                                                                                 char const * __restrict __src ) ;
# 576 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) __stpncpy)(char * __restrict __dest ,
                                                                                                    char const * __restrict __src ,
                                                                                                    size_t __n ) ;
# 579 "/usr/include/string.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__nonnull__(1,2), __leaf__)) stpncpy)(char * __restrict __dest ,
                                                                                                  char const * __restrict __src ,
                                                                                                  size_t __n ) ;
# 551 "/usr/include/gpg-error.h"
extern gpg_error_t gpg_err_init(void) __attribute__((__constructor__)) ;
# 562 "/usr/include/gpg-error.h"
extern void gpg_err_deinit(int mode ) ;
# 569 "/usr/include/gpg-error.h"
__inline static gpg_error_t gpg_err_make(gpg_err_source_t source , gpg_err_code_t code )
{
  unsigned int tmp ;
  unsigned int __cil_tmp4 ;
  unsigned int __cil_tmp5 ;
  unsigned int __cil_tmp6 ;
  unsigned int __cil_tmp7 ;
  unsigned int __cil_tmp8 ;
  unsigned int __cil_tmp9 ;
  int __cil_tmp10 ;
  unsigned int __cil_tmp11 ;
  unsigned int __cil_tmp12 ;
  unsigned int __cil_tmp13 ;
  unsigned int __cil_tmp14 ;
  unsigned int __cil_tmp15 ;
  unsigned int __cil_tmp16 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(unsigned int ));
  {
# 572 "/usr/include/gpg-error.h"
  __cil_tmp4 = (unsigned int )0;
# 572 "/usr/include/gpg-error.h"
  __cil_tmp5 = (unsigned int )code;
# 572 "/usr/include/gpg-error.h"
  if (__cil_tmp5 == __cil_tmp4) {
# 572 "/usr/include/gpg-error.h"
    tmp = (unsigned int )0;
  } else {
# 572 "/usr/include/gpg-error.h"
    __cil_tmp6 = (unsigned int )65535;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp7 = (unsigned int )code;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp8 = __cil_tmp7 & __cil_tmp6;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp9 = (unsigned int )__cil_tmp8;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp10 = (int )24;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp11 = (unsigned int )127;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp12 = (unsigned int )source;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp13 = __cil_tmp12 & __cil_tmp11;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp14 = (unsigned int )__cil_tmp13;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp15 = __cil_tmp14 << __cil_tmp10;
# 572 "/usr/include/gpg-error.h"
    __cil_tmp16 = (unsigned int )__cil_tmp15;
# 572 "/usr/include/gpg-error.h"
    tmp = __cil_tmp16 | __cil_tmp9;
  }
  }
# 572 "/usr/include/gpg-error.h"
  return (tmp);
}
}
# 584 "/usr/include/gpg-error.h"
__inline static gpg_error_t gpg_error(gpg_err_code_t code )
{
  gpg_error_t tmp ;
  gpg_err_source_t __cil_tmp3 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_error_t ));
# 587 "/usr/include/gpg-error.h"
  __cil_tmp3 = (gpg_err_source_t )0;
# 587 "/usr/include/gpg-error.h"
  tmp = gpg_err_make(__cil_tmp3, code);
# 587 "/usr/include/gpg-error.h"
  return (tmp);
}
}
# 592 "/usr/include/gpg-error.h"
__inline static gpg_err_code_t gpg_err_code(gpg_error_t err )
{
  unsigned int __cil_tmp2 ;
  unsigned int __cil_tmp3 ;
  unsigned int __cil_tmp4 ;

  {
  {
# 595 "/usr/include/gpg-error.h"
  __cil_tmp2 = (unsigned int )65535;
# 595 "/usr/include/gpg-error.h"
  __cil_tmp3 = (unsigned int )err;
# 595 "/usr/include/gpg-error.h"
  __cil_tmp4 = __cil_tmp3 & __cil_tmp2;
# 595 "/usr/include/gpg-error.h"
  return ((gpg_err_code_t )__cil_tmp4);
  }
}
}
# 600 "/usr/include/gpg-error.h"
__inline static gpg_err_source_t gpg_err_source(gpg_error_t err )
{
  unsigned int __cil_tmp2 ;
  int __cil_tmp3 ;
  gpg_error_t __cil_tmp4 ;
  gpg_error_t __cil_tmp5 ;
  unsigned int __cil_tmp6 ;
  unsigned int __cil_tmp7 ;

  {
  {
# 603 "/usr/include/gpg-error.h"
  __cil_tmp2 = (unsigned int )127;
# 603 "/usr/include/gpg-error.h"
  __cil_tmp3 = (int )24;
# 603 "/usr/include/gpg-error.h"
  __cil_tmp4 = (gpg_error_t )err;
# 603 "/usr/include/gpg-error.h"
  __cil_tmp5 = __cil_tmp4 >> __cil_tmp3;
# 603 "/usr/include/gpg-error.h"
  __cil_tmp6 = (unsigned int )__cil_tmp5;
# 603 "/usr/include/gpg-error.h"
  __cil_tmp7 = __cil_tmp6 & __cil_tmp2;
# 603 "/usr/include/gpg-error.h"
  return ((gpg_err_source_t )__cil_tmp7);
  }
}
}
# 612 "/usr/include/gpg-error.h"
extern char const *gpg_strerror(gpg_error_t err ) ;
# 621 "/usr/include/gpg-error.h"
extern int gpg_strerror_r(gpg_error_t err , char *buf , size_t buflen ) ;
# 625 "/usr/include/gpg-error.h"
extern char const *gpg_strsource(gpg_error_t err ) ;
# 633 "/usr/include/gpg-error.h"
extern gpg_err_code_t gpg_err_code_from_errno(int err ) ;
# 638 "/usr/include/gpg-error.h"
extern int gpg_err_code_to_errno(gpg_err_code_t code ) ;
# 644 "/usr/include/gpg-error.h"
extern gpg_err_code_t gpg_err_code_from_syserror(void) ;
# 649 "/usr/include/gpg-error.h"
extern void gpg_err_set_errno(int err ) ;
# 654 "/usr/include/gpg-error.h"
__inline static gpg_error_t gpg_err_make_from_errno(gpg_err_source_t source , int err )
{
  gpg_err_code_t tmp ;
  gpg_error_t tmp___0 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_err_code_t ));
# 49 "mutual.oc"
  memset((void *)(& tmp___0), 0, sizeof(gpg_error_t ));
# 657 "/usr/include/gpg-error.h"
  tmp = gpg_err_code_from_errno(err);
# 657 "/usr/include/gpg-error.h"
  tmp___0 = gpg_err_make(source, tmp);
# 657 "/usr/include/gpg-error.h"
  return (tmp___0);
}
}
# 661 "/usr/include/gpg-error.h"
__inline static gpg_error_t gpg_error_from_errno(int err )
{
  gpg_err_code_t tmp ;
  gpg_error_t tmp___0 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_err_code_t ));
# 49 "mutual.oc"
  memset((void *)(& tmp___0), 0, sizeof(gpg_error_t ));
# 664 "/usr/include/gpg-error.h"
  tmp = gpg_err_code_from_errno(err);
# 664 "/usr/include/gpg-error.h"
  tmp___0 = gpg_error(tmp);
# 664 "/usr/include/gpg-error.h"
  return (tmp___0);
}
}
# 667 "/usr/include/gpg-error.h"
__inline static gpg_error_t gpg_error_from_syserror(void)
{
  gpg_err_code_t tmp ;
  gpg_error_t tmp___0 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_err_code_t ));
# 49 "mutual.oc"
  memset((void *)(& tmp___0), 0, sizeof(gpg_error_t ));
# 670 "/usr/include/gpg-error.h"
  tmp = gpg_err_code_from_syserror();
# 670 "/usr/include/gpg-error.h"
  tmp___0 = gpg_error(tmp);
# 670 "/usr/include/gpg-error.h"
  return (tmp___0);
}
}
# 58 "/usr/include/x86_64-linux-gnu/bits/uio.h"
extern __attribute__((__nothrow__)) ssize_t ( __attribute__((__leaf__)) process_vm_readv)(pid_t __pid ,
                                                                                           struct iovec const *__lvec ,
                                                                                           unsigned long __liovcnt ,
                                                                                           struct iovec const *__rvec ,
                                                                                           unsigned long __riovcnt ,
                                                                                           unsigned long __flags ) ;
# 66 "/usr/include/x86_64-linux-gnu/bits/uio.h"
extern __attribute__((__nothrow__)) ssize_t ( __attribute__((__leaf__)) process_vm_writev)(pid_t __pid ,
                                                                                            struct iovec const *__lvec ,
                                                                                            unsigned long __liovcnt ,
                                                                                            struct iovec const *__rvec ,
                                                                                            unsigned long __riovcnt ,
                                                                                            unsigned long __flags ) ;
# 40 "/usr/include/x86_64-linux-gnu/sys/uio.h"
extern ssize_t readv(int __fd , struct iovec const *__iovec , int __count ) ;
# 51 "/usr/include/x86_64-linux-gnu/sys/uio.h"
extern ssize_t writev(int __fd , struct iovec const *__iovec , int __count ) ;
# 66 "/usr/include/x86_64-linux-gnu/sys/uio.h"
extern ssize_t preadv(int __fd , struct iovec const *__iovec , int __count , __off_t __offset ) ;
# 78 "/usr/include/x86_64-linux-gnu/sys/uio.h"
extern ssize_t pwritev(int __fd , struct iovec const *__iovec , int __count , __off_t __offset ) ;
# 310 "/usr/include/x86_64-linux-gnu/bits/socket.h"
extern __attribute__((__nothrow__)) struct cmsghdr *( __attribute__((__leaf__)) __cmsg_nxthdr)(struct msghdr *__mhdr ,
                                                                                                struct cmsghdr *__cmsg ) ;
# 431 "/usr/include/x86_64-linux-gnu/bits/socket.h"
extern int recvmmsg(int __fd , struct mmsghdr *__vmessages , unsigned int __vlen ,
                    int __flags , struct timespec const *__tmo ) ;
# 439 "/usr/include/x86_64-linux-gnu/bits/socket.h"
extern int sendmmsg(int __fd , struct mmsghdr *__vmessages , unsigned int __vlen ,
                    int __flags ) ;
# 105 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) socket)(int __domain ,
                                                                             int __type ,
                                                                             int __protocol ) ;
# 111 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) socketpair)(int __domain ,
                                                                                 int __type ,
                                                                                 int __protocol ,
                                                                                 int *__fds ) ;
# 115 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) bind)(int __fd ,
                                                                           struct sockaddr const *__addr ,
                                                                           socklen_t __len ) ;
# 119 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) getsockname)(int __fd ,
                                                                                  struct sockaddr * __restrict __addr ,
                                                                                  socklen_t * __restrict __len ) ;
# 129 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern int connect(int __fd , struct sockaddr const *__addr , socklen_t __len ) ;
# 133 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) getpeername)(int __fd ,
                                                                                  struct sockaddr * __restrict __addr ,
                                                                                  socklen_t * __restrict __len ) ;
# 141 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t send(int __fd , void const *__buf , size_t __n , int __flags ) ;
# 148 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t recv(int __fd , void *__buf , size_t __n , int __flags ) ;
# 155 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t sendto(int __fd , void const *__buf , size_t __n , int __flags ,
                      struct sockaddr const *__addr , socklen_t __addr_len ) ;
# 166 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t recvfrom(int __fd , void * __restrict __buf , size_t __n , int __flags ,
                        struct sockaddr * __restrict __addr , socklen_t * __restrict __addr_len ) ;
# 176 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t sendmsg(int __fd , struct msghdr const *__message , int __flags ) ;
# 184 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern ssize_t recvmsg(int __fd , struct msghdr *__message , int __flags ) ;
# 190 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) getsockopt)(int __fd ,
                                                                                 int __level ,
                                                                                 int __optname ,
                                                                                 void * __restrict __optval ,
                                                                                 socklen_t * __restrict __optlen ) ;
# 197 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) setsockopt)(int __fd ,
                                                                                 int __level ,
                                                                                 int __optname ,
                                                                                 void const *__optval ,
                                                                                 socklen_t __optlen ) ;
# 204 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) listen)(int __fd ,
                                                                             int __n ) ;
# 214 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern int accept(int __fd , struct sockaddr * __restrict __addr , socklen_t * __restrict __addr_len ) ;
# 232 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) shutdown)(int __fd ,
                                                                               int __how ) ;
# 237 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) sockatmark)(int __fd ) ;
# 245 "/usr/include/x86_64-linux-gnu/sys/socket.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) isfdtype)(int __fd ,
                                                                               int __fdtype ) ;
# 73 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) gettimeofday)(struct timeval * __restrict __tv ,
                                                                                                   __timezone_ptr_t __tz ) ;
# 79 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) settimeofday)(struct timeval const *__tv ,
                                                                                                   struct timezone const *__tz ) ;
# 87 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) adjtime)(struct timeval const *__delta ,
                                                                              struct timeval *__olddelta ) ;
# 127 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) getitimer)(__itimer_which_t __which ,
                                                                                struct itimerval *__value ) ;
# 133 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) setitimer)(__itimer_which_t __which ,
                                                                                struct itimerval const * __restrict __new ,
                                                                                struct itimerval * __restrict __old ) ;
# 140 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) utimes)(char const *__file ,
                                                                                             struct timeval const *__tvp ) ;
# 145 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__nonnull__(1), __leaf__)) lutimes)(char const *__file ,
                                                                                              struct timeval const *__tvp ) ;
# 149 "/usr/include/x86_64-linux-gnu/sys/time.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) futimes)(int __fd ,
                                                                              struct timeval const *__tvp ) ;
# 121 "/usr/include/gcrypt.h"
__inline static gcry_error_t gcry_err_make(gcry_err_source_t source , gcry_err_code_t code )
{
  gpg_error_t tmp ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_error_t ));
# 124 "/usr/include/gcrypt.h"
  tmp = gpg_err_make(source, code);
# 124 "/usr/include/gcrypt.h"
  return (tmp);
}
}
# 133 "/usr/include/gcrypt.h"
__inline static gcry_error_t gcry_error(gcry_err_code_t code )
{
  gcry_error_t tmp ;
  gcry_err_source_t __cil_tmp3 ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gcry_error_t ));
# 136 "/usr/include/gcrypt.h"
  __cil_tmp3 = (gcry_err_source_t )32;
# 136 "/usr/include/gcrypt.h"
  tmp = gcry_err_make(__cil_tmp3, code);
# 136 "/usr/include/gcrypt.h"
  return (tmp);
}
}
# 139 "/usr/include/gcrypt.h"
__inline static gcry_err_code_t gcry_err_code(gcry_error_t err )
{
  gpg_err_code_t tmp ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_err_code_t ));
# 142 "/usr/include/gcrypt.h"
  tmp = gpg_err_code(err);
# 142 "/usr/include/gcrypt.h"
  return (tmp);
}
}
# 146 "/usr/include/gcrypt.h"
__inline static gcry_err_source_t gcry_err_source(gcry_error_t err )
{
  gpg_err_source_t tmp ;

  {
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(gpg_err_source_t ));
# 149 "/usr/include/gcrypt.h"
  tmp = gpg_err_source(err);
# 149 "/usr/include/gcrypt.h"
  return (tmp);
}
}
# 154 "/usr/include/gcrypt.h"
extern char const *gcry_strerror(gcry_error_t err ) ;
# 158 "/usr/include/gcrypt.h"
extern char const *gcry_strsource(gcry_error_t err ) ;
# 163 "/usr/include/gcrypt.h"
extern gcry_err_code_t gcry_err_code_from_errno(int err ) ;
# 167 "/usr/include/gcrypt.h"
extern int gcry_err_code_to_errno(gcry_err_code_t code ) ;
# 171 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_err_make_from_errno(gcry_err_source_t source , int err ) ;
# 174 "/usr/include/gcrypt.h"
extern gcry_err_code_t gcry_error_from_errno(int err ) ;
# 354 "/usr/include/gcrypt.h"
extern char const *gcry_check_version(char const *req_version ) ;
# 422 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_control(enum gcry_ctl_cmds CMD , ...) ;
# 449 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_sexp_new(gcry_sexp_t *retsexp , void const *buffer , size_t length ,
                                  int autodetect ) ;
# 455 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_sexp_create(gcry_sexp_t *retsexp , void *buffer , size_t length ,
                                     int autodetect , void (*freefnc)(void * ) ) ;
# 461 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_sexp_sscan(gcry_sexp_t *retsexp , size_t *erroff , char const *buffer ,
                                    size_t length ) ;
# 466 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_sexp_build(gcry_sexp_t *retsexp , size_t *erroff , char const *format
                                    , ...) ;
# 471 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_sexp_build_array(gcry_sexp_t *retsexp , size_t *erroff ,
                                          char const *format , void **arg_list ) ;
# 475 "/usr/include/gcrypt.h"
extern void gcry_sexp_release(gcry_sexp_t sexp ) ;
# 479 "/usr/include/gcrypt.h"
extern size_t gcry_sexp_canon_len(unsigned char const *buffer , size_t length ,
                                  size_t *erroff , gcry_error_t *errcode ) ;
# 484 "/usr/include/gcrypt.h"
extern size_t gcry_sexp_sprint(gcry_sexp_t sexp , int mode , void *buffer , size_t maxlength ) ;
# 489 "/usr/include/gcrypt.h"
extern void gcry_sexp_dump(gcry_sexp_t const a ) ;
# 491 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_cons(gcry_sexp_t const a , gcry_sexp_t const b ) ;
# 492 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_alist(gcry_sexp_t const *array ) ;
# 493 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_vlist(gcry_sexp_t const a , ...) ;
# 494 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_append(gcry_sexp_t const a , gcry_sexp_t const n ) ;
# 495 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_prepend(gcry_sexp_t const a , gcry_sexp_t const n ) ;
# 502 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_find_token(gcry_sexp_t list , char const *tok , size_t toklen ) ;
# 506 "/usr/include/gcrypt.h"
extern int gcry_sexp_length(gcry_sexp_t const list ) ;
# 511 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_nth(gcry_sexp_t const list , int number ) ;
# 516 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_car(gcry_sexp_t const list ) ;
# 523 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_cdr(gcry_sexp_t const list ) ;
# 525 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_sexp_cadr(gcry_sexp_t const list ) ;
# 534 "/usr/include/gcrypt.h"
extern char const *gcry_sexp_nth_data(gcry_sexp_t const list , int number , size_t *datalen ) ;
# 542 "/usr/include/gcrypt.h"
extern char *gcry_sexp_nth_string(gcry_sexp_t list , int number ) ;
# 550 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_sexp_nth_mpi(gcry_sexp_t list , int number , int mpifmt ) ;
# 583 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_new(unsigned int nbits ) ;
# 586 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_snew(unsigned int nbits ) ;
# 589 "/usr/include/gcrypt.h"
extern void gcry_mpi_release(gcry_mpi_t a ) ;
# 592 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_copy(gcry_mpi_t const a ) ;
# 595 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_set(gcry_mpi_t w , gcry_mpi_t const u ) ;
# 598 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_set_ui(gcry_mpi_t w , unsigned long u ) ;
# 601 "/usr/include/gcrypt.h"
extern void gcry_mpi_swap(gcry_mpi_t a , gcry_mpi_t b ) ;
# 605 "/usr/include/gcrypt.h"
extern int gcry_mpi_cmp(gcry_mpi_t const u , gcry_mpi_t const v ) ;
# 610 "/usr/include/gcrypt.h"
extern int gcry_mpi_cmp_ui(gcry_mpi_t const u , unsigned long v ) ;
# 616 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_mpi_scan(gcry_mpi_t *ret_mpi , enum gcry_mpi_format format ,
                                  void const *buffer , size_t buflen , size_t *nscanned ) ;
# 625 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_mpi_print(enum gcry_mpi_format format , unsigned char *buffer ,
                                   size_t buflen , size_t *nwritten , gcry_mpi_t const a ) ;
# 634 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_mpi_aprint(enum gcry_mpi_format format , unsigned char **buffer ,
                                    size_t *nwritten , gcry_mpi_t const a ) ;
# 642 "/usr/include/gcrypt.h"
extern void gcry_mpi_dump(gcry_mpi_t const a ) ;
# 646 "/usr/include/gcrypt.h"
extern void gcry_mpi_add(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v ) ;
# 649 "/usr/include/gcrypt.h"
extern void gcry_mpi_add_ui(gcry_mpi_t w , gcry_mpi_t u , unsigned long v ) ;
# 652 "/usr/include/gcrypt.h"
extern void gcry_mpi_addm(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v , gcry_mpi_t m ) ;
# 655 "/usr/include/gcrypt.h"
extern void gcry_mpi_sub(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v ) ;
# 658 "/usr/include/gcrypt.h"
extern void gcry_mpi_sub_ui(gcry_mpi_t w , gcry_mpi_t u , unsigned long v ) ;
# 661 "/usr/include/gcrypt.h"
extern void gcry_mpi_subm(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v , gcry_mpi_t m ) ;
# 664 "/usr/include/gcrypt.h"
extern void gcry_mpi_mul(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v ) ;
# 667 "/usr/include/gcrypt.h"
extern void gcry_mpi_mul_ui(gcry_mpi_t w , gcry_mpi_t u , unsigned long v ) ;
# 670 "/usr/include/gcrypt.h"
extern void gcry_mpi_mulm(gcry_mpi_t w , gcry_mpi_t u , gcry_mpi_t v , gcry_mpi_t m ) ;
# 673 "/usr/include/gcrypt.h"
extern void gcry_mpi_mul_2exp(gcry_mpi_t w , gcry_mpi_t u , unsigned long cnt ) ;
# 677 "/usr/include/gcrypt.h"
extern void gcry_mpi_div(gcry_mpi_t q , gcry_mpi_t r , gcry_mpi_t dividend , gcry_mpi_t divisor ,
                         int round ) ;
# 681 "/usr/include/gcrypt.h"
extern void gcry_mpi_mod(gcry_mpi_t r , gcry_mpi_t dividend , gcry_mpi_t divisor ) ;
# 684 "/usr/include/gcrypt.h"
extern void gcry_mpi_powm(gcry_mpi_t w , gcry_mpi_t const b , gcry_mpi_t const e ,
                          gcry_mpi_t const m ) ;
# 690 "/usr/include/gcrypt.h"
extern int gcry_mpi_gcd(gcry_mpi_t g , gcry_mpi_t a , gcry_mpi_t b ) ;
# 694 "/usr/include/gcrypt.h"
extern int gcry_mpi_invm(gcry_mpi_t x , gcry_mpi_t a , gcry_mpi_t m ) ;
# 698 "/usr/include/gcrypt.h"
extern unsigned int gcry_mpi_get_nbits(gcry_mpi_t a ) ;
# 701 "/usr/include/gcrypt.h"
extern int gcry_mpi_test_bit(gcry_mpi_t a , unsigned int n ) ;
# 704 "/usr/include/gcrypt.h"
extern void gcry_mpi_set_bit(gcry_mpi_t a , unsigned int n ) ;
# 707 "/usr/include/gcrypt.h"
extern void gcry_mpi_clear_bit(gcry_mpi_t a , unsigned int n ) ;
# 710 "/usr/include/gcrypt.h"
extern void gcry_mpi_set_highbit(gcry_mpi_t a , unsigned int n ) ;
# 713 "/usr/include/gcrypt.h"
extern void gcry_mpi_clear_highbit(gcry_mpi_t a , unsigned int n ) ;
# 716 "/usr/include/gcrypt.h"
extern void gcry_mpi_rshift(gcry_mpi_t x , gcry_mpi_t a , unsigned int n ) ;
# 719 "/usr/include/gcrypt.h"
extern void gcry_mpi_lshift(gcry_mpi_t x , gcry_mpi_t a , unsigned int n ) ;
# 724 "/usr/include/gcrypt.h"
extern gcry_mpi_t gcry_mpi_set_opaque(gcry_mpi_t a , void *p , unsigned int nbits ) ;
# 729 "/usr/include/gcrypt.h"
extern void *gcry_mpi_get_opaque(gcry_mpi_t a , unsigned int *nbits ) ;
# 734 "/usr/include/gcrypt.h"
extern void gcry_mpi_set_flag(gcry_mpi_t a , enum gcry_mpi_flag flag ) ;
# 738 "/usr/include/gcrypt.h"
extern void gcry_mpi_clear_flag(gcry_mpi_t a , enum gcry_mpi_flag flag ) ;
# 741 "/usr/include/gcrypt.h"
extern int gcry_mpi_get_flag(gcry_mpi_t a , enum gcry_mpi_flag flag ) ;
# 873 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_open(gcry_cipher_hd_t *handle , int algo , int mode ,
                                     unsigned int flags ) ;
# 877 "/usr/include/gcrypt.h"
extern void gcry_cipher_close(gcry_cipher_hd_t h ) ;
# 880 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_ctl(gcry_cipher_hd_t h , int cmd , void *buffer ,
                                    size_t buflen ) ;
# 884 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_info(gcry_cipher_hd_t h , int what , void *buffer ,
                                     size_t *nbytes ) ;
# 888 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_algo_info(int algo , int what , void *buffer , size_t *nbytes ) ;
# 894 "/usr/include/gcrypt.h"
extern char const *gcry_cipher_algo_name(int algorithm ) __attribute__((__pure__)) ;
# 898 "/usr/include/gcrypt.h"
extern int gcry_cipher_map_name(char const *name ) __attribute__((__pure__)) ;
# 903 "/usr/include/gcrypt.h"
extern int gcry_cipher_mode_from_oid(char const *string ) __attribute__((__pure__)) ;
# 909 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_encrypt(gcry_cipher_hd_t h , void *out , size_t outsize ,
                                        void const *in , size_t inlen ) ;
# 914 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_decrypt(gcry_cipher_hd_t h , void *out , size_t outsize ,
                                        void const *in , size_t inlen ) ;
# 919 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_setkey(gcry_cipher_hd_t hd , void const *key , size_t keylen ) ;
# 924 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_setiv(gcry_cipher_hd_t hd , void const *iv , size_t ivlen ) ;
# 941 "/usr/include/gcrypt.h"
extern gpg_error_t gcry_cipher_setctr(gcry_cipher_hd_t hd , void const *ctr , size_t ctrlen ) ;
# 945 "/usr/include/gcrypt.h"
extern size_t gcry_cipher_get_algo_keylen(int algo ) ;
# 948 "/usr/include/gcrypt.h"
extern size_t gcry_cipher_get_algo_blklen(int algo ) ;
# 960 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_cipher_list(int *list , int *list_length ) ;
# 991 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_encrypt(gcry_sexp_t *result , gcry_sexp_t data , gcry_sexp_t pkey ) ;
# 996 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_decrypt(gcry_sexp_t *result , gcry_sexp_t data , gcry_sexp_t skey ) ;
# 1001 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_sign(gcry_sexp_t *result , gcry_sexp_t data , gcry_sexp_t skey ) ;
# 1005 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_verify(gcry_sexp_t sigval , gcry_sexp_t data , gcry_sexp_t pkey ) ;
# 1009 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_testkey(gcry_sexp_t key ) ;
# 1014 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_genkey(gcry_sexp_t *r_key , gcry_sexp_t s_parms ) ;
# 1017 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_ctl(int cmd , void *buffer , size_t buflen ) ;
# 1020 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_algo_info(int algo , int what , void *buffer , size_t *nbytes ) ;
# 1026 "/usr/include/gcrypt.h"
extern char const *gcry_pk_algo_name(int algorithm ) __attribute__((__pure__)) ;
# 1030 "/usr/include/gcrypt.h"
extern int gcry_pk_map_name(char const *name ) __attribute__((__pure__)) ;
# 1034 "/usr/include/gcrypt.h"
extern unsigned int gcry_pk_get_nbits(gcry_sexp_t key ) __attribute__((__pure__)) ;
# 1038 "/usr/include/gcrypt.h"
extern unsigned char *gcry_pk_get_keygrip(gcry_sexp_t key , unsigned char *array ) ;
# 1041 "/usr/include/gcrypt.h"
extern char const *gcry_pk_get_curve(gcry_sexp_t key , int iterator , unsigned int *r_nbits ) ;
# 1046 "/usr/include/gcrypt.h"
extern gcry_sexp_t gcry_pk_get_param(int algo , char const *name ) ;
# 1058 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_pk_list(int *list , int *list_length ) ;
# 1126 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_open(gcry_md_hd_t *h , int algo , unsigned int flags ) ;
# 1129 "/usr/include/gcrypt.h"
extern void gcry_md_close(gcry_md_hd_t hd ) ;
# 1132 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_enable(gcry_md_hd_t hd , int algo ) ;
# 1135 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_copy(gcry_md_hd_t *bhd , gcry_md_hd_t ahd ) ;
# 1138 "/usr/include/gcrypt.h"
extern void gcry_md_reset(gcry_md_hd_t hd ) ;
# 1141 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_ctl(gcry_md_hd_t hd , int cmd , void *buffer , size_t buflen ) ;
# 1147 "/usr/include/gcrypt.h"
extern void gcry_md_write(gcry_md_hd_t hd , void const *buffer , size_t length ) ;
# 1151 "/usr/include/gcrypt.h"
extern unsigned char *gcry_md_read(gcry_md_hd_t hd , int algo ) ;
# 1158 "/usr/include/gcrypt.h"
extern void gcry_md_hash_buffer(int algo , void *digest , void const *buffer , size_t length ) ;
# 1163 "/usr/include/gcrypt.h"
extern int gcry_md_get_algo(gcry_md_hd_t hd ) ;
# 1167 "/usr/include/gcrypt.h"
extern unsigned int gcry_md_get_algo_dlen(int algo ) ;
# 1171 "/usr/include/gcrypt.h"
extern int gcry_md_is_enabled(gcry_md_hd_t a , int algo ) ;
# 1174 "/usr/include/gcrypt.h"
extern int gcry_md_is_secure(gcry_md_hd_t a ) ;
# 1177 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_info(gcry_md_hd_t h , int what , void *buffer , size_t *nbytes ) ;
# 1181 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_algo_info(int algo , int what , void *buffer , size_t *nbytes ) ;
# 1187 "/usr/include/gcrypt.h"
extern char const *gcry_md_algo_name(int algo ) __attribute__((__pure__)) ;
# 1191 "/usr/include/gcrypt.h"
extern int gcry_md_map_name(char const *name ) __attribute__((__pure__)) ;
# 1195 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_setkey(gcry_md_hd_t hd , void const *key , size_t keylen ) ;
# 1200 "/usr/include/gcrypt.h"
extern void gcry_md_debug(gcry_md_hd_t hd , char const *suffix ) ;
# 1249 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_md_list(int *list , int *list_length ) ;
# 1407 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_new(gcry_ac_data_t *data ) __attribute__((__deprecated__)) ;
# 1411 "/usr/include/gcrypt.h"
extern void gcry_ac_data_destroy(gcry_ac_data_t data ) __attribute__((__deprecated__)) ;
# 1415 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_copy(gcry_ac_data_t *data_cp , gcry_ac_data_t data ) __attribute__((__deprecated__)) ;
# 1421 "/usr/include/gcrypt.h"
extern unsigned int gcry_ac_data_length(gcry_ac_data_t data ) __attribute__((__deprecated__)) ;
# 1425 "/usr/include/gcrypt.h"
extern void gcry_ac_data_clear(gcry_ac_data_t data ) __attribute__((__deprecated__)) ;
# 1433 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_set(gcry_ac_data_t data , unsigned int flags , char const *name ,
                                     gcry_mpi_t mpi ) __attribute__((__deprecated__)) ;
# 1440 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_get_name(gcry_ac_data_t data , unsigned int flags ,
                                          char const *name , gcry_mpi_t *mpi ) __attribute__((__deprecated__)) ;
# 1448 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_get_index(gcry_ac_data_t data , unsigned int flags ,
                                           unsigned int idx , char const **name ,
                                           gcry_mpi_t *mpi ) __attribute__((__deprecated__)) ;
# 1456 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_to_sexp(gcry_ac_data_t data , gcry_sexp_t *sexp ,
                                         char const **identifiers ) __attribute__((__deprecated__)) ;
# 1463 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_from_sexp(gcry_ac_data_t *data , gcry_sexp_t sexp ,
                                           char const **identifiers ) __attribute__((__deprecated__)) ;
# 1470 "/usr/include/gcrypt.h"
extern void gcry_ac_io_init(gcry_ac_io_t *ac_io , gcry_ac_io_mode_t mode , gcry_ac_io_type_t type
                            , ...) __attribute__((__deprecated__)) ;
# 1477 "/usr/include/gcrypt.h"
extern void gcry_ac_io_init_va(gcry_ac_io_t *ac_io , gcry_ac_io_mode_t mode , gcry_ac_io_type_t type ,
                               va_list ap ) __attribute__((__deprecated__)) ;
# 1482 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_open(gcry_ac_handle_t *handle , gcry_ac_id_t algorithm ,
                                 unsigned int flags ) __attribute__((__deprecated__)) ;
# 1487 "/usr/include/gcrypt.h"
extern void gcry_ac_close(gcry_ac_handle_t handle ) __attribute__((__deprecated__)) ;
# 1491 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_key_init(gcry_ac_key_t *key , gcry_ac_handle_t handle ,
                                     gcry_ac_key_type_t type , gcry_ac_data_t data ) __attribute__((__deprecated__)) ;
# 1500 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_key_pair_generate(gcry_ac_handle_t handle , unsigned int nbits ,
                                              void *spec , gcry_ac_key_pair_t *key_pair ,
                                              gcry_mpi_t **misc_data ) __attribute__((__deprecated__)) ;
# 1507 "/usr/include/gcrypt.h"
extern gcry_ac_key_t gcry_ac_key_pair_extract(gcry_ac_key_pair_t key_pair , gcry_ac_key_type_t which ) __attribute__((__deprecated__)) ;
# 1512 "/usr/include/gcrypt.h"
extern gcry_ac_data_t gcry_ac_key_data_get(gcry_ac_key_t key ) __attribute__((__deprecated__)) ;
# 1516 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_key_test(gcry_ac_handle_t handle , gcry_ac_key_t key ) __attribute__((__deprecated__)) ;
# 1520 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_key_get_nbits(gcry_ac_handle_t handle , gcry_ac_key_t key ,
                                          unsigned int *nbits ) __attribute__((__deprecated__)) ;
# 1526 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_key_get_grip(gcry_ac_handle_t handle , gcry_ac_key_t key ,
                                         unsigned char *key_grip ) __attribute__((__deprecated__)) ;
# 1531 "/usr/include/gcrypt.h"
extern void gcry_ac_key_destroy(gcry_ac_key_t key ) __attribute__((__deprecated__)) ;
# 1535 "/usr/include/gcrypt.h"
extern void gcry_ac_key_pair_destroy(gcry_ac_key_pair_t key_pair ) __attribute__((__deprecated__)) ;
# 1541 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_encode(gcry_ac_em_t method , unsigned int flags ,
                                        void *options , gcry_ac_io_t *io_read , gcry_ac_io_t *io_write ) __attribute__((__deprecated__)) ;
# 1550 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_decode(gcry_ac_em_t method , unsigned int flags ,
                                        void *options , gcry_ac_io_t *io_read , gcry_ac_io_t *io_write ) __attribute__((__deprecated__)) ;
# 1559 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_encrypt(gcry_ac_handle_t handle , unsigned int flags ,
                                         gcry_ac_key_t key , gcry_mpi_t data_plain ,
                                         gcry_ac_data_t *data_encrypted ) __attribute__((__deprecated__)) ;
# 1569 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_decrypt(gcry_ac_handle_t handle , unsigned int flags ,
                                         gcry_ac_key_t key , gcry_mpi_t *data_plain ,
                                         gcry_ac_data_t data_encrypted ) __attribute__((__deprecated__)) ;
# 1578 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_sign(gcry_ac_handle_t handle , gcry_ac_key_t key ,
                                      gcry_mpi_t data , gcry_ac_data_t *data_signature ) __attribute__((__deprecated__)) ;
# 1587 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_verify(gcry_ac_handle_t handle , gcry_ac_key_t key ,
                                        gcry_mpi_t data , gcry_ac_data_t data_signature ) __attribute__((__deprecated__)) ;
# 1598 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_encrypt_scheme(gcry_ac_handle_t handle , gcry_ac_scheme_t scheme ,
                                                unsigned int flags , void *opts ,
                                                gcry_ac_key_t key , gcry_ac_io_t *io_message ,
                                                gcry_ac_io_t *io_cipher ) __attribute__((__deprecated__)) ;
# 1611 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_decrypt_scheme(gcry_ac_handle_t handle , gcry_ac_scheme_t scheme ,
                                                unsigned int flags , void *opts ,
                                                gcry_ac_key_t key , gcry_ac_io_t *io_cipher ,
                                                gcry_ac_io_t *io_message ) __attribute__((__deprecated__)) ;
# 1624 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_sign_scheme(gcry_ac_handle_t handle , gcry_ac_scheme_t scheme ,
                                             unsigned int flags , void *opts , gcry_ac_key_t key ,
                                             gcry_ac_io_t *io_message , gcry_ac_io_t *io_signature ) __attribute__((__deprecated__)) ;
# 1638 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_data_verify_scheme(gcry_ac_handle_t handle , gcry_ac_scheme_t scheme ,
                                               unsigned int flags , void *opts , gcry_ac_key_t key ,
                                               gcry_ac_io_t *io_message , gcry_ac_io_t *io_signature ) __attribute__((__deprecated__)) ;
# 1649 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_id_to_name(gcry_ac_id_t algorithm , char const **name ) __attribute__((__deprecated__)) ;
# 1655 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_ac_name_to_id(char const *name , gcry_ac_id_t *algorithm ) __attribute__((__deprecated__)) ;
# 1679 "/usr/include/gcrypt.h"
extern gpg_error_t gcry_kdf_derive(void const *passphrase , size_t passphraselen ,
                                   int algo , int subalgo , void const *salt , size_t saltlen ,
                                   unsigned long iterations , size_t keysize , void *keybuffer ) ;
# 1708 "/usr/include/gcrypt.h"
extern void gcry_randomize(void *buffer , size_t length , enum gcry_random_level level ) ;
# 1714 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_random_add_bytes(void const *buffer , size_t length , int quality ) ;
# 1725 "/usr/include/gcrypt.h"
extern void *gcry_random_bytes(size_t nbytes , enum gcry_random_level level ) __attribute__((__malloc__)) ;
# 1731 "/usr/include/gcrypt.h"
extern void *gcry_random_bytes_secure(size_t nbytes , enum gcry_random_level level ) __attribute__((__malloc__)) ;
# 1738 "/usr/include/gcrypt.h"
extern void gcry_mpi_randomize(gcry_mpi_t w , unsigned int nbits , enum gcry_random_level level ) ;
# 1743 "/usr/include/gcrypt.h"
extern void gcry_create_nonce(void *buffer , size_t length ) ;
# 1780 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_prime_generate(gcry_mpi_t *prime , unsigned int prime_bits ,
                                        unsigned int factor_bits , gcry_mpi_t **factors ,
                                        int (*cb_func)(void *arg , int mode , gcry_mpi_t candidate ) ,
                                        void *cb_arg , gcry_random_level_t random_level ,
                                        unsigned int flags ) ;
# 1793 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_prime_group_generator(gcry_mpi_t *r_g , gcry_mpi_t prime ,
                                               gcry_mpi_t *factors , gcry_mpi_t start_g ) ;
# 1800 "/usr/include/gcrypt.h"
extern void gcry_prime_release_factors(gcry_mpi_t *factors ) ;
# 1804 "/usr/include/gcrypt.h"
extern gcry_error_t gcry_prime_check(gcry_mpi_t x , unsigned int flags ) ;
# 1852 "/usr/include/gcrypt.h"
extern void gcry_set_progress_handler(void (*cb)(void * , char const * , int ,
                                                 int , int ) , void *cb_data ) ;
# 1856 "/usr/include/gcrypt.h"
extern void gcry_set_allocation_handler(void *(*func_alloc)(size_t n ) , void *(*func_alloc_secure)(size_t n ) ,
                                        int (*func_secure_check)(void const * ) ,
                                        void *(*func_realloc)(void *p , size_t n ) ,
                                        void (*func_free)(void * ) ) ;
# 1865 "/usr/include/gcrypt.h"
extern void gcry_set_outofcore_handler(int (*h)(void * , size_t , unsigned int ) ,
                                       void *opaque ) ;
# 1869 "/usr/include/gcrypt.h"
extern void gcry_set_fatalerror_handler(void (*fnc)(void * , int , char const * ) ,
                                        void *opaque ) ;
# 1873 "/usr/include/gcrypt.h"
extern void gcry_set_log_handler(void (*f)(void * , int , char const * , va_list ) ,
                                 void *opaque ) ;
# 1876 "/usr/include/gcrypt.h"
extern void gcry_set_gettext_handler(char const *(*f)(char const * ) ) ;
# 1880 "/usr/include/gcrypt.h"
extern void *gcry_malloc(size_t n ) __attribute__((__malloc__)) ;
# 1881 "/usr/include/gcrypt.h"
extern void *gcry_calloc(size_t n , size_t m ) __attribute__((__malloc__)) ;
# 1882 "/usr/include/gcrypt.h"
extern void *gcry_malloc_secure(size_t n ) __attribute__((__malloc__)) ;
# 1883 "/usr/include/gcrypt.h"
extern void *gcry_calloc_secure(size_t n , size_t m ) __attribute__((__malloc__)) ;
# 1884 "/usr/include/gcrypt.h"
extern void *gcry_realloc(void *a , size_t n ) ;
# 1885 "/usr/include/gcrypt.h"
extern char *gcry_strdup(char const *string ) __attribute__((__malloc__)) ;
# 1886 "/usr/include/gcrypt.h"
extern void *gcry_xmalloc(size_t n ) __attribute__((__malloc__)) ;
# 1887 "/usr/include/gcrypt.h"
extern void *gcry_xcalloc(size_t n , size_t m ) __attribute__((__malloc__)) ;
# 1888 "/usr/include/gcrypt.h"
extern void *gcry_xmalloc_secure(size_t n ) __attribute__((__malloc__)) ;
# 1889 "/usr/include/gcrypt.h"
extern void *gcry_xcalloc_secure(size_t n , size_t m ) __attribute__((__malloc__)) ;
# 1890 "/usr/include/gcrypt.h"
extern void *gcry_xrealloc(void *a , size_t n ) ;
# 1891 "/usr/include/gcrypt.h"
extern char *gcry_xstrdup(char const *a ) __attribute__((__malloc__)) ;
# 1892 "/usr/include/gcrypt.h"
extern void gcry_free(void *a ) ;
# 1895 "/usr/include/gcrypt.h"
extern int gcry_is_secure(void const *a ) __attribute__((__pure__)) ;
# 99 "/usr/include/gcrypt-module.h"
extern gcry_error_t gcry_cipher_register(gcry_cipher_spec_t *cipher , int *algorithm_id ,
                                         gcry_module_t *module ) __attribute__((__deprecated__)) ;
# 107 "/usr/include/gcrypt-module.h"
extern void gcry_cipher_unregister(gcry_module_t module ) __attribute__((__deprecated__)) ;
# 177 "/usr/include/gcrypt-module.h"
extern gcry_error_t gcry_pk_register(gcry_pk_spec_t *pubkey , unsigned int *algorithm_id ,
                                     gcry_module_t *module ) __attribute__((__deprecated__)) ;
# 184 "/usr/include/gcrypt-module.h"
extern void gcry_pk_unregister(gcry_module_t module ) __attribute__((__deprecated__)) ;
# 224 "/usr/include/gcrypt-module.h"
extern gcry_error_t gcry_md_register(gcry_md_spec_t *digest , unsigned int *algorithm_id ,
                                     gcry_module_t *module ) __attribute__((__deprecated__)) ;
# 231 "/usr/include/gcrypt-module.h"
extern void gcry_md_unregister(gcry_module_t module ) __attribute__((__deprecated__)) ;
# 40 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
static struct __anonstruct___obliv_c__bool_42 const __obliv_c__trueCond = {{{(_Bool )0, {(_Bool )1}}}};
# 44 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__assignBitKnown(OblivBit___0 *dest , _Bool value ) ;
# 45 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__copyBit(OblivBit___0 *dest , OblivBit___0 const *src ) ;
# 46 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern _Bool __obliv_c__bitIsKnown(_Bool *val , OblivBit___0 const *bit ) ;
# 49 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitAnd(OblivBit___0 *dest , OblivBit___0 const *a , OblivBit___0 const *b ) ;
# 50 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitOr(OblivBit___0 *dest , OblivBit___0 const *a , OblivBit___0 const *b ) ;
# 51 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitXor(OblivBit___0 *dest , OblivBit___0 const *a , OblivBit___0 const *b ) ;
# 52 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitNot(OblivBit___0 *dest , OblivBit___0 const *a ) ;
# 53 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__flipBit(OblivBit___0 *dest ) ;
# 58 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern int ocCurrentParty(void) ;
# 63 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setSignedKnown(void *dest , size_t size , long long value ) ;
# 65 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setUnsignedKnown(void *dest , size_t size , unsigned long long value ) ;
# 67 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitsKnown(OblivBit___0 *dest , _Bool const *value , size_t size ) ;
# 68 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__copyBits(OblivBit___0 *dest , OblivBit___0 const *src , size_t size ) ;
# 70 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern _Bool __obliv_c__allBitsKnown(OblivBit___0 const *bits , _Bool *dest , size_t size ) ;
# 72 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitwiseAnd(void *dest , void const *op1 , void const *op2 ,
                                     size_t size ) ;
# 75 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitwiseOr(void *dest , void const *op1 , void const *op2 ,
                                    size_t size ) ;
# 78 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitwiseXor(void *dest , void const *op1 , void const *op2 ,
                                     size_t size ) ;
# 81 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitwiseNot(void *dest , void const *op , size_t size ) ;
# 82 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitwiseNotInPlace(void *dest , size_t size ) ;
# 83 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLShift(void *vdest , void const *vsrc , size_t size ,
                                 unsigned int shiftAmt ) ;
# 85 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setRShiftSigned(void *vdest , void const *vsrc , size_t size ,
                                       unsigned int shiftAmt ) ;
# 87 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setRShiftUnsigned(void *vdest , void const *vsrc , size_t size ,
                                         unsigned int shiftAmt ) ;
# 89 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setRShift(void *vdest , void const *vsrc , size_t size ,
                                 unsigned int shiftAmt , _Bool isSigned ) ;
# 97 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setPlainAdd(void *vdest , void const *vop1 , void const *vop2 ,
                                   size_t size ) ;
# 100 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setPlainSub(void *vdest , void const *vop1 , void const *vop2 ,
                                   size_t size ) ;
# 103 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitsAdd(void *dest , void *carryOut , void const *op1 ,
                                  void const *op2 , void const *carryIn , size_t size ) ;
# 107 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setNeg(void *vdest , void const *vsrc , size_t n ) ;
# 108 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__condNeg(void const *vcond , void *vdest , void const *vsrc ,
                               size_t n ) ;
# 112 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setMul(void *vdest , void const *vop1 , void const *vop2 ,
                              size_t size ) ;
# 115 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setDivModUnsigned(void *vquot , void *vrem , void const *vop1 ,
                                         void const *vop2 , size_t size ) ;
# 118 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setDivModSigned(void *vquot , void *vrem , void const *vop1 ,
                                       void const *vop2 , size_t size ) ;
# 121 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setDivUnsigned(void *vdest , void const *vop1 , void const *vop2 ,
                                      size_t size ) ;
# 124 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setModUnsigned(void *vdest , void const *vop1 , void const *vop2 ,
                                      size_t size ) ;
# 127 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setDivSigned(void *vdest , void const *vop1 , void const *vop2 ,
                                    size_t size ) ;
# 130 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setModSigned(void *vdest , void const *vop1 , void const *vop2 ,
                                    size_t size ) ;
# 134 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setBitsSub(void *dest , void *borrowOut , void const *op1 ,
                                  void const *op2 , void const *borrowIn , size_t size ) ;
# 137 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setSignExtend(void *dest , size_t dsize , void const *src ,
                                     size_t ssize ) ;
# 139 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setZeroExtend(void *dest , size_t dsize , void const *src ,
                                     size_t ssize ) ;
# 141 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__ifThenElse(void *dest , void const *tsrc , void const *fsrc ,
                                  size_t size , void const *cond ) ;
# 144 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLessThanUnit(OblivBit___0 *ltOut , OblivBit___0 const *op1 ,
                                       OblivBit___0 const *op2 , size_t size , OblivBit___0 const *ltIn ) ;
# 147 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLessThanUnsigned(void *dest , void const *op1 , void const *op2 ,
                                           size_t size ) ;
# 150 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLessOrEqualUnsigned(void *dest , void const *op1 , void const *op2 ,
                                              size_t size ) ;
# 153 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLessThanSigned(void *dest , void const *op1 , void const *op2 ,
                                         size_t size ) ;
# 156 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLessOrEqualSigned(void *dest , void const *op1 , void const *op2 ,
                                            size_t size ) ;
# 159 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setEqualTo(void *dest , void const *op1 , void const *op2 ,
                                  size_t size ) ;
# 162 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setNotEqual(void *dest , void const *op1 , void const *op2 ,
                                   size_t size ) ;
# 165 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__setLogicalNot(void *dest , void const *op , size_t size ) ;
# 167 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__condAssign(void const *cond , void *dest , void const *src ,
                                  size_t size ) ;
# 170 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
__inline static void __obliv_c__condAssignKnown(void const *cond , void *dest ,
                                                size_t size , widest_t val )
{
  OblivBit___0 ov[(unsigned long )8 * (unsigned long )sizeof(widest_t )] ;
  unsigned long __cil_tmp6 ;
  unsigned long __cil_tmp7 ;
  OblivBit___0 *__cil_tmp8 ;
  void *__cil_tmp9 ;
  unsigned long __cil_tmp10 ;
  unsigned long __cil_tmp11 ;
  OblivBit___0 *__cil_tmp12 ;
  void const *__cil_tmp13 ;
  void const *__cil_tmp14 ;

  {
# 49 "mutual.oc"
  memset((void *)(& ov), 0, sizeof(OblivBit___0 [(unsigned long )8 * (unsigned long )sizeof(widest_t )]));
# 175 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp6 = 0 * 13UL;
# 175 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp7 = (unsigned long )(ov) + __cil_tmp6;
# 175 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp8 = (OblivBit___0 *)__cil_tmp7;
# 175 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp9 = (void *)__cil_tmp8;
# 175 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __obliv_c__setSignedKnown(__cil_tmp9, size, val);
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp10 = 0 * 13UL;
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp11 = (unsigned long )(ov) + __cil_tmp10;
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp12 = (OblivBit___0 *)__cil_tmp11;
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp13 = (void const *)__cil_tmp12;
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __cil_tmp14 = (void const *)dest;
# 176 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  __obliv_c__ifThenElse(dest, __cil_tmp13, __cil_tmp14, size, cond);
# 177 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
  return;
}
}
# 179 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__condAdd(void const *c , void *dest , void const *x , size_t size ) ;
# 182 "/home/student/obliv-c/src/ext/oblivc/obliv_bits.h"
extern void __obliv_c__condSub(void const *c , void *dest , void const *x , size_t size ) ;
# 348 "/usr/include/libio.h"
extern struct _IO_FILE_plus _IO_2_1_stdin_ ;
# 349 "/usr/include/libio.h"
extern struct _IO_FILE_plus _IO_2_1_stdout_ ;
# 350 "/usr/include/libio.h"
extern struct _IO_FILE_plus _IO_2_1_stderr_ ;
# 418 "/usr/include/libio.h"
extern int __underflow(_IO_FILE * ) ;
# 419 "/usr/include/libio.h"
extern int __uflow(_IO_FILE * ) ;
# 420 "/usr/include/libio.h"
extern int __overflow(_IO_FILE * , int ) ;
# 462 "/usr/include/libio.h"
extern int _IO_getc(_IO_FILE *__fp ) ;
# 463 "/usr/include/libio.h"
extern int _IO_putc(int __c , _IO_FILE *__fp ) ;
# 464 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) _IO_feof)(_IO_FILE *__fp ) ;
# 465 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) _IO_ferror)(_IO_FILE *__fp ) ;
# 467 "/usr/include/libio.h"
extern int _IO_peekc_locked(_IO_FILE *__fp ) ;
# 473 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) _IO_flockfile)(_IO_FILE * ) ;
# 474 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) _IO_funlockfile)(_IO_FILE * ) ;
# 475 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) _IO_ftrylockfile)(_IO_FILE * ) ;
# 492 "/usr/include/libio.h"
extern int _IO_vfscanf(_IO_FILE * __restrict , char const * __restrict , __gnuc_va_list ,
                       int * __restrict ) ;
# 494 "/usr/include/libio.h"
extern int _IO_vfprintf(_IO_FILE * __restrict , char const * __restrict , __gnuc_va_list ) ;
# 496 "/usr/include/libio.h"
extern __ssize_t _IO_padn(_IO_FILE * , int , __ssize_t ) ;
# 497 "/usr/include/libio.h"
extern size_t _IO_sgetn(_IO_FILE * , void * , size_t ) ;
# 499 "/usr/include/libio.h"
extern __off64_t _IO_seekoff(_IO_FILE * , __off64_t , int , int ) ;
# 500 "/usr/include/libio.h"
extern __off64_t _IO_seekpos(_IO_FILE * , __off64_t , int ) ;
# 502 "/usr/include/libio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) _IO_free_backup_area)(_IO_FILE * ) ;
# 169 "/usr/include/stdio.h"
extern struct _IO_FILE *stdin ;
# 170 "/usr/include/stdio.h"
extern struct _IO_FILE *stdout ;
# 171 "/usr/include/stdio.h"
extern struct _IO_FILE *stderr ;
# 179 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) remove)(char const *__filename ) ;
# 181 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) rename)(char const *__old ,
                                                                             char const *__new ) ;
# 186 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) renameat)(int __oldfd ,
                                                                               char const *__old ,
                                                                               int __newfd ,
                                                                               char const *__new ) ;
# 196 "/usr/include/stdio.h"
extern FILE *tmpfile(void) ;
# 210 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) tmpnam)(char *__s ) ;
# 216 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) tmpnam_r)(char *__s ) ;
# 228 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) tempnam)(char const *__dir ,
                                                                                char const *__pfx ) __attribute__((__malloc__)) ;
# 238 "/usr/include/stdio.h"
extern int fclose(FILE *__stream ) ;
# 243 "/usr/include/stdio.h"
extern int fflush(FILE *__stream ) ;
# 253 "/usr/include/stdio.h"
extern int fflush_unlocked(FILE *__stream ) ;
# 273 "/usr/include/stdio.h"
extern FILE *fopen(char const * __restrict __filename , char const * __restrict __modes ) ;
# 279 "/usr/include/stdio.h"
extern FILE *freopen(char const * __restrict __filename , char const * __restrict __modes ,
                     FILE * __restrict __stream ) ;
# 307 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) FILE *( __attribute__((__leaf__)) fdopen)(int __fd ,
                                                                               char const *__modes ) ;
# 320 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) FILE *( __attribute__((__leaf__)) fmemopen)(void *__s ,
                                                                                 size_t __len ,
                                                                                 char const *__modes ) ;
# 326 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) FILE *( __attribute__((__leaf__)) open_memstream)(char **__bufloc ,
                                                                                       size_t *__sizeloc ) ;
# 333 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) setbuf)(FILE * __restrict __stream ,
                                                                              char * __restrict __buf ) ;
# 337 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) setvbuf)(FILE * __restrict __stream ,
                                                                              char * __restrict __buf ,
                                                                              int __modes ,
                                                                              size_t __n ) ;
# 344 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) setbuffer)(FILE * __restrict __stream ,
                                                                                 char * __restrict __buf ,
                                                                                 size_t __size ) ;
# 348 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) setlinebuf)(FILE *__stream ) ;
# 357 "/usr/include/stdio.h"
extern int fprintf(FILE * __restrict __stream , char const * __restrict __format
                   , ...) ;
# 363 "/usr/include/stdio.h"
extern int printf(char const * __restrict __format , ...) ;
# 365 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int sprintf(char * __restrict __s , char const * __restrict __format
                                                 , ...) ;
# 372 "/usr/include/stdio.h"
extern int vfprintf(FILE * __restrict __s , char const * __restrict __format ,
                    __gnuc_va_list __arg ) ;
# 378 "/usr/include/stdio.h"
extern int vprintf(char const * __restrict __format , __gnuc_va_list __arg ) ;
# 380 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int vsprintf(char * __restrict __s , char const * __restrict __format ,
                                                  __gnuc_va_list __arg ) ;
# 387 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( snprintf)(char * __restrict __s ,
                                                                             size_t __maxlen ,
                                                                             char const * __restrict __format
                                                                             , ...) ;
# 391 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( vsnprintf)(char * __restrict __s ,
                                                                              size_t __maxlen ,
                                                                              char const * __restrict __format ,
                                                                              __gnuc_va_list __arg ) ;
# 418 "/usr/include/stdio.h"
extern int ( vdprintf)(int __fd , char const * __restrict __fmt ,
                                               __gnuc_va_list __arg ) ;
# 421 "/usr/include/stdio.h"
extern int ( dprintf)(int __fd , char const * __restrict __fmt
                                              , ...) ;
# 431 "/usr/include/stdio.h"
extern int fscanf(FILE * __restrict __stream , char const * __restrict __format
                  , ...) __asm__("__isoc99_fscanf") ;
# 437 "/usr/include/stdio.h"
extern int scanf(char const * __restrict __format , ...) __asm__("__isoc99_scanf") ;
# 439 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) sscanf)(char const * __restrict __s ,
                                                                             char const * __restrict __format
                                                                             , ...) __asm__("__isoc99_sscanf") ;
# 477 "/usr/include/stdio.h"
extern int ( vfscanf)(FILE * __restrict __s , char const * __restrict __format ,
                                              __gnuc_va_list __arg ) __asm__("__isoc99_vfscanf") ;
# 485 "/usr/include/stdio.h"
extern int ( vscanf)(char const * __restrict __format ,
                                             __gnuc_va_list __arg ) __asm__("__isoc99_vscanf") ;
# 489 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) vsscanf)(char const * __restrict __s ,
                                                                                                     char const * __restrict __format ,
                                                                                                     __gnuc_va_list __arg ) __asm__("__isoc99_vsscanf") ;
# 537 "/usr/include/stdio.h"
extern int fgetc(FILE *__stream ) ;
# 538 "/usr/include/stdio.h"
extern int getc(FILE *__stream ) ;
# 544 "/usr/include/stdio.h"
extern int getchar(void) ;
# 556 "/usr/include/stdio.h"
extern int getc_unlocked(FILE *__stream ) ;
# 557 "/usr/include/stdio.h"
extern int getchar_unlocked(void) ;
# 567 "/usr/include/stdio.h"
extern int fgetc_unlocked(FILE *__stream ) ;
# 579 "/usr/include/stdio.h"
extern int fputc(int __c , FILE *__stream ) ;
# 580 "/usr/include/stdio.h"
extern int putc(int __c , FILE *__stream ) ;
# 586 "/usr/include/stdio.h"
extern int putchar(int __c ) ;
# 600 "/usr/include/stdio.h"
extern int fputc_unlocked(int __c , FILE *__stream ) ;
# 608 "/usr/include/stdio.h"
extern int putc_unlocked(int __c , FILE *__stream ) ;
# 609 "/usr/include/stdio.h"
extern int putchar_unlocked(int __c ) ;
# 616 "/usr/include/stdio.h"
extern int getw(FILE *__stream ) ;
# 619 "/usr/include/stdio.h"
extern int putw(int __w , FILE *__stream ) ;
# 628 "/usr/include/stdio.h"
extern char *fgets(char * __restrict __s , int __n , FILE * __restrict __stream ) ;
# 636 "/usr/include/stdio.h"
extern char *gets(char *__s ) ;
# 662 "/usr/include/stdio.h"
extern __ssize_t __getdelim(char ** __restrict __lineptr , size_t * __restrict __n ,
                            int __delimiter , FILE * __restrict __stream ) ;
# 665 "/usr/include/stdio.h"
extern __ssize_t getdelim(char ** __restrict __lineptr , size_t * __restrict __n ,
                          int __delimiter , FILE * __restrict __stream ) ;
# 675 "/usr/include/stdio.h"
extern __ssize_t getline(char ** __restrict __lineptr , size_t * __restrict __n ,
                         FILE * __restrict __stream ) ;
# 686 "/usr/include/stdio.h"
extern int fputs(char const * __restrict __s , FILE * __restrict __stream ) ;
# 692 "/usr/include/stdio.h"
extern int puts(char const *__s ) ;
# 699 "/usr/include/stdio.h"
extern int ungetc(int __c , FILE *__stream ) ;
# 706 "/usr/include/stdio.h"
extern size_t fread(void * __restrict __ptr , size_t __size , size_t __n , FILE * __restrict __stream ) ;
# 712 "/usr/include/stdio.h"
extern size_t fwrite(void const * __restrict __ptr , size_t __size , size_t __n ,
                     FILE * __restrict __s ) ;
# 734 "/usr/include/stdio.h"
extern size_t fread_unlocked(void * __restrict __ptr , size_t __size , size_t __n ,
                             FILE * __restrict __stream ) ;
# 736 "/usr/include/stdio.h"
extern size_t fwrite_unlocked(void const * __restrict __ptr , size_t __size , size_t __n ,
                              FILE * __restrict __stream ) ;
# 746 "/usr/include/stdio.h"
extern int fseek(FILE *__stream , long __off , int __whence ) ;
# 751 "/usr/include/stdio.h"
extern long ftell(FILE *__stream ) ;
# 756 "/usr/include/stdio.h"
extern void rewind(FILE *__stream ) ;
# 770 "/usr/include/stdio.h"
extern int fseeko(FILE *__stream , __off_t __off , int __whence ) ;
# 775 "/usr/include/stdio.h"
extern __off_t ftello(FILE *__stream ) ;
# 795 "/usr/include/stdio.h"
extern int fgetpos(FILE * __restrict __stream , fpos_t * __restrict __pos ) ;
# 800 "/usr/include/stdio.h"
extern int fsetpos(FILE *__stream , fpos_t const *__pos ) ;
# 823 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) clearerr)(FILE *__stream ) ;
# 825 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) feof)(FILE *__stream ) ;
# 827 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) ferror)(FILE *__stream ) ;
# 832 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) clearerr_unlocked)(FILE *__stream ) ;
# 833 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) feof_unlocked)(FILE *__stream ) ;
# 834 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) ferror_unlocked)(FILE *__stream ) ;
# 843 "/usr/include/stdio.h"
extern void perror(char const *__s ) ;
# 27 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h"
extern int sys_nerr ;
# 28 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h"
extern char const * const sys_errlist[] ;
# 855 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) fileno)(FILE *__stream ) ;
# 860 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) fileno_unlocked)(FILE *__stream ) ;
# 870 "/usr/include/stdio.h"
extern FILE *popen(char const *__command , char const *__modes ) ;
# 876 "/usr/include/stdio.h"
extern int pclose(FILE *__stream ) ;
# 882 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) char *( __attribute__((__leaf__)) ctermid)(char *__s ) ;
# 910 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) flockfile)(FILE *__stream ) ;
# 914 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) int ( __attribute__((__leaf__)) ftrylockfile)(FILE *__stream ) ;
# 917 "/usr/include/stdio.h"
extern __attribute__((__nothrow__)) void ( __attribute__((__leaf__)) funlockfile)(FILE *__stream ) ;
# 4 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern struct ProtocolDesc *ocCurrentProto(void) ;
# 7 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivBool(OblivInputs *spec , __obliv_c__bool *dest , _Bool v ) ;
# 8 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivChar(OblivInputs *spec , __obliv_c__char *dest , char v ) ;
# 9 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivInt(OblivInputs *spec , __obliv_c__int *dest , int v ) ;
# 10 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivShort(OblivInputs *spec , __obliv_c__short *dest , short v ) ;
# 11 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivLong(OblivInputs *spec , __obliv_c__long *dest , long v ) ;
# 12 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void setupOblivLLong(OblivInputs *spec , __obliv_c__lLong *dest , long long v ) ;
# 14 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern void feedOblivInputs(OblivInputs *spec , size_t count , int party ) ;
# 17 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__bool feedOblivBool(_Bool v , int party ) ;
# 18 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__char feedOblivChar(char v , int party ) ;
# 19 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__short feedOblivShort(short v , int party ) ;
# 20 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__int feedOblivInt(int v , int party ) ;
# 21 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__long feedOblivLong(long v , int party ) ;
# 22 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern __obliv_c__lLong feedOblivLLong(long long v , int party ) ;
# 27 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivBool(_Bool *dest , __obliv_c__bool src , int party ) ;
# 28 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivChar(char *dest , __obliv_c__char src , int party ) ;
# 29 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivInt(int *dest , __obliv_c__int src , int party ) ;
# 30 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivShort(short *dest , __obliv_c__short src , int party ) ;
# 31 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivLong(long *dest , __obliv_c__long src , int party ) ;
# 32 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool revealOblivLLong(long long *dest , __obliv_c__lLong src , int party ) ;
# 34 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern _Bool ocBroadcastBool(int source , _Bool v ) ;
# 35 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern char ocBroadcastChar(int source , char v ) ;
# 36 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern int ocBroadcastInt(int source , int v ) ;
# 37 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern short ocBroadcastShort(int source , short v ) ;
# 38 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern long ocBroadcastLong(int source , long v ) ;
# 39 "/home/student/obliv-c/src/ext/oblivc/obliv.oh"
extern long long ocBroadcastLLong(int source , long long v ) ;
# 15 "./mutual.h"
extern char const *mySide() ;
# 17 "./mutual.h"
void mutualFriends(void *args ) ;
# 9 "mutual.oc"
obool oblivStrCmp(__obliv_c__bool const *__obliv_c__en , __obliv_c__char *s1 , __obliv_c__char *s2 )
{
  __obliv_c__bool afternull ;
  int i ;
  obool ob ;
  __obliv_c__char c ;
  int __cil_tmp7 ;
  int __cil_tmp8 ;
  __obliv_c__char *__cil_tmp9 ;
  __obliv_c__char __cil_tmp10 ;
  __obliv_c__char *__cil_tmp11 ;
  __obliv_c__char __cil_tmp12 ;
  __obliv_c__int __cil_tmp13 ;
  __obliv_c__int __cil_tmp14 ;
  __obliv_c__int __cil_tmp15 ;
  __obliv_c__int __cil_tmp16 ;
  int __cil_tmp17 ;
  int __cil_tmp18 ;
  int __cil_tmp19 ;
  __obliv_c__bool __obliv_c__cond21 ;
  __obliv_c__bool __obliv_c__cond22 ;
  __obliv_c__bool __obliv_c__cond23 ;
  __obliv_c__bool __obliv_c__cond24 ;
  __obliv_c__bool __obliv_c__cond25 ;
  __obliv_c__bool __obliv_c__cond26 ;
  __obliv_c__bool __obliv_c__cond27 ;
  __obliv_c__bool __obliv_c__cond28 ;
  __obliv_c__bool __obliv_c__cond29 ;

  {
# 49 "mutual.oc"
  memset((void *)(& afternull), 0, sizeof(__obliv_c__bool ));
# 49 "mutual.oc"
  memset((void *)(& i), 0, sizeof(int ));
# 49 "mutual.oc"
  memset((void *)(& ob), 0, sizeof(obool ));
# 49 "mutual.oc"
  memset((void *)(& c), 0, sizeof(__obliv_c__char ));
# 11 "mutual.oc"
  __obliv_c__setSignedKnown(& afternull, 1UL, (widest_t )((_Bool )0));
# 13 "mutual.oc"
  __obliv_c__setSignedKnown(& ob, 1UL, (widest_t )((_Bool )1));
# 15 "mutual.oc"
  i = 0;
# 15 "mutual.oc"
  while (1) {
    {
# 15 "mutual.oc"
    __cil_tmp7 = (int )10;
# 15 "mutual.oc"
    __cil_tmp8 = (int )i;
# 15 "mutual.oc"
    if (! (__cil_tmp8 < __cil_tmp7)) {
# 15 "mutual.oc"
      break;
    }
    }
    {
# 17 "mutual.oc"
    __obliv_c__cond21 = afternull;
# 17 "mutual.oc"
    __obliv_c__setBitAnd((struct OblivBit *)(& __obliv_c__cond22), (struct OblivBit const *)(& __obliv_c__cond21),
                         (struct OblivBit const *)(& *__obliv_c__en));
# 17 "mutual.oc"
    __obliv_c__setNotEqual(& __obliv_c__cond23, & __obliv_c__cond22, & *__obliv_c__en,
                           1UL);
    {

    }
    {
# 20 "mutual.oc"
    __cil_tmp9 = s1 + i;
# 20 "mutual.oc"
    __cil_tmp10 = *((__obliv_c__char *)__cil_tmp9);
# 20 "mutual.oc"
    __obliv_c__setSignExtend(& c, 8UL, & __cil_tmp10, 8UL);
    {
# 21 "mutual.oc"
    __cil_tmp11 = s2 + i;
# 21 "mutual.oc"
    __cil_tmp12 = *((__obliv_c__char *)__cil_tmp11);
# 21 "mutual.oc"
    __obliv_c__setSignExtend(& __cil_tmp13, 32UL, & __cil_tmp12, 8UL);
# 21 "mutual.oc"
    __obliv_c__setSignExtend(& __cil_tmp14, 32UL, & c, 8UL);
    {
# 21 "mutual.oc"
    __obliv_c__setNotEqual(& __obliv_c__cond24, & __cil_tmp14, & __cil_tmp13, 32UL);
# 21 "mutual.oc"
    __obliv_c__setBitAnd((struct OblivBit *)(& __obliv_c__cond25), (struct OblivBit const *)(& __obliv_c__cond24),
                         (struct OblivBit const *)(& __obliv_c__cond23));
# 21 "mutual.oc"
    __obliv_c__setNotEqual(& __obliv_c__cond26, & __obliv_c__cond25, & __obliv_c__cond23,
                           1UL);
    {
# 22 "mutual.oc"
    __obliv_c__condAssignKnown(& __obliv_c__cond25, & ob, 1UL, (widest_t )((_Bool )0));
    }
    {
    {
# 24 "mutual.oc"
    __obliv_c__setSignedKnown(& __cil_tmp15, 32UL, (widest_t )((int )0));
# 24 "mutual.oc"
    __obliv_c__setSignExtend(& __cil_tmp16, 32UL, & c, 8UL);
    {
# 24 "mutual.oc"
    __obliv_c__setEqualTo(& __obliv_c__cond27, & __cil_tmp16, & __cil_tmp15, 32UL);
# 24 "mutual.oc"
    __obliv_c__setBitAnd((struct OblivBit *)(& __obliv_c__cond28), (struct OblivBit const *)(& __obliv_c__cond27),
                         (struct OblivBit const *)(& __obliv_c__cond26));
# 24 "mutual.oc"
    __obliv_c__setNotEqual(& __obliv_c__cond29, & __obliv_c__cond28, & __obliv_c__cond26,
                           1UL);
    {
# 25 "mutual.oc"
    __obliv_c__condAssignKnown(& __obliv_c__cond28, & afternull, 1UL, (widest_t )((_Bool )1));
    }
    {

    }
    }
    }
    }
    }
    }
    }
    }
# 15 "mutual.oc"
    __cil_tmp17 = (int )1;
# 15 "mutual.oc"
    __cil_tmp18 = (int )i;
# 15 "mutual.oc"
    __cil_tmp19 = __cil_tmp18 + __cil_tmp17;
# 15 "mutual.oc"
    i = (int )__cil_tmp19;
  }
# 33 "mutual.oc"
  return (ob);
}
}
# 36 "mutual.oc"
void addString(__obliv_c__bool const *__obliv_c__en , __obliv_c__char *src , __obliv_c__char *dest )
{
  int i ;
  int __cil_tmp4 ;
  int __cil_tmp5 ;
  __obliv_c__char *__cil_tmp6 ;
  __obliv_c__char *__cil_tmp7 ;
  int __cil_tmp8 ;
  int __cil_tmp9 ;
  int __cil_tmp10 ;

  {
# 49 "mutual.oc"
  memset((void *)(& i), 0, sizeof(int ));
# 38 "mutual.oc"
  i = 0;
# 38 "mutual.oc"
  while (1) {
    {
# 38 "mutual.oc"
    __cil_tmp4 = (int )10;
# 38 "mutual.oc"
    __cil_tmp5 = (int )i;
# 38 "mutual.oc"
    if (! (__cil_tmp5 < __cil_tmp4)) {
# 38 "mutual.oc"
      break;
    }
    }
# 39 "mutual.oc"
    __cil_tmp6 = dest + i;
# 39 "mutual.oc"
    __cil_tmp7 = src + i;
# 39 "mutual.oc"
    __obliv_c__condAssign(__obliv_c__en, (__obliv_c__char *)__cil_tmp6, (__obliv_c__char *)__cil_tmp7,
                          8UL);
# 38 "mutual.oc"
    __cil_tmp8 = (int )1;
# 38 "mutual.oc"
    __cil_tmp9 = (int )i;
# 38 "mutual.oc"
    __cil_tmp10 = __cil_tmp9 + __cil_tmp8;
# 38 "mutual.oc"
    i = (int )__cil_tmp10;
  }
# 40 "mutual.oc"
  return;
}
}
# 42 "mutual.oc"
void readString(__obliv_c__char *dest , int n , char const *src , int party )
{
  OblivInputs specs[45] ;
  int i ;
  unsigned long __cil_tmp7 ;
  unsigned long __cil_tmp8 ;
  OblivInputs *__cil_tmp9 ;
  size_t __cil_tmp10 ;

  {
# 49 "mutual.oc"
  memset((void *)(& specs), 0, sizeof(OblivInputs [45]));
# 49 "mutual.oc"
  memset((void *)(& i), 0, sizeof(int ));
# 47 "mutual.oc"
  __cil_tmp7 = 0 * 24UL;
# 47 "mutual.oc"
  __cil_tmp8 = (unsigned long )(specs) + __cil_tmp7;
# 47 "mutual.oc"
  __cil_tmp9 = (OblivInputs *)__cil_tmp8;
# 47 "mutual.oc"
  __cil_tmp10 = (size_t )n;
# 47 "mutual.oc"
  feedOblivInputs(__cil_tmp9, __cil_tmp10, party);
# 48 "mutual.oc"
  return;
}
}
# 49 "mutual.oc"
void mutualFriends(void *args )
{
  protocolIO *io ;
  int size1 ;
  int size2 ;
  int i ;
  int j ;
  obool match[45] ;
  __obliv_c__char friends1[45][10] ;
  __obliv_c__char friends2[45][10] ;
  __obliv_c__char commonFriends[45][10] ;
  __obliv_c__int commonSize ;
  obool tmp ;
  _Bool pubMatch[45] ;
  unsigned long __cil_tmp14 ;
  unsigned long __cil_tmp15 ;
  int __cil_tmp16 ;
  unsigned long __cil_tmp17 ;
  unsigned long __cil_tmp18 ;
  int __cil_tmp19 ;
  int __cil_tmp20 ;
  int __cil_tmp21 ;
  int __cil_tmp22 ;
  int __cil_tmp23 ;
  unsigned long __cil_tmp24 ;
  unsigned long __cil_tmp25 ;
  unsigned long __cil_tmp26 ;
  unsigned long __cil_tmp27 ;
  unsigned long __cil_tmp28 ;
  unsigned long __cil_tmp29 ;
  unsigned long __cil_tmp30 ;
  unsigned long __cil_tmp31 ;
  unsigned long __cil_tmp32 ;
  unsigned long __cil_tmp33 ;
  char __cil_tmp34 ;
  int __cil_tmp35 ;
  int __cil_tmp36 ;
  int __cil_tmp37 ;
  int __cil_tmp38 ;
  int __cil_tmp39 ;
  int __cil_tmp40 ;
  int __cil_tmp41 ;
  int __cil_tmp42 ;
  unsigned long __cil_tmp43 ;
  unsigned long __cil_tmp44 ;
  int __cil_tmp45 ;
  int __cil_tmp46 ;
  int __cil_tmp47 ;
  int __cil_tmp48 ;
  int __cil_tmp49 ;
  int __cil_tmp50 ;
  int __cil_tmp51 ;
  unsigned long __cil_tmp52 ;
  unsigned long __cil_tmp53 ;
  unsigned long __cil_tmp54 ;
  unsigned long __cil_tmp55 ;
  unsigned long __cil_tmp56 ;
  unsigned long __cil_tmp57 ;
  unsigned long __cil_tmp58 ;
  unsigned long __cil_tmp59 ;
  unsigned long __cil_tmp60 ;
  unsigned long __cil_tmp61 ;
  char __cil_tmp62 ;
  int __cil_tmp63 ;
  int __cil_tmp64 ;
  int __cil_tmp65 ;
  int __cil_tmp66 ;
  int __cil_tmp67 ;
  int __cil_tmp68 ;
  int __cil_tmp69 ;
  int __cil_tmp70 ;
  int __cil_tmp71 ;
  int __cil_tmp72 ;
  __obliv_c__int __cil_tmp73 ;
  unsigned long __cil_tmp74 ;
  unsigned long __cil_tmp75 ;
  obool __cil_tmp76 ;
  __obliv_c__int __cil_tmp77 ;
  unsigned long __cil_tmp78 ;
  unsigned long __cil_tmp79 ;
  unsigned long __cil_tmp80 ;
  unsigned long __cil_tmp81 ;
  __obliv_c__char *__cil_tmp82 ;
  unsigned long __cil_tmp83 ;
  unsigned long __cil_tmp84 ;
  unsigned long __cil_tmp85 ;
  unsigned long __cil_tmp86 ;
  __obliv_c__char *__cil_tmp87 ;
  __obliv_c__int __cil_tmp88 ;
  __obliv_c__int __cil_tmp89 ;
  __obliv_c__int __cil_tmp90 ;
  unsigned long __cil_tmp91 ;
  unsigned long __cil_tmp92 ;
  unsigned long __cil_tmp93 ;
  unsigned long __cil_tmp94 ;
  __obliv_c__char *__cil_tmp95 ;
  unsigned long __cil_tmp96 ;
  unsigned long __cil_tmp97 ;
  unsigned long __cil_tmp98 ;
  unsigned long __cil_tmp99 ;
  __obliv_c__char *__cil_tmp100 ;
  int __cil_tmp101 ;
  int __cil_tmp102 ;
  int __cil_tmp103 ;
  int __cil_tmp104 ;
  int __cil_tmp105 ;
  int __cil_tmp106 ;
  int __cil_tmp107 ;
  int __cil_tmp108 ;
  int __cil_tmp109 ;
  int __cil_tmp110 ;
  int __cil_tmp111 ;
  unsigned long __cil_tmp112 ;
  unsigned long __cil_tmp113 ;
  _Bool __cil_tmp114 ;
  int __cil_tmp115 ;
  int __cil_tmp116 ;
  int __cil_tmp117 ;
  int __cil_tmp118 ;
  int __cil_tmp119 ;
  int __cil_tmp120 ;
  int __cil_tmp121 ;
  int __cil_tmp122 ;
  int __cil_tmp123 ;
  int __cil_tmp124 ;
  unsigned long __cil_tmp125 ;
  unsigned long __cil_tmp126 ;
  int *__cil_tmp127 ;
  int __cil_tmp128 ;
  int __cil_tmp129 ;
  int __cil_tmp130 ;
  int __cil_tmp131 ;
  unsigned long __cil_tmp132 ;
  unsigned long __cil_tmp133 ;
  unsigned long __cil_tmp134 ;
  unsigned long __cil_tmp135 ;
  unsigned long __cil_tmp136 ;
  unsigned long __cil_tmp137 ;
  char *__cil_tmp138 ;
  unsigned long __cil_tmp139 ;
  unsigned long __cil_tmp140 ;
  unsigned long __cil_tmp141 ;
  unsigned long __cil_tmp142 ;
  __obliv_c__char __cil_tmp143 ;
  int __cil_tmp144 ;
  int __cil_tmp145 ;
  int __cil_tmp146 ;
  int __cil_tmp147 ;
  int __cil_tmp148 ;
  int __cil_tmp149 ;
  __obliv_c__bool __obliv_c__cond150 ;
  __obliv_c__bool __obliv_c__cond151 ;
  __obliv_c__bool __obliv_c__cond152 ;
  obool __cil_tmp153 ;
  __obliv_c__bool __obliv_c__cond154 ;
  __obliv_c__bool __obliv_c__cond155 ;
  __obliv_c__bool __obliv_c__cond156 ;
  __obliv_c__int __cil_tmp157 ;

  {
# 49 "mutual.oc"
  memset((void *)(& io), 0, sizeof(protocolIO *));
# 49 "mutual.oc"
  memset((void *)(& size1), 0, sizeof(int ));
# 49 "mutual.oc"
  memset((void *)(& size2), 0, sizeof(int ));
# 49 "mutual.oc"
  memset((void *)(& i), 0, sizeof(int ));
# 49 "mutual.oc"
  memset((void *)(& j), 0, sizeof(int ));
# 49 "mutual.oc"
  memset((void *)(& match), 0, sizeof(obool [45]));
# 49 "mutual.oc"
  memset((void *)(& friends1), 0, sizeof(__obliv_c__char [45][10]));
# 49 "mutual.oc"
  memset((void *)(& friends2), 0, sizeof(__obliv_c__char [45][10]));
# 49 "mutual.oc"
  memset((void *)(& commonFriends), 0, sizeof(__obliv_c__char [45][10]));
# 49 "mutual.oc"
  memset((void *)(& commonSize), 0, sizeof(__obliv_c__int ));
# 49 "mutual.oc"
  memset((void *)(& tmp), 0, sizeof(obool ));
# 49 "mutual.oc"
  memset((void *)(& pubMatch), 0, sizeof(_Bool [45]));
# 50 "mutual.oc"
  io = (protocolIO *)args;
# 58 "mutual.oc"
  __cil_tmp14 = (unsigned long )io;
# 58 "mutual.oc"
  __cil_tmp15 = __cil_tmp14 + 452;
# 58 "mutual.oc"
  __cil_tmp16 = *((int *)__cil_tmp15);
# 58 "mutual.oc"
  size1 = ocBroadcastInt(1, __cil_tmp16);
# 59 "mutual.oc"
  __cil_tmp17 = (unsigned long )io;
# 59 "mutual.oc"
  __cil_tmp18 = __cil_tmp17 + 452;
# 59 "mutual.oc"
  __cil_tmp19 = *((int *)__cil_tmp18);
# 59 "mutual.oc"
  size2 = ocBroadcastInt(2, __cil_tmp19);
# 60 "mutual.oc"
  i = 0;
# 60 "mutual.oc"
  while (1) {
    {
# 60 "mutual.oc"
    __cil_tmp20 = (int )size1;
# 60 "mutual.oc"
    __cil_tmp21 = (int )i;
# 60 "mutual.oc"
    if (! (__cil_tmp21 < __cil_tmp20)) {
# 60 "mutual.oc"
      break;
    }
    }
# 61 "mutual.oc"
    j = 0;
# 61 "mutual.oc"
    while (1) {
      {
# 61 "mutual.oc"
      __cil_tmp22 = (int )10;
# 61 "mutual.oc"
      __cil_tmp23 = (int )j;
# 61 "mutual.oc"
      if (! (__cil_tmp23 < __cil_tmp22)) {
# 61 "mutual.oc"
        break;
      }
      }
# 62 "mutual.oc"
      __cil_tmp24 = j * 104UL;
# 62 "mutual.oc"
      __cil_tmp25 = i * 1040UL;
# 62 "mutual.oc"
      __cil_tmp26 = __cil_tmp25 + __cil_tmp24;
# 62 "mutual.oc"
      __cil_tmp27 = (unsigned long )(friends1) + __cil_tmp26;
# 62 "mutual.oc"
      __cil_tmp28 = j * 1UL;
# 62 "mutual.oc"
      __cil_tmp29 = i * 10UL;
# 62 "mutual.oc"
      __cil_tmp30 = __cil_tmp29 + __cil_tmp28;
# 62 "mutual.oc"
      __cil_tmp31 = 0 + __cil_tmp30;
# 62 "mutual.oc"
      __cil_tmp32 = (unsigned long )io;
# 62 "mutual.oc"
      __cil_tmp33 = __cil_tmp32 + __cil_tmp31;
# 62 "mutual.oc"
      __cil_tmp34 = *((char *)__cil_tmp33);
# 62 "mutual.oc"
      *((__obliv_c__char *)__cil_tmp27) = feedOblivChar(__cil_tmp34, 1);
# 61 "mutual.oc"
      __cil_tmp35 = (int )1;
# 61 "mutual.oc"
      __cil_tmp36 = (int )j;
# 61 "mutual.oc"
      __cil_tmp37 = __cil_tmp36 + __cil_tmp35;
# 61 "mutual.oc"
      j = (int )__cil_tmp37;
    }
# 60 "mutual.oc"
    __cil_tmp38 = (int )1;
# 60 "mutual.oc"
    __cil_tmp39 = (int )i;
# 60 "mutual.oc"
    __cil_tmp40 = __cil_tmp39 + __cil_tmp38;
# 60 "mutual.oc"
    i = (int )__cil_tmp40;
  }
# 65 "mutual.oc"
  i = 0;
# 65 "mutual.oc"
  while (1) {
    {
# 65 "mutual.oc"
    __cil_tmp41 = (int )45;
# 65 "mutual.oc"
    __cil_tmp42 = (int )i;
# 65 "mutual.oc"
    if (! (__cil_tmp42 < __cil_tmp41)) {
# 65 "mutual.oc"
      break;
    }
    }
# 66 "mutual.oc"
    __cil_tmp43 = i * 13UL;
# 66 "mutual.oc"
    __cil_tmp44 = (unsigned long )(match) + __cil_tmp43;
# 66 "mutual.oc"
    __obliv_c__setSignedKnown(& *((obool *)__cil_tmp44), 1UL, (widest_t )((_Bool )0));
# 65 "mutual.oc"
    __cil_tmp45 = (int )1;
# 65 "mutual.oc"
    __cil_tmp46 = (int )i;
# 65 "mutual.oc"
    __cil_tmp47 = __cil_tmp46 + __cil_tmp45;
# 65 "mutual.oc"
    i = (int )__cil_tmp47;
  }
# 67 "mutual.oc"
  i = 0;
# 67 "mutual.oc"
  while (1) {
    {
# 67 "mutual.oc"
    __cil_tmp48 = (int )size2;
# 67 "mutual.oc"
    __cil_tmp49 = (int )i;
# 67 "mutual.oc"
    if (! (__cil_tmp49 < __cil_tmp48)) {
# 67 "mutual.oc"
      break;
    }
    }
# 68 "mutual.oc"
    j = 0;
# 68 "mutual.oc"
    while (1) {
      {
# 68 "mutual.oc"
      __cil_tmp50 = (int )10;
# 68 "mutual.oc"
      __cil_tmp51 = (int )j;
# 68 "mutual.oc"
      if (! (__cil_tmp51 < __cil_tmp50)) {
# 68 "mutual.oc"
        break;
      }
      }
# 69 "mutual.oc"
      __cil_tmp52 = j * 104UL;
# 69 "mutual.oc"
      __cil_tmp53 = i * 1040UL;
# 69 "mutual.oc"
      __cil_tmp54 = __cil_tmp53 + __cil_tmp52;
# 69 "mutual.oc"
      __cil_tmp55 = (unsigned long )(friends2) + __cil_tmp54;
# 69 "mutual.oc"
      __cil_tmp56 = j * 1UL;
# 69 "mutual.oc"
      __cil_tmp57 = i * 10UL;
# 69 "mutual.oc"
      __cil_tmp58 = __cil_tmp57 + __cil_tmp56;
# 69 "mutual.oc"
      __cil_tmp59 = 0 + __cil_tmp58;
# 69 "mutual.oc"
      __cil_tmp60 = (unsigned long )io;
# 69 "mutual.oc"
      __cil_tmp61 = __cil_tmp60 + __cil_tmp59;
# 69 "mutual.oc"
      __cil_tmp62 = *((char *)__cil_tmp61);
# 69 "mutual.oc"
      *((__obliv_c__char *)__cil_tmp55) = feedOblivChar(__cil_tmp62, 2);
# 68 "mutual.oc"
      __cil_tmp63 = (int )1;
# 68 "mutual.oc"
      __cil_tmp64 = (int )j;
# 68 "mutual.oc"
      __cil_tmp65 = __cil_tmp64 + __cil_tmp63;
# 68 "mutual.oc"
      j = (int )__cil_tmp65;
    }
# 67 "mutual.oc"
    __cil_tmp66 = (int )1;
# 67 "mutual.oc"
    __cil_tmp67 = (int )i;
# 67 "mutual.oc"
    __cil_tmp68 = __cil_tmp67 + __cil_tmp66;
# 67 "mutual.oc"
    i = (int )__cil_tmp68;
  }
# 71 "mutual.oc"
  i = 0;
# 71 "mutual.oc"
  while (1) {
    {
# 71 "mutual.oc"
    __cil_tmp69 = (int )size1;
# 71 "mutual.oc"
    __cil_tmp70 = (int )i;
# 71 "mutual.oc"
    if (! (__cil_tmp70 < __cil_tmp69)) {
# 71 "mutual.oc"
      break;
    }
    }
# 72 "mutual.oc"
    j = 0;
# 72 "mutual.oc"
    while (1) {
      {
# 72 "mutual.oc"
      __cil_tmp71 = (int )size2;
# 72 "mutual.oc"
      __cil_tmp72 = (int )j;
# 72 "mutual.oc"
      if (! (__cil_tmp72 < __cil_tmp71)) {
# 72 "mutual.oc"
        break;
      }
      }
      {
# 73 "mutual.oc"
      __obliv_c__setSignedKnown(& __cil_tmp73, 32UL, (widest_t )((int )0));
# 73 "mutual.oc"
      __cil_tmp74 = j * 13UL;
# 73 "mutual.oc"
      __cil_tmp75 = (unsigned long )(match) + __cil_tmp74;
# 73 "mutual.oc"
      __cil_tmp76 = *((obool *)__cil_tmp75);
# 73 "mutual.oc"
      __obliv_c__setZeroExtend(& __cil_tmp77, 32UL, & __cil_tmp76, 1UL);
      {
# 73 "mutual.oc"
      __obliv_c__setEqualTo(& __obliv_c__cond150, & __cil_tmp77, & __cil_tmp73, 32UL);
# 73 "mutual.oc"
      __obliv_c__setBitAnd((struct OblivBit *)(& __obliv_c__cond151), (struct OblivBit const *)(& __obliv_c__cond150),
                           (struct OblivBit const *)(& __obliv_c__trueCond));
# 73 "mutual.oc"
      __obliv_c__setNotEqual(& __obliv_c__cond152, & __obliv_c__cond151, & __obliv_c__trueCond,
                             1UL);
      {
# 74 "mutual.oc"
      __cil_tmp78 = 0 * 104UL;
# 74 "mutual.oc"
      __cil_tmp79 = i * 1040UL;
# 74 "mutual.oc"
      __cil_tmp80 = __cil_tmp79 + __cil_tmp78;
# 74 "mutual.oc"
      __cil_tmp81 = (unsigned long )(friends1) + __cil_tmp80;
# 74 "mutual.oc"
      __cil_tmp82 = (__obliv_c__char *)__cil_tmp81;
# 74 "mutual.oc"
      __cil_tmp83 = 0 * 104UL;
# 74 "mutual.oc"
      __cil_tmp84 = j * 1040UL;
# 74 "mutual.oc"
      __cil_tmp85 = __cil_tmp84 + __cil_tmp83;
# 74 "mutual.oc"
      __cil_tmp86 = (unsigned long )(friends2) + __cil_tmp85;
# 74 "mutual.oc"
      __cil_tmp87 = (__obliv_c__char *)__cil_tmp86;
# 74 "mutual.oc"
      __cil_tmp153 = oblivStrCmp(& __obliv_c__cond151, __cil_tmp82, __cil_tmp87);
# 74 "mutual.oc"
      tmp = __cil_tmp153;
      {
# 74 "mutual.oc"
      __obliv_c__cond154 = tmp;
# 74 "mutual.oc"
      __obliv_c__setBitAnd((struct OblivBit *)(& __obliv_c__cond155), (struct OblivBit const *)(& __obliv_c__cond154),
                           (struct OblivBit const *)(& __obliv_c__cond151));
# 74 "mutual.oc"
      __obliv_c__setNotEqual(& __obliv_c__cond156, & __obliv_c__cond155, & __obliv_c__cond151,
                             1UL);
      {
# 75 "mutual.oc"
      __obliv_c__setSignedKnown(& __cil_tmp88, 32UL, (widest_t )((int )1));
# 75 "mutual.oc"
      __obliv_c__setSignExtend(& __cil_tmp89, 32UL, & commonSize, 32UL);
# 75 "mutual.oc"
      __obliv_c__setPlainAdd(& __cil_tmp90, & __cil_tmp89, & __cil_tmp88, 32UL);
# 75 "mutual.oc"
      __obliv_c__setSignExtend(& __cil_tmp157, 32UL, & __cil_tmp90, 32UL);
# 75 "mutual.oc"
      __obliv_c__condAssign(& __obliv_c__cond155, & commonSize, & __cil_tmp157, 32UL);
# 77 "mutual.oc"
      __cil_tmp91 = 0 * 104UL;
# 77 "mutual.oc"
      __cil_tmp92 = i * 1040UL;
# 77 "mutual.oc"
      __cil_tmp93 = __cil_tmp92 + __cil_tmp91;
# 77 "mutual.oc"
      __cil_tmp94 = (unsigned long )(friends1) + __cil_tmp93;
# 77 "mutual.oc"
      __cil_tmp95 = (__obliv_c__char *)__cil_tmp94;
# 77 "mutual.oc"
      __cil_tmp96 = 0 * 104UL;
# 77 "mutual.oc"
      __cil_tmp97 = i * 1040UL;
# 77 "mutual.oc"
      __cil_tmp98 = __cil_tmp97 + __cil_tmp96;
# 77 "mutual.oc"
      __cil_tmp99 = (unsigned long )(commonFriends) + __cil_tmp98;
# 77 "mutual.oc"
      __cil_tmp100 = (__obliv_c__char *)__cil_tmp99;
# 77 "mutual.oc"
      addString(& __obliv_c__cond155, __cil_tmp95, __cil_tmp100);
      }
      {

      }
      }
      }
      {

      }
      }
      }
# 72 "mutual.oc"
      __cil_tmp101 = (int )1;
# 72 "mutual.oc"
      __cil_tmp102 = (int )j;
# 72 "mutual.oc"
      __cil_tmp103 = __cil_tmp102 + __cil_tmp101;
# 72 "mutual.oc"
      j = (int )__cil_tmp103;
    }
# 71 "mutual.oc"
    __cil_tmp104 = (int )1;
# 71 "mutual.oc"
    __cil_tmp105 = (int )i;
# 71 "mutual.oc"
    __cil_tmp106 = __cil_tmp105 + __cil_tmp104;
# 71 "mutual.oc"
    i = (int )__cil_tmp106;
  }
# 80 "mutual.oc"
  j = 0;
# 83 "mutual.oc"
  i = 0;
# 83 "mutual.oc"
  while (1) {
    {
# 83 "mutual.oc"
    __cil_tmp107 = (int )45;
# 83 "mutual.oc"
    __cil_tmp108 = (int )i;
# 83 "mutual.oc"
    if (! (__cil_tmp108 < __cil_tmp107)) {
# 83 "mutual.oc"
      break;
    }
    }
# 86 "mutual.oc"
    i = 0;
# 86 "mutual.oc"
    while (1) {
      {
# 86 "mutual.oc"
      __cil_tmp109 = (int )45;
# 86 "mutual.oc"
      __cil_tmp110 = (int )i;
# 86 "mutual.oc"
      if (! (__cil_tmp110 < __cil_tmp109)) {
# 86 "mutual.oc"
        break;
      }
      }
      {
# 87 "mutual.oc"
      __cil_tmp111 = (int )1;
# 87 "mutual.oc"
      __cil_tmp112 = i * 1UL;
# 87 "mutual.oc"
      __cil_tmp113 = (unsigned long )(pubMatch) + __cil_tmp112;
# 87 "mutual.oc"
      __cil_tmp114 = *((_Bool *)__cil_tmp113);
# 87 "mutual.oc"
      __cil_tmp115 = (int )__cil_tmp114;
# 87 "mutual.oc"
      if (__cil_tmp115 == __cil_tmp111) {
# 89 "mutual.oc"
        __cil_tmp116 = (int )1;
# 89 "mutual.oc"
        __cil_tmp117 = (int )j;
# 89 "mutual.oc"
        __cil_tmp118 = __cil_tmp117 + __cil_tmp116;
# 89 "mutual.oc"
        j = (int )__cil_tmp118;
      }
      }
# 86 "mutual.oc"
      __cil_tmp119 = (int )1;
# 86 "mutual.oc"
      __cil_tmp120 = (int )i;
# 86 "mutual.oc"
      __cil_tmp121 = __cil_tmp120 + __cil_tmp119;
# 86 "mutual.oc"
      i = (int )__cil_tmp121;
    }
# 83 "mutual.oc"
    __cil_tmp122 = (int )1;
# 83 "mutual.oc"
    __cil_tmp123 = (int )i;
# 83 "mutual.oc"
    __cil_tmp124 = __cil_tmp123 + __cil_tmp122;
# 83 "mutual.oc"
    i = (int )__cil_tmp124;
  }
# 91 "mutual.oc"
  __cil_tmp125 = (unsigned long )io;
# 91 "mutual.oc"
  __cil_tmp126 = __cil_tmp125 + 908;
# 91 "mutual.oc"
  __cil_tmp127 = (int *)__cil_tmp126;
# 91 "mutual.oc"
  revealOblivInt(__cil_tmp127, commonSize, 0);
# 92 "mutual.oc"
  i = 0;
# 92 "mutual.oc"
  while (1) {
    {
# 92 "mutual.oc"
    __cil_tmp128 = (int )45;
# 92 "mutual.oc"
    __cil_tmp129 = (int )i;
# 92 "mutual.oc"
    if (! (__cil_tmp129 < __cil_tmp128)) {
# 92 "mutual.oc"
      break;
    }
    }
# 93 "mutual.oc"
    j = 0;
# 93 "mutual.oc"
    while (1) {
      {
# 93 "mutual.oc"
      __cil_tmp130 = (int )10;
# 93 "mutual.oc"
      __cil_tmp131 = (int )j;
# 93 "mutual.oc"
      if (! (__cil_tmp131 < __cil_tmp130)) {
# 93 "mutual.oc"
        break;
      }
      }
# 94 "mutual.oc"
      __cil_tmp132 = j * 1UL;
# 94 "mutual.oc"
      __cil_tmp133 = i * 10UL;
# 94 "mutual.oc"
      __cil_tmp134 = __cil_tmp133 + __cil_tmp132;
# 94 "mutual.oc"
      __cil_tmp135 = 456 + __cil_tmp134;
# 94 "mutual.oc"
      __cil_tmp136 = (unsigned long )io;
# 94 "mutual.oc"
      __cil_tmp137 = __cil_tmp136 + __cil_tmp135;
# 94 "mutual.oc"
      __cil_tmp138 = (char *)__cil_tmp137;
# 94 "mutual.oc"
      __cil_tmp139 = j * 104UL;
# 94 "mutual.oc"
      __cil_tmp140 = i * 1040UL;
# 94 "mutual.oc"
      __cil_tmp141 = __cil_tmp140 + __cil_tmp139;
# 94 "mutual.oc"
      __cil_tmp142 = (unsigned long )(commonFriends) + __cil_tmp141;
# 94 "mutual.oc"
      __cil_tmp143 = *((__obliv_c__char *)__cil_tmp142);
# 94 "mutual.oc"
      revealOblivChar(__cil_tmp138, __cil_tmp143, 0);
# 93 "mutual.oc"
      __cil_tmp144 = (int )1;
# 93 "mutual.oc"
      __cil_tmp145 = (int )j;
# 93 "mutual.oc"
      __cil_tmp146 = __cil_tmp145 + __cil_tmp144;
# 93 "mutual.oc"
      j = (int )__cil_tmp146;
    }
# 92 "mutual.oc"
    __cil_tmp147 = (int )1;
# 92 "mutual.oc"
    __cil_tmp148 = (int )i;
# 92 "mutual.oc"
    __cil_tmp149 = __cil_tmp148 + __cil_tmp147;
# 92 "mutual.oc"
    i = (int )__cil_tmp149;
  }
# 96 "mutual.oc"
  return;
}
}
