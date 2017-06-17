#pragma once

#include "Vector3.hpp"


#include <string>
#include <cstdint>

namespace fbs
{
  enum class DIRECTION_TYPE : uint32_t;
}

// Utility to remove type safety on enum classes
//
// Usage: Create the desired type (int or enum) on the stack and assign an any_enum:
// PRODUCTION_TYPE pt = any_enum(3);
// uint32_t pt_int = any_enum(pt);
class any_enum                                                                                                
{                                                                                                             
public:                                                                                                       
  template <typename T>                                                                                       
  explicit any_enum(T f) : foo(static_cast<uint32_t>(f)) {}                                                            
  operator uint32_t() const { return (foo); }                                                                       
  // We could use this type with std algorithm, but that too leads to code bloat. 
  //template <typename T> friend bool operator==(T val, any_enum);
                                                                                                              
  // should be able to implicit convert to common types
  template <typename T>                                                                                       
    operator T() const { return T(foo); }                                                                           
  // or compare the common type to foo by cast
  template <typename T>                                                                                        
  friend bool operator==(T val, const any_enum& other);
private:
  any_enum(any_enum&);
  any_enum& operator=(any_enum&);

  uint32_t foo;                                                                                               
};

template  <typename T>
bool operator==(T val, const any_enum& other) {
    return val == (T)other.foo;
}

namespace util {
  sf::Vector3i str_to_vector3(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f str_to_vector3f(const std::string& x, const std::string& y, const std::string& z);
  sf::Vector3f to_vector3f(const sf::Vector3i& vector);
  fbs::DIRECTION_TYPE get_direction(const sf::Vector3i& diff);
  sf::Vector3i get_direction(fbs::DIRECTION_TYPE type);

  size_t enum_from_names(const std::string& searchName, const char** names);
}
