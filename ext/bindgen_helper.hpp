/* This file is part of bindgen
 *   See: https://github.com/Papierkorb/bindgen
 *
 * This file is licensed under the following "public domain" license.
 * IT APPLIES ONLY TO THIS FILE `bindgen_helper.h` AND NOT TO ANY OTHER FILE.
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

/*********************** TAKE CARE WHEN CHANGING THINGS ***********************
 * This file may be a symlink, pointing at the `bindgen_helper.hpp` inside the *
 * shard installation directory of `bindgen`.  It is *not* a local copy!   If  *
 * you want to make changes, make sure that you have your own copy first, and  *
 * only then do changes.  Otherwise a bindgen update may override your changes.*
 ******************************************************************************/

#ifndef BINDGEN_HELPER_HPP
#define BINDGEN_HELPER_HPP
#define GC_NEW_ABORTS_ON_OOM

#include <gc/gc.h> // Boehm GC
#include <string.h>
#include <stdlib.h> // abort()
#include <stdio.h>  // fprintf()
#include <QtWidgets/QtWidgets>

// Helper structure to transfer a `String` between C/C++ and Crystal.
typedef struct CrystalString { // C compatibility
  const char *ptr;
  int size;
} CrystalString;

// Compiler branching hint
#define bindgen_likely(x) __builtin_expect(!!(x), 1)

static __attribute__((noreturn)) void bindgen_fatal_panic(const char *message) {
  fprintf(stderr, "Fatal error in bindings: %s\n", message);
  abort();
}

#ifdef __cplusplus
#include <gc/gc_cpp.h>
#include <string>

// Break C++'s encapsulation to allow easy wrapping of protected methods.
#define protected public

static CrystalString bindgen_stdstring_to_crystal(const std::string &str) {
  return CrystalString{ str.data(), static_cast<int>(str.size()) };
}

static std::string bindgen_crystal_to_stdstring(CrystalString str) {
  return std::string(str.ptr, str.size);
}

/* Wrapper for a Crystal `Proc`. */
template<typename T, typename ... Args>
struct CrystalProc {
  union {
    T (*withSelf)(void *, Args ...);
    T (*withoutSelf)(Args ...);
  };

  void *self;

  CrystalProc() : withSelf(nullptr), self(nullptr) { }

  inline bool isValid() const {
    return (withSelf != nullptr);
  }

  /* Fun fact: If the Crystal `Proc` doesn't capture any context, it won't
   * allocate any - But also don't expect any!  We have to accomodate for this
   * by only passing `this->self` iff it is non-NULL.
   */

  T operator()(Args ... arguments) const {
    if (this->self) {
      return this->withSelf(this->self, arguments...);
    } else {
      return this->withoutSelf(arguments...);
    }
  }
};

/// C++ wrapper for unmanaged objects.
///
/// It basically makes that object visible to the garbage collector.
template <typename T, typename Enable = void>
struct CrystalGCWrapper: public T, public gc_cleanup
{
  using T::T;
};

/// Type trait for identifying managed and non-managed Qt objects.
template <typename T, typename Enable = void> struct bg_type;

/// Type tag for construction of wrappers from plain pointers.
struct bg_from_ptr {};

/// C++ wrapper for managed Qt objects.
///
/// The wrapper itself is garbage collected but the wrapped Qt-object is not.
/// If the wrapper is deleted it also deletes the wrapped object unless that
/// object has parent.
template <typename T>
struct CrystalGCWrapper<T, typename std::enable_if<bg_type<typename std::remove_pointer<T>::type*>::wrap>::type> : public gc_cleanup
{
  using bg_type = ::bg_type<typename std::remove_pointer<T>::type*>;

  template <typename... Args>
  CrystalGCWrapper(Args&&... args) : q(new T(std::forward<Args>(args)...)) {
  }

  /// Constructor to wrap an existing plain pointer.
  ///
  /// The `bg_from_ptr` tag is only used to ensure that the correct
  /// constructor is called (there might be another constructor of T
  /// also taking a T* argument).
  CrystalGCWrapper(T* ptr, bg_from_ptr) : q((typename std::remove_const<typename std::remove_pointer<T>::type>::type*)(ptr)) {
  }

  /// Destructor.
  ///
  /// Delete the wrapped Qt-object unless it has a parent.
  ~CrystalGCWrapper() {
    if (q != nullptr) {
      auto ptr = static_cast<T*>(&*q);
      if (!bg_type::has_parent(ptr)) {
        delete ptr;
      }
    }
  }

  /// Return a pointer to the wrapped object.
  ///
  /// Raises an error of the wrapped object has been deleted (i.e. if
  /// the pointer is null)
  T* ptr() const {
    if (q != nullptr)
      return static_cast<T*>(&*q);
    else {
      fprintf(stderr, "Fatal error: C++ object has been deleted");
      std::abort();
    }
  }

  /// Pointer to the underlying object.
  typename bg_type::pointer q;
};

/// Small "smart"-pointer around managed Qt-objects.
///
/// The purpose of the wrapper is basically to forward method access
/// to the wrapped C++ object.
template <typename T>
struct CrystalQObjectPtr {
  template <typename Q>
  CrystalQObjectPtr(CrystalGCWrapper<Q>* q, typename std::enable_if<std::is_base_of<T, Q>::value, int>::type = 0):
    q(reinterpret_cast<CrystalGCWrapper<T>*>(q))
  {

  }

  /// Create a new pointer from a plain C++ pointer.
  ///
  /// This function create a new pointer-wrapper that points to the actual C++ object.
  ///
  /// If `ptr` is `nullptr` then no wrapper is created
  /// (pointer-wrapper should never point to null-objects).
  CrystalQObjectPtr(T* ptr) : q(ptr ? new (UseGC) CrystalGCWrapper<T>(ptr, bg_from_ptr{}) : nullptr) {}

  operator T*() { return q ? q->ptr() : nullptr; }

  T* operator->() {
    if (q != nullptr) return q->ptr();
    fprintf(stderr, "Fatal error: reference null-pointer to C++ object");
    std::abort();
  }

  T& operator*() {
    if (q != nullptr) return *q->ptr();
    fprintf(stderr, "Fatal error: reference null-pointer to C++ object");
    std::abort();
  }

  CrystalGCWrapper<T>* q;
};

/// By default types are not wrapped.
template <typename T, typename Enable>
struct bg_type {
  typedef T type;
  static constexpr bool wrap = false;
};

/// Register a wrapped type T.
///
/// - T is the type (base class) to be wrapped
/// - P is the pointer to be used in the wrapper
/// - Parent is the method that returns the parent object
#define BG_WRAP(T, P, Parent)                                                  \
  class T;                                                                     \
  template <typename S>                                                        \
  struct bg_type<                                                              \
      S *, typename std::enable_if<std::is_base_of<T, S>::value>::type> {      \
    typedef CrystalQObjectPtr<S> type;                                         \
    typedef P pointer;                                                         \
    static constexpr bool wrap = true;                                         \
                                                                               \
    static bool has_parent(const S *ptr) {                                     \
      return ptr != nullptr &&                                                 \
             static_cast<const T *>(ptr)->Parent() != nullptr;                 \
    }                                                                          \
  };

/// Marks a type as non-wrapped even if it is a derivative of a wrapped class.
///
/// Mostly useful for internal classes that should actually not even be part
/// of the (visible) API.
#define BG_NO_WRAP(T)                                                          \
  class T;                                                                     \
  template <> struct bg_type<T *> {                                            \
    typedef T *type;                                                           \
    static constexpr bool wrap = false;                                        \
  };

// Wrap base classes that have some parent handling
BG_WRAP(QObject, QPointer<QObject>, parent)
BG_WRAP(QListWidgetItem, QListWidgetItem*, listWidget)

// Do not wrap private classes
BG_NO_WRAP(QTextDocumentPrivate)
BG_NO_WRAP(QTextCursorPrivate)
BG_NO_WRAP(QGraphicsSceneEventPrivate)
BG_NO_WRAP(QPostEventList)
BG_NO_WRAP(QPlatformPixmap)
BG_NO_WRAP(QPlatformSurface)
BG_NO_WRAP(QPlatformWindow)
BG_NO_WRAP(QVulkanInstance)
BG_NO_WRAP(QPlatformNativeInterface)
BG_NO_WRAP(QPlatformScreen)
BG_NO_WRAP(QPlatformMenu)
BG_NO_WRAP(QPlatformMenuBar)

// Classes without accessible destructor
BG_NO_WRAP(QInputMethod)
BG_NO_WRAP(QClipboard)


#endif // __cplusplus
#endif // BINDGEN_HELPER_HPP
