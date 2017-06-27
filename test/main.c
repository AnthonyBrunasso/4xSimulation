#include <stdio.h>
#include <stdint.h>
#include <string.h>
//#include <assert.h>

#include "entity.h"

struct Unit {
  uint32_t m_life;
};

struct City {
  uint32_t m_size;
  uint32_t m_queue;
  uint32_t m_owner;
};

ECS_COMPONENT(City, 4);
ECS_COMPONENT(Unit, 8);
namespace Testing { 
  struct Tile {
    uint32_t m_terrain;
    uint32_t m_movement;
  };

  ECS_COMPONENT(Tile, 16);
}

#include "entity.hpp"

int main() {
  // create all
  for (int i = 0; i <= 10; ++i) {
    Unit* u = CreateEntity<Unit>(i);
    printf("%d unit address %p\n", i, u);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, s_City());//mapping_City, pool_City);
    uint32_t get_id = get(i, s_City());
    printf("create %d, get %d", id, get_id);
    //City* c= VALID_COMPONENT(id)?&many_City[id]:0;
    City* c = c_City(id);
    printf("%d city address %p\n", i, c);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, Testing::s_Tile());//mapping_Tile, pool_Tile);
    uint32_t get_id = get(i, Testing::s_Tile());
    printf("create %d, get %d", id, get_id);
    //Tile* t= VALID_COMPONENT(id)?&many_Tile[id]:0;
    Testing::Tile* t = Testing::c_Tile(id);
    printf("%d tile address %p\n", i, t);
  }

  printf("press enter\n");
  getchar();
  // delete all in reverse order
  for (int i = 10; i >= 0; --i) {
    Unit* u = DeleteEntity<Unit>(i);
    printf("%d unit address %p\n", i, u);
  }
  for (int i = 10; i >= 0; --i) {
    uint32_t id= delete_c(i, s_City());//mapping_City, pool_City);
    City* c= VALID_COMPONENT(id)?&many_City[id]:0;
    printf("%d city address %p\n", i, c);
  }
  for (int i = 10; i >= 0; --i) {
    uint32_t id= delete_c(i, Testing::s_Tile());//mapping_Tile, pool_Tile);
    Testing::Tile* t= VALID_COMPONENT(id)?&Testing::many_Tile[id]:0;
    printf("%d tile address %p\n", i, t);
  }

  printf("press enter\n");
  getchar();
  // recreate
  for (int i = 0; i <= 10; ++i) {
    Unit* u = CreateEntity<Unit>(i);
    printf("%d unit address %p\n", i, u);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, s_City());//mapping_City, pool_City);
    City* c= VALID_COMPONENT(id)?&many_City[id]:0;
    printf("%d city address %p\n", i, c);
  }
  for (int i = 0; i <= 10; ++i) {
    uint32_t id= create(i, Testing::s_Tile());//mapping_Tile, pool_Tile);
    Testing::Tile* t= VALID_COMPONENT(id)?&Testing::many_Tile[id]:0;
    printf("%d tile address %p\n", i, t);
  }

  uint32_t c;
  printf("press enter\n");
  // delete an invalid entity
  c = delete_c(5, s_City());
  printf("invalid delete, %d\n", c);
  // delete a valid entity
  c = delete_c(1, s_City());
  printf("valid delete %d\n", c);
  // delete 2nd valid 
  c = delete_c(4, s_City());
  printf("2nd valid delete %d\n", c);

  // duplicate valid entity
  c = create(2, s_City());
  printf("duplicate entity %d\n", c);
  c = create(3, s_City());
  printf("duplicate entity %d\n", c);
  c = create(4, s_City());
  printf("recycled entity %d\n", c);
  c = create(10, s_City());
  printf("new entity %d\n", c);

  // prefab?
  // set operations
  // c2e functions?
  
  return 0;
}
