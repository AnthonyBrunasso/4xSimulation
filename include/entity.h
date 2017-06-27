#include <stdint.h>

#define INVALID_ENTITY 0
#define VALID_ENTITY(x) (x!=INVALID_ENTITY)

#define INVALID_COMPONENT 0x7fffffff
#define VALID_COMPONENT(x) (x!=INVALID_COMPONENT)

// Internal structure
//
// e2c provides a mapping of entity id to component id.
struct e2c {
  uint32_t entity;
  uint32_t component;
}; 

// The C-API requires this object when accessing components.
// One ComponentSum instance will exist for each Component.
// The singleton is named s_<type> where 
//   <type> is declared in the ECS_COMPONENT macro that follows.
// 
// ComponentSum holds all component instances and mappings to 
//   valid entity ids.
struct ComponentSum {
  ComponentSum(void* c, int *p, e2c* m, int l);

  void *component;
  int *pool;
  e2c* mapping;
  int limit;
};

// Test c++ API in the ecs_merge branch
//
// todo: does it make sense to compile with a C compiler?
// TODO: Allow a function for look-up by component id.
//  This should still check the e2c mapping to verify validity.
//  Science and Production have an assumption that enum can be mapped
//  into a meaningful object.
// TODO: Document batch operations 
// TODO: API to count created components
//
// example: 
// class MyComponent {};
// ECS_COMPONENT(MyComponent, 64);
//
#define ECS_COMPONENT(c_type, C_LIMIT) \
c_type many_##c_type[C_LIMIT]; \
int pool_##c_type[C_LIMIT+1]; \
struct e2c mapping_##c_type[C_LIMIT+1]; \
ComponentSum& s_##c_type() { static ComponentSum s_##c_type(many_##c_type, pool_##c_type, mapping_##c_type, C_LIMIT); return s_##c_type; }; \
c_type* c_##c_type(int c) { return VALID_COMPONENT(c)?&many_##c_type[c]:0; }

// entity: 0 is invalid, non-zero is safe
// cs: reference to the component singleton
//
// returns: integer in the valid component range
// OR INVALID_COMPONENT when:
//   unable to find the entity
uint32_t get(uint32_t entity, const ComponentSum& cs);

// entity: 0 is invalid, non-zero is safe
// cs: reference to the component singleton
// 
// returns: integer in the valid component range
// OR INVALID_COMPONENT when:
//   duplicate entity id is created
//   no room is left in the fixed size component array
uint32_t create(uint32_t entity, ComponentSum& cs);

// entity: 0 is invalid, non-zero is safe
// cs: reference to the component singleton
// 
// returns: integer in the valid component range on success
// OR INVALID_COMPONENT when:
//   entity is invalid
//   entity is not found 
uint32_t delete_c(uint32_t entity, ComponentSum& cs);

// cs: reference to the component singleton
//
// reset_ecs will initialize components of a type to the 
// default state. This is equivalent to calling 
// delete_c on all entities. It is called during global
// static initialization by default.
void reset_ecs(ComponentSum& cs);

