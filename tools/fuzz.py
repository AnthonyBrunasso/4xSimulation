import random

try:
  from builtins import input
  p = input('>')
except ImportError:
  p = raw_input('>')

  #quit
  #join <name> [<aiType>]
helpOutput = """
Admin Commands:
  active_player <playerId>
  barbarians
  begin_turn
  kill <unitId>
  spawn <unitType> <x> <y> <z>
  stats <unitId> <health> <attack> <range>
  tile_cost <x> <y> <z> <cost>
  tile_resource <resourceType> <x> <y> <z> <quantity>
Player Commands:
  attack <attacker unitId> <defender unitId>
  colonize <x> <y> <z>
  construct <cityId> <productionType>
  discover <x> <y> <z>
  end_turn
  harvest <x> <y> <z>
  improve <improvementType> <x> <y> <z>
  move <unitId> <x> <y> <z>
  queue_move <unitId> <x> <y> <z>
  purchase <cityId> <buildingId>
  purchase <cityId> <unitId>
  sell <buildingId>
  sell <unitId>
  specialize <cityId> <terrain_type>

Queries:
  building_types
  cities
  city <cityId>
  construction_types
  definitions
  draw tile <x> <y> <z>
  help
  improvement_types
  improvements
  path_to <x> <y> <z> <tox> <toy> <toz>
  players
  range <x> <y> <z> <n>
  resource_types
  route <xs> <ys> <zs> <xt> <yt> <zt>
  terrain_types
  tile <x> <y> <z>
  tiles
  unit <unitId>
  unit_types
  units

ACII Drawing:
  draw tile <x> <y> <z>
"""

cmds = helpOutput.splitlines()
uniqueCmds = set()
maxVal = 0
for cmd in cmds:
  cmd = cmd.strip()
  if(len(cmd) == 0): continue
  maxVal = max(maxVal, len(cmd.split(' ')))
  uniqueCmds.add(cmd.split(' ')[0])

#print(maxVal)
#print(uniqueCmds)

def GenerateCharacter():
  tokens = 'abcdefhijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
  return random.choice(tokens)

def GenerateNumber():
  tokens = '12345667890'
  return random.choice(tokens)

def GenerateToken():
  length = random.randint(0, 16)
  token = [GenerateNumber() for _ in range(length)]
  return ''.join(token)

print("join a")
print("join b")

try:
  for i in range(1000):
    for cmd in uniqueCmds:
      tokenCount = random.randint(0, maxVal-1)
      tokenList = [cmd]
      tokenList += [GenerateToken() for _ in range(tokenCount)]
      inputString = ' '.join(tokenList)
      print(inputString)
except KeyboardInterrupt:
  raise Exception(inputString)

print("players")

