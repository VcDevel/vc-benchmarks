#ifndef VC_TO_STRING_H
#define VC_TO_STRING_H
#include <sstream>
#include <typelist.h>

// typeToString {{{1
template <typename T> inline std::string typeToString();
// std::array<T, N> {{{2
template <typename T, std::size_t N>
inline std::string typeToString_impl(std::array<T, N> const &)
{
    std::stringstream s;
    s << "array<" << typeToString<T>() << ", " << N << '>';
    return s.str();
}
// std::vector<T> {{{2
template <typename T>
inline std::string typeToString_impl(std::vector<T> const &)
{
    std::stringstream s;
    s << "vector<" << typeToString<T>() << '>';
    return s.str();
}
// std::integral_constant<T, N> {{{2
template <typename T, T N>
inline std::string typeToString_impl(std::integral_constant<T, N> const &)
{
    std::stringstream s;
    s << "integral_constant<" << N << '>';
    return s.str();
}
// SimdArray to string {{{2
template <typename T, std::size_t N, typename V, std::size_t M>
inline std::string typeToString_impl(Vc::SimdArray<T, N, V, M> const &)
{
    std::stringstream s;
    s << "SimdArray<" << typeToString<T>() << ", " << N << '>';
    return s.str();
}
template <typename T, std::size_t N, typename V, std::size_t M>
inline std::string typeToString_impl(Vc::SimdMaskArray<T, N, V, M> const &)
{
    std::stringstream s;
    s << "SimdMaskArray<" << typeToString<T>() << ", " << N << ", " << typeToString<V>() << '>';
    return s.str();
}
// template parameter pack to a comma separated string {{{2
template <typename T0, typename... Ts> std::string typeToString_impl(Typelist<T0, Ts...> const &)
{
    std::stringstream s;
    s << '{' << typeToString<T0>();
    auto &&x = {(s << ", " << typeToString<Ts>(), 0)...};
    if (&x == nullptr) {}  // avoid warning about unused 'x'
    s << '}';
    return s.str();
}
// Vc::<Impl>::Vector<T> to string {{{2
template <typename V>
inline std::string typeToString_impl(
    V const &, typename std::enable_if<Vc::is_simd_vector<V>::value, int>::type = 0)
{
    using T = typename V::EntryType;
    std::stringstream s;
    if (std::is_same<V, Vc::Scalar::Vector<T>>::value) {
        s << "Scalar::";
    } else if (std::is_same<V, Vc::SSE::Vector<T>>::value) {
        s << "SSE::";
    } else if (std::is_same<V, Vc::AVX::Vector<T>>::value) {
        s << "AVX::";
    } else if (std::is_same<V, Vc::MIC::Vector<T>>::value) {
        s << "MIC::";
    }
    s << "Vector<" << typeToString<T>() << '>';
    return s.str();
}
template <typename V>
inline std::string typeToString_impl(
    V const &, typename std::enable_if<Vc::is_simd_mask<V>::value, int>::type = 0)
{
    using T = typename V::EntryType;
    std::stringstream s;
    if (std::is_same<V, Vc::Scalar::Mask<T>>::value) {
        s << "Scalar::";
    } else if (std::is_same<V, Vc::SSE::Mask<T>>::value) {
        s << "SSE::";
    } else if (std::is_same<V, Vc::AVX::Mask<T>>::value) {
        s << "AVX::";
    } else if (std::is_same<V, Vc::MIC::Mask<T>>::value) {
        s << "MIC::";
    }
    s << "Mask<" << typeToString<T>() << '>';
    return s.str();
}
// generic fallback (typeid::name) {{{2
template <typename T>
inline std::string typeToString_impl(
    T const &,
    typename std::enable_if<!Vc::is_simd_vector<T>::value && !Vc::is_simd_mask<T>::value,
                            int>::type = 0)
{
    return typeid(T).name();
}
// typeToString specializations {{{2
template <typename T> inline std::string typeToString() { return typeToString_impl(T()); }
template <> inline std::string typeToString<void>() { return ""; }

template <> inline std::string typeToString<long double>() { return "long double"; }
template <> inline std::string typeToString<double>() { return "double"; }
template <> inline std::string typeToString< float>() { return " float"; }
template <> inline std::string typeToString<         long long>() { return " llong"; }
template <> inline std::string typeToString<unsigned long long>() { return "ullong"; }
template <> inline std::string typeToString<         long>() { return "  long"; }
template <> inline std::string typeToString<unsigned long>() { return " ulong"; }
template <> inline std::string typeToString<         int>() { return "   int"; }
template <> inline std::string typeToString<unsigned int>() { return "  uint"; }
template <> inline std::string typeToString<         short>() { return " short"; }
template <> inline std::string typeToString<unsigned short>() { return "ushort"; }
template <> inline std::string typeToString<         char>() { return "  char"; }
template <> inline std::string typeToString<unsigned char>() { return " uchar"; }
template <> inline std::string typeToString<  signed char>() { return " schar"; }
#endif
