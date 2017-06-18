#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

struct Unit {
  uint32_t m_life;
};

struct City {
  uint32_t m_size;
  uint32_t m_queue;
  uint32_t m_owner;
};

struct Tile {
  uint32_t m_terrain;
  uint32_t m_movement;
};

struct Test {
};

// maps entity id to component id
struct e2c {
  uint32_t entity;
  uint32_t component;
}; 

struct ComponentSum {
  ComponentSum(void* c, void **p, e2c* m, size_t s, int l)
    : component(c)
    , pool(p)
    , mapping(m)
    , size(s)
    , limit(l)
  {
    printf("sizeof %d, limit of %d\n", (int)s, l);
    char* walk = (char*)c;
    for (int i = 0; i < l; ++i)
    {
      p[i] = walk;
      walk += s;
    }
  }

  void *component;
  void **pool;
  e2c* mapping;
  size_t size;
  int limit;
};

#define INVALID_COMPONENT 0xffffffff
#define VALID_COMPONENT(x) (x!=INVALID_COMPONENT)

#define INVALID_ENTITY 0
#define VALID_ENTITY(x) (x!=INVALID_ENTITY)

#define ECS_COMPONENT(c_type, C_LIMIT) \
c_type many_##c_type[C_LIMIT]; \
void* pool_##c_type[C_LIMIT+1]; \
struct e2c mapping_##c_type[C_LIMIT+1]; \
ComponentSum s_##c_type(many_##c_type, pool_##c_type, mapping_##c_type, sizeof(c_type), C_LIMIT)

ECS_COMPONENT(Test, 64);

#define MAX_SIZE 16
#define ECS_CREATE
ECS_COMPONENT(City, 4);
ECS_COMPONENT(Unit, 8);
ECS_COMPONENT(Tile, 16);

uint32_t get(uint32_t entity, ComponentSum* cs) {
  uint32_t c = INVALID_COMPONENT;
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
      cs->pool[i] = 0;
      a->entity = entity;
      a->component = i;
      //printf("Assigning component id %d\n", i);
      break;
    }
  }

  if (a->entity == INVALID_ENTITY ) return INVALID_COMPONENT; // not found

  return a->component;
}

uint32_t delete_c(uint32_t entity, ComponentSum* cs) {
  uint32_t c = INVALID_COMPONENT;
  if (entity == INVALID_ENTITY) return c;
  for (int i = 0; i < cs->limit; ++i) {
    if (cs->mapping[i].entity == entity) {
      c = cs->mapping[i].component;
      printf("found entity %d component %d\n", entity, c);
      cs->mapping[i].component = INVALID_COMPONENT;
      cs->mapping[i].entity = INVALID_ENTITY;
      break;
    }
  }
  if (c == INVALID_COMPONENT) return c;

  // optional (clear)
  char* free_ec = ((char*)cs->component) + (c*cs->size);
  memset(free_ec, 0, cs->size);

  // restore to pool
  void** a = cs->pool;
  while(*a) {
    ++a;
  }
  assert(a-cs->pool < cs->limit);
  *a = free_ec;
  printf("%p stored %p\n", a, free_ec);

  // return component index
  return c;
}

int main() {
  for (int i = 0; i <= 10; ++i) {
    uint32_t id = create(i, &s_Unit);// mapping_Unit, pool_Unit);
    Unit* u = VALID_COMPONENT(id)?&many_Unit[id]:0;
    printf("%d unit address %p\n", i, u);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, &s_City);//mapping_City, pool_City);
    City* c= VALID_COMPONENT(id)?&many_City[id]:0;
    printf("%d city address %p\n", i, c);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, &s_Tile);//mapping_Tile, pool_Tile);
    Tile* t= VALID_COMPONENT(id)?&many_Tile[id]:0;
    printf("%d tile address %p\n", i, t);
  }

  uint32_t c;
  // delete an invalid entity
  c = delete_c(5, &s_City);
  printf("invalid delete, %d\n", c);
  // delete a valid entity
  c = delete_c(1, &s_City);
  printf("valid delete %d\n", c);
  // delete 2nd valid 
  c = delete_c(4, &s_City);
  printf("2nd valid delete %d\n", c);

  // duplicate valid entity
  c = create(2, &s_City);
  printf("duplicate entity %d\n", c);
  c = create(3, &s_City);
  printf("duplicate entity %d\n", c);
  c = create(4, &s_City);
  printf("recycled entity %d\n", c);

  return 0;
}
