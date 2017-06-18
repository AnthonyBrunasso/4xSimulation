#include <stdio.h>
#include <stdint.h>

struct Unit {
  uint32_t m_life;
};

struct City {
  uint32_t m_size;
};

struct Tile {
  uint32_t m_terrain;
};

struct Test {
};

// maps entity id to component id
struct e2c {
  uint32_t entity;
  uint32_t component;
}; 

struct ComponentSum {
  ComponentSum(void* c, void **p, e2c* m, uint32_t s) 
    : component(c)
    , pool(p)
    , mapping(m)
    , size(s)
  {}

  void *component;
  void **pool;
  e2c* mapping;
  uint32_t size;
};

#define INVALID_COMPONENT 0xffffffff
#define VALID_COMPONENT(x) (x!=INVALID_COMPONENT)

#define INVALID_ENTITY 0
#define VALID_ENTITY(x) (x!=INVALID_ENTITY)

#define ECS_COMPONENT(c_type, C_LIMIT) \
c_type many_##c_type[C_LIMIT]; \
void* pool_##c_type[C_LIMIT]; \
struct e2c mapping_##c_type[C_LIMIT+1]; \
ComponentSum s_##c_type(many_##c_type, pool_##c_type, mapping_##c_type, C_LIMIT)

ECS_COMPONENT(Test, 64);

#define MAX_SIZE 16
#define ECS_CREATE
ECS_COMPONENT(City, 4);
ECS_COMPONENT(Unit, 8);
ECS_COMPONENT(Tile, 16);

void init() {
  for (int i = 0; i < MAX_SIZE; ++i)
  {
    pool_Unit[i] = &many_Unit[i];
  }
  for (int i = 0; i < MAX_SIZE; ++i)
  {
    pool_City[i] = &many_City[i];
  }
  for (int i = 0; i < MAX_SIZE; ++i)
  {
    pool_Tile[i] = &many_Tile[i];
  }
}

//uint32_t create(uint32_t entity, e2c* mapping, void** avail) {
uint32_t create(uint32_t entity, ComponentSum* cs) {
  if (entity == INVALID_ENTITY) return INVALID_COMPONENT;
  e2c* a;
  a = cs->mapping;
  while (a->entity != INVALID_ENTITY) {
    ++a;
  }

  if (a-cs->mapping >= MAX_SIZE) return INVALID_COMPONENT; // not found

  for (int i = 0; i <MAX_SIZE; ++i){
    if (cs->pool[i])
    {
      cs->pool[i] = 0;
      a->entity = entity;
      a->component = i;
      break;
    }
  }

  if (a->entity == INVALID_ENTITY ) return INVALID_COMPONENT; // not found

  return a->component;
}

int main() {
  init();

  for (int i = 0; i <= 10; ++i) {
    uint32_t id = create(i, &s_Unit);// mapping_Unit, pool_Unit);
    Unit* u = VALID_COMPONENT(id)?&many_Unit[id]:0;
    printf("%d unit address %p\n", i, u);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, &s_City);//mapping_City, pool_City);
    City* c= VALID_COMPONENT(id)?&many_City[id]:0;
    printf("%d unit address %p\n", i, c);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, &s_Tile);//mapping_Tile, pool_Tile);
    Tile* t= VALID_COMPONENT(id)?&many_Tile[id]:0;
    printf("%d unit address %p\n", i, t);
  }

  return 0;
}
