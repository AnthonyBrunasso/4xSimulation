game_types = {}

def add_gametype(type_name, object_name, type_id):
  if type_name not in game_types:
    game_types[type_name] = []

  game_types[type_name].append([object_name, type_id])
