from store import add_gametype 

# NOTE: All game types will generate an UNKNOWN enum at id 0 so that
# keyword and id is reserved, (unknown, 0)

RESOURCE     = "RESOURCE"
IMPROVEMENT  = "IMPROVEMENT"
TERRAIN      = "TERRAIN"
UNIT         = "UNIT"
BUILDING     = "BUILDING"
CONSTRUCTION = "CONSTRUCTION"
AI           = "AI"
AI_ORDER     = "AI_ORDER"
TURN         = "TURN"
SEARCH       = "SEARCH"
DIRECTION    = "DIRECTION"
SCIENCE      = "SCIENCE"
MAGIC        = "MAGIC"
STATUS       = "STATUS"
COMMAND      = "COMMAND"

# Resources to generate when running type_generator.py
add_gametype(RESOURCE, 'Luxury_Gold', 1)
add_gametype(RESOURCE, 'Luxury_Sugar', 2)
add_gametype(RESOURCE, 'Strategic_Iron', 3)
add_gametype(RESOURCE, 'Strategic_Coal', 4)
add_gametype(RESOURCE, 'Cattle', 5)
add_gametype(RESOURCE, 'Deer', 6)
add_gametype(RESOURCE, 'Fish', 7)
add_gametype(RESOURCE, 'Stone', 8)

add_gametype(IMPROVEMENT, 'Mine', 1)
add_gametype(IMPROVEMENT, 'Pasture', 2)
add_gametype(IMPROVEMENT, 'Camp', 3)
add_gametype(IMPROVEMENT, 'Plantation', 4)
add_gametype(IMPROVEMENT, 'Quarry', 5)
add_gametype(IMPROVEMENT, 'Fish_Boats', 6)

add_gametype(TERRAIN, 'Desert'   , ord('d') - ord('a'))
add_gametype(TERRAIN, 'Grassland', ord('g') - ord('a'))
add_gametype(TERRAIN, 'Mountain' , ord('m') - ord('a'))
add_gametype(TERRAIN, 'Plains'   , ord('p') - ord('a'))
add_gametype(TERRAIN, 'Water'    , ord('w') - ord('a'))

add_gametype(UNIT, 'Scout'  , 1)
add_gametype(UNIT, 'Archer' , 2)
add_gametype(UNIT, 'Phalanx', 3)
add_gametype(UNIT, 'Worker' , 4)

add_gametype(BUILDING, 'Town', 1)

add_gametype(CONSTRUCTION, 'Granary', 1)
add_gametype(CONSTRUCTION, 'Range'  , 2)
add_gametype(CONSTRUCTION, 'Forge'  , 3)
add_gametype(CONSTRUCTION, 'Melee'  , 4)
add_gametype(CONSTRUCTION, 'Factory', 5)
add_gametype(CONSTRUCTION, 'Scout'  , 6)
add_gametype(CONSTRUCTION, 'Worker' , 8)

add_gametype(AI, "Barbarian", 1)
add_gametype(AI, "Human"    , 2)

add_gametype(TURN, 'TurnActive'    , 1)
add_gametype(TURN, 'TurnCompleted' , 2)

add_gametype(SEARCH, 'Units'       , 1)
add_gametype(SEARCH, 'Cities'      , 2)
add_gametype(SEARCH, 'Improvements', 3)
add_gametype(SEARCH, 'Resources'   , 4)

add_gametype(AI_ORDER, 'Attack_Unit'         , 1)
add_gametype(AI_ORDER, 'Approach_Unit'       , 2)
add_gametype(AI_ORDER, 'Attack_City'         , 3)
add_gametype(AI_ORDER, 'Approach_City'       , 4)
add_gametype(AI_ORDER, 'Pillage_Improvement' , 5)
add_gametype(AI_ORDER, 'Approach_Improvement', 6)
add_gametype(AI_ORDER, 'Wander'              , 7)

add_gametype(DIRECTION, "North_East", 1)
add_gametype(DIRECTION, "East"      , 2)
add_gametype(DIRECTION, "South_East", 3)
add_gametype(DIRECTION, "South_West", 4)
add_gametype(DIRECTION, "West"      , 5)
add_gametype(DIRECTION, "North_West", 6)

add_gametype(SCIENCE, "Agriculture", 1)
add_gametype(SCIENCE, "Pottery", 2)
add_gametype(SCIENCE, "Animal_Husbandry", 3)
add_gametype(SCIENCE, "Archery", 4)
add_gametype(SCIENCE, "Mining", 5)
add_gametype(SCIENCE, "Sailing", 6)
add_gametype(SCIENCE, "Calendar", 7)
add_gametype(SCIENCE, "Writing", 8)
add_gametype(SCIENCE, "Trapping", 9)
add_gametype(SCIENCE, "Wheel", 10)
add_gametype(SCIENCE, "Masonry", 11)
add_gametype(SCIENCE, "Bronze_Working", 12)

add_gametype(MAGIC, "Fireball"    , 1)
add_gametype(MAGIC, "Magic_Missle", 2)

add_gametype(STATUS, "Resist_Modifiers", 1);
add_gametype(STATUS, "Stasis", 2);
add_gametype(STATUS, "Constructing_Improvement", 3);

add_gametype(COMMAND, "Quit", 1)
add_gametype(COMMAND, "Begin_Turn", 2)
add_gametype(COMMAND, "End_Turn", 3)
add_gametype(COMMAND, "Attack", 4)
add_gametype(COMMAND, "Colonize", 5)
add_gametype(COMMAND, "Construct", 6)
add_gametype(COMMAND, "Discover", 7)
add_gametype(COMMAND, "Improve", 8)
add_gametype(COMMAND, "Kill", 9)
add_gametype(COMMAND, "Move", 10)
add_gametype(COMMAND, "Queue_Move", 11)
add_gametype(COMMAND, "Purchase", 12)
add_gametype(COMMAND, "Sell", 13)
add_gametype(COMMAND, "Tile_Mutator", 14)
add_gametype(COMMAND, "Resource_Mutator", 15)
add_gametype(COMMAND, "Spawn", 16)
add_gametype(COMMAND, "Add_Player", 17)
add_gametype(COMMAND, "Modify_Unit_Stats", 18)
add_gametype(COMMAND, "Harvest", 19)
add_gametype(COMMAND, "Specialize", 20)
add_gametype(COMMAND, "Barbarian_Turn", 21)
add_gametype(COMMAND, "City_Defense", 22)
add_gametype(COMMAND, "Pillage", 23)
add_gametype(COMMAND, "Abort", 24)
add_gametype(COMMAND, "Siege", 25)
add_gametype(COMMAND, "Grant", 26)
add_gametype(COMMAND, "Magic", 27)
add_gametype(COMMAND, "Research", 28)
add_gametype(COMMAND, "Status", 29)
