#ifndef UTIL_XCS_FUNCTIONAL_HPP
#define UTIL_XCS_FUNCTIONAL_HPP
/* TODO(aug): Aus Kompatibilitätsgründen für C++17 angelegt, wo diese structs (binary_function, unary_function) vom Standard
entfernt wurden (aber offensichtlich im GCC noch nicht). Seit C++11 waren sie als 'depracated' markiert!

Diese Datei unbedingt entfernen, wenn echte Lösung eingebaut!
*/
#if __cplusplus >= 201700L
namespace std {
  template<class Arg1, class Arg2, class Result>
  struct binary_function {
    using first_argument_type = Arg1;
    using second_argument_type = Arg2;
    using result_type = Result;
  };

  template <typename ArgumentType, typename ResultType>
  struct unary_function {
    using argument_type = ArgumentType;
    using result_type = ResultType;
  };
}

#endif
#endif  // UTIL_XCS_FUNCTIONAL_HPP