#include <cstddef>
#include <cstring>
#include <cstdio>
#include <stdint.h>
//#include <vector>

struct Unit
{
  static const uint32_t POOL_SIZE = 164;
  uint32_t life;
};

struct City
{
  static const uint32_t POOL_SIZE = 164;
  uint32_t size;
};

struct Tile
{
  static const uint32_t POOL_SIZE = 164;
  uint32_t terrain;
};

uint32_t s_next_id = 1;
//std::vector<Unit> s_unit; //6k of obj code
//std::vector<City> s_city; //6k of obj code
//std::vector<Tile> s_tile; //6k of obj code

//Unit* s_units;

template <typename T>
T* GetAs()
{
  static T memory[1];
  return (T*)memory;
}

template <typename T>
T (&get_block())[T::POOL_SIZE] {
  static T data[T::POOL_SIZE];

  return data;
}

template <typename T>
bool (&get_used())[T::POOL_SIZE] {
  static bool data[T::POOL_SIZE];
  return data;
}

struct EntityToComponent {
  uint32_t entity_id = 0;
  void* ptr = nullptr;
};

template <typename T>
EntityToComponent (&get_mapping())[T::POOL_SIZE+1] {
  static EntityToComponent mapping[T::POOL_SIZE+1];

  return mapping;
}

template <typename T>
T* get(uint32_t id) {
  for (auto etc : get_mapping<T>()) {
    if (etc.entity_id == id) {
      return (T*)etc.ptr;
    }
  }

  return nullptr;
}

template <typename T>
T* create(uint32_t id) {
  if (id == 0) return nullptr;
  T* exists = get<T>(id);
  if (exists) return nullptr; // failure to create

  EntityToComponent* const mapping = get_mapping<T>();
  EntityToComponent* etc_found = mapping;
  while (etc_found->entity_id != 0) {
    etc_found = etc_found+1;
  }
  if (etc_found - mapping >= T::POOL_SIZE) return nullptr; //failure to create, full

  size_t index = 0;
  for (auto& used : get_used<T>()) {
    if (!used) {
      used = true;
      break;
    }
    ++index;
  }

  etc_found->entity_id = id;
  etc_found->ptr = (void*)&get_block<T>()[index];

  return (T*)etc_found->ptr;
}


uint32_t create_entity() {
  return s_next_id++;
}

int main()
{
  for (int i = 0; i < 10; ++i) {
    uint32_t n = create_entity();
    //Unit* u = create<Unit>(n);
    //printf("%d unit address %p\n", n, u);
    //City* c = create<City>(n);
    //printf("%d city address %p\n", n, c);
    //Tile* t = create<Tile>(n);
    //printf("%d tile address %p\n", n, t);
  }

  Unit* u2 = GetAs<Unit>();
  printf("%p", u2);

  return 0;
}
