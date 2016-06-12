import sys

game_types = {}

def add_gametype(type_name, object_name, type_id):
  if type_id == 0:
    sys.stderr.write("0 is an invalid type_id for " + object_name + ", it is reserved for UNKNOWN.\n")
    sys.exit()

  if type_name not in game_types:
    game_types[type_name] = []

  game_types[type_name].append([object_name, type_id])
