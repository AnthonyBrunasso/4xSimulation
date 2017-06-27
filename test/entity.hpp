
#pragma once

// Returns the address of the component mapped to entity "id"
//   or nullptr if not found.
template <typename T>
inline T* GetEntity(uint32_t id)
{
  static_assert("Use ECS_CPP_COMPONENT macro to define GetEntity specialization for type.");
}

// Returns the address of the created entity
//   or nullptr if full.
template <typename T>
inline T* CreateEntity(uint32_t id)
{
  static_assert("Use ECS_CPP_COMPONENT macro to define CreateEntity specialization for type.");
}

// Returns the address of the deleted entity
//   or nullptr if not found.
template <typename T>
T* DeleteEntity(uint32_t)
{
  static_assert("Use ECS_CPP_COMPONENT macro to define DeleteEntity specialization for type.");
}

#define ECS_CPP_COMPONENT(c_type, C_LIMIT)

//extern Unit many_Unit[8];
template <>
inline Unit* GetEntity(uint32_t id)
{
  uint32_t c = get(id, s_Unit());
  return c_Unit(c);
}

template <>
inline Unit* CreateEntity(uint32_t id)
{
  uint32_t c = create(id, s_Unit());
  return c_Unit(c);
}

template <>
inline Unit* DeleteEntity(uint32_t id)
{
  uint32_t c = delete_c(id, s_Unit());
  return c_Unit(c);
}

