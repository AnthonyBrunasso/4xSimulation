#include "entity.h"

ComponentSum::ComponentSum(void* c, void **p, e2c* m, uint32_t s, int l)
  : component(c)
  , pool(p)
  , mapping(m)
  , size(s)
  , limit(l)
{
  //printf("sizeof %d, limit of %d\n", (int)s, l);
  char* walk = (char*)c;
  for (int i = 0; i < l; ++i)
  {
    p[i] = walk;
    walk += s;
  }
}

uint32_t get(uint32_t entity, ComponentSum* cs) {
  uint32_t c = INVALID_COMPONENT;
  if (!cs) return c;
  if (entity == INVALID_ENTITY) return c;
  e2c* a;
  a = cs->mapping;
  while (a < (cs->mapping+cs->limit)) {
    if (a->entity == entity) {
      c = a->component;
      break;
    }
    ++a;
  }

  return c;
}

uint32_t create(uint32_t entity, ComponentSum* cs) {
  if (!cs) return INVALID_COMPONENT;
  if (entity == INVALID_ENTITY) return INVALID_COMPONENT;
  if (VALID_COMPONENT(get(entity, cs))) return INVALID_COMPONENT; // fail creation, already exists
  e2c* a;
  a = cs->mapping;
  while (a->entity != INVALID_ENTITY) {
    ++a;
  }
  //printf("mapping used %p of %p %doffset\n", a, cs->mapping, (int)(a-cs->mapping));

  if (a-cs->mapping >= cs->limit) return INVALID_COMPONENT; // not found

  for (int i = 0; i < cs->limit; ++i){
    if (cs->pool[i])
    {
      char* walk = (char*)cs->component;
      char* p = (char*)cs->pool[i];
      int c = (p-walk)/cs->size;
      a->entity = entity;
      a->component = c;
      cs->pool[i] = 0;
      //printf("Assigning component id %d from pool index %d\n", c, i);
      break;
    }
  }

  if (a->entity == INVALID_ENTITY ) return INVALID_COMPONENT; // not found

  return a->component;
}

uint32_t delete_c(uint32_t entity, ComponentSum* cs) {
  uint32_t c = INVALID_COMPONENT;
  if (!cs) return c;
  if (entity == INVALID_ENTITY) return c;
  for (int i = 0; i < cs->limit; ++i) {
    if (cs->mapping[i].entity == entity) {
      c = cs->mapping[i].component;
      //printf("found entity %d component %d\n", entity, c);
      cs->mapping[i].component = INVALID_COMPONENT;
      cs->mapping[i].entity = INVALID_ENTITY;
      break;
    }
  }
  if (c == INVALID_COMPONENT) return c;

  // optional (clear)
  char* free_ec = ((char*)cs->component) + (c*cs->size);
  // memset(free_ec, 0, cs->size);

  // restore to pool
  void** a = cs->pool;
  while(*a) {
    ++a;
  }
  //assert(a-cs->pool < cs->limit);
  *a = free_ec;
  //printf("%p stored %p\n", a, free_ec);

  // return component index
  return c;
}

