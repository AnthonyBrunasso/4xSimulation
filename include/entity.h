#include <stdint.h>

// entity header?
#define INVALID_ENTITY 0
#define VALID_ENTITY(x) (x!=INVALID_ENTITY)

#define INVALID_COMPONENT 0x7fffffff
#define VALID_COMPONENT(x) (x!=INVALID_COMPONENT)

// maps entity id to component id
// TODO: Clear both fields on deletion
struct e2c {
  uint32_t entity;
  uint32_t component;
}; 

// all component instances and mappings to their entities
struct ComponentSum {
  ComponentSum(void* c, int *p, e2c* m, int l);

  void *component;
  int *pool;
  e2c* mapping;
  int limit;
};

// do inlined templates add a lot of bloat? Do they improve the api?
// todo: support ecs_component macro inside a namespace
// todo: does it make sense to compile with a C compiler?
// clearing e2c entity and component on deletion seems preferable.
// simplifies iteration, and allows for look-ups by component id.
// TODO: Allow a function for look-up by component id.
//  This should still check the e2c mapping to verify validity.
//  Science and Production have an assumption that enum can be mapped
//  into a meaningful object.
// TODO: Document how to get the ComponentSum, the Component
// TODO: Document batch operations (for each...)
// TODO: Document batch deletion** (2x)
// TODO: Document batch count
// TODO: Avail/alive pointer list. Make it compact and null terminated.
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
// cs: pointer to the component information
//
// returns: integer in the valid component range
// OR INVALID_COMPONENT when:
//   unable to find the entity
uint32_t get(uint32_t entity, const ComponentSum& cs);

// entity: 0 is invalid, non-zero is safe
// cs: pointer to the component information
// 
// returns: integer in the valid component range
// OR INVALID_COMPONENT when:
//   duplicate entity id is created
//   no room is left in the fixed size component array
uint32_t create(uint32_t entity, ComponentSum& cs);

// entity: 0 is invalid, non-zero is safe
// cs: pointer to the component information
// 
// returns: integer in the valid component range on success
// OR INVALID_COMPONENT when:
//   entity is invalid
//   entity is not found 
uint32_t delete_c(uint32_t entity, ComponentSum& cs);

// reset to initial state
void reset_ecs(ComponentSum& cs);

