from cpp_ext import GetVector3i, SetVector3i, GetString, SetString, GetBool, SetBool
from type_decl import Struct, Field, TypeInfo, Ctor

bool = TypeInfo('bool', storage_type='uint32_t', storage_count=1, initial_value=Ctor('false'), getter=GetBool, setter=SetBool)
v3i = TypeInfo('vector3i', access_type="sf::Vector3i", storage_type='int', storage_count=3, initial_value='{0,0,0}', getter=GetVector3i, setter=SetVector3i)
s32 = TypeInfo('int32_t', initial_value=Ctor('-1'))
u32 = TypeInfo('uint32_t', initial_value=Ctor('0xffffffff'))
string = TypeInfo('string32', access_type="std::string", storage_type='char', storage_count=32, getter=GetString, setter=SetString)

first_struct = Struct('AllTypesTestStruct', [
(bool, 'alive'),
(v3i, 'position'),
(s32, 'count'),
(u32, 'player'),
(string, 'name'),
])

container_struct = Struct('ContainerStruct', [
(TypeInfo('AllTypesTestStruct'), 'member'),
(u32, 'extra'),
])

spawn_step = Struct('SpawnStep')
Field(spawn_step, u32, 'unit_type')
Field(spawn_step, v3i, 'location')
Field(spawn_step, u32, 'player')

Struct('ImproveStep', [
(u32, 'improvement_type'),
(v3i, 'location'),
(u32, 'resource'),
(u32, 'player'), 
])
Struct('ColonizeStep', [
(v3i, 'location'),
(u32, 'player'),
])
Struct('ConstructionStep', [
(u32, 'city_id'),
(u32, 'production_id'),
(bool, 'cheat'),
(u32, 'player'),
])
Struct('MoveStep', [
(u32, 'unit_id'),
(v3i, 'destination'),
(u32, 'player'),
(bool, 'immediate'),
(bool, 'avoid_unit'),
(bool, 'avoid_city'),
])
Struct('PurchaseStep', [
(u32, 'player'),
(u32, 'production'),
(u32, 'city'),
])
Struct('SellStep', [
(u32, 'player'),
(u32, 'city'),
(u32, 'production_id'),
])
Struct('AddPlayerStep', [
(string, 'name'),
(TypeInfo('AI_TYPE'), 'ai_type'),
])
Struct('AttackStep', [
(u32,'attacker_id'),
(u32,'defender_id'),
(u32,'player'),
])
Struct('KillStep', [
(u32, 'unit_id'),
])
Struct('UnitStatsStep', [
(u32, 'unit_id'),
(u32, 'health'),
(u32, 'attack'),
(u32, 'range'),
])
Struct('TileMutatorStep', [
(v3i, 'destination'),
(u32, 'movement_cost'),
])
Struct('ResourceMutatorStep', [
(v3i, 'destination'),
(u32, 'type'),
(u32, 'quantity'),
])
Struct('EndTurnStep', [
(u32, 'player'),
(u32, 'next_player'),
])
Struct('HarvestStep', [
(u32, 'player'),
(v3i, 'destination'),
])
Struct('SpecializeStep', [
(u32, 'city_id'),
(u32, 'terrain_type'),
(u32, 'player'),
])
Struct('BarbarianStep', [
(u32, 'player'),
])
Struct('BeginStep', [
(u32, 'active_player'),
])
Struct('CityDefenseStep', [
(u32, 'player'),
(u32, 'unit'),
])
Struct('PillageStep', [
(u32, 'player'),
(u32, 'unit'),
])
Struct('AbortStep', [
(u32, 'player'),
(u32, 'city'),
(u32, 'index'),
])
Struct('SiegeStep', [
(u32, 'player'),
(u32, 'unit'),
(u32, 'city'),
])
Struct('GrantStep', [
(u32, 'player'),
(u32, 'science'),
])
Struct('MagicStep', [
(u32, 'player'),
(v3i, 'location'),
(TypeInfo('MAGIC_TYPE'), 'type'),
(bool, 'cheat'),
])
Struct('ResearchStep', [
(u32, 'player'),
(u32, 'science'),
])
Struct('StatusStep', [
(TypeInfo('STATUS_TYPE'), 'type'),
(v3i, 'location'),
])
Struct('QuitStep', [])
