if (MSVC)
  set(SRC ${SRC} src/mirage_base/synchronize/lock_impl_msvc.cpp)
else ()
  set(SRC ${SRC} src/mirage_base/synchronize/lock_impl_posix.cpp)
endif ()

set(SRC ${SRC}
    src/mirage_base/synchronize/lock.cpp
    PARENT_SCOPE)
