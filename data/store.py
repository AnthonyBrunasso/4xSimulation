import sys

game_types = {}

# This will need to be updated if unique characters are added
# to game type names that aren't valid as the key of an enum.
def enumify(s):
  return s.replace(" ","_").upper()
  
def add_gametype(type_name, object_name, type_id):
  if type_id == 0:
    sys.stderr.write("0 is an invalid type_id for " + object_name + ", it is reserved for UNKNOWN.\n")
    sys.exit()

  if type_name not in game_types:
    game_types[type_name] = []

  enumified_object_name = enumify(object_name)
  game_types[type_name].append([object_name, type_id, enumified_object_name])
  return enumified_object_name
