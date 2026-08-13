#if defined(_MSVC_LANG)
static_assert(_MSVC_LANG >= 202002L, "Precept requires C++20 or later");
#else
static_assert(__cplusplus >= 202002L, "Precept requires C++20 or later");
#endif
