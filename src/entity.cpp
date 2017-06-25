#include "entity.h"

ComponentSum::ComponentSum(void* c, int* p, e2c* m, int l)
  : component(c)
  , pool(p)
  , mapping(m)
  , limit(l)
{
  reset_ecs(*this);
}

// INVALID_ENTITY should match an e2c with INVALID_COMPONENT
// may require data initialization
uint32_t get(uint32_t entity, const ComponentSum& cs) {
  e2c* a = cs.mapping;
  while (a < (cs.mapping+cs.limit)) {
    if (a->entity == entity) {
      return a->component;
    }
    ++a;
  }

  return INVALID_COMPONENT;
}

uint32_t create(uint32_t entity, ComponentSum& cs) {
    if (entity == INVALID_ENTITY) return INVALID_COMPONENT;
    if (VALID_COMPONENT(get(entity, cs))) return INVALID_COMPONENT; // fail creation, already exists
    e2c* a;
    a = cs.mapping;
    while (a->entity != INVALID_ENTITY) {
      ++a;
    }
    //printf("mapping used %p of %p %doffset\n", a, cs->mapping, (int)(a-cs->mapping));
    if (a-cs.mapping >= cs.limit) return INVALID_COMPONENT; // not found

    int* p = cs.pool;
    while (*p == -1)
      ++p;

    if (*p < 0) return INVALID_COMPONENT;

    a->entity = entity;
    a->component = *p;
    *p = -1;
    //printf("Assigning component id %d from pool index %d\n", c, i);

    return a->component;
  }

uint32_t delete_c(uint32_t entity, ComponentSum& cs) {
    if (entity == INVALID_ENTITY) return INVALID_COMPONENT;
    //printf("delete called on %d", entity);
    e2c* a = cs.mapping;
    e2c* end = cs.mapping+cs.limit;
    uint32_t c = INVALID_COMPONENT;
    while (a < (end)) {
      if (a->entity == entity) {
        c = a->component;
        //printf("found entity %d component %d\n", entity, c);
        a->component = INVALID_COMPONENT;
        a->entity = INVALID_ENTITY;
        break;
      }
      ++a;
    }
    if (a >= end) return INVALID_COMPONENT;
    
    // restore to pool
    int* p = cs.pool;
    while(*p >= 0) {
      ++p;
    }
    *p = c;
    return c;
}

void reset_ecs(ComponentSum& cs) {
  //printf("limit of %d\n", l);
  // pool has l+1 elements
  for (int i = 0; i < cs.limit; ++i)
  {
    cs.pool[i] = i;
  }
  // marker for the end-of-pool
  cs.pool[cs.limit] = -2;

  // mapping has l+1 elements
  e2c* a = cs.mapping;
  while (a <= (cs.mapping+cs.limit)) {
    a->entity = INVALID_ENTITY;
    a->component = INVALID_COMPONENT;
    ++a;
  }
}
