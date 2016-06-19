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

# Resources to generate when running type_generator.py
add_gametype(RESOURCE, 'Gold'     , 1)
add_gametype(RESOURCE, 'Happiness', 2)
add_gametype(RESOURCE, 'Sugar'    , 3)
add_gametype(RESOURCE, 'Stone'    , 4)

add_gametype(IMPROVEMENT, 'Resource', 1)

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

add_gametype(AI_ORDER, 'Attack_Unit'        , 1)
add_gametype(AI_ORDER, 'Approach_Unit'      , 2)
add_gametype(AI_ORDER, 'Attack_City'        , 3)
add_gametype(AI_ORDER, 'Approach_City'      , 4)
add_gametype(AI_ORDER, 'Pillage_Improvement', 5)
add_gametype(AI_ORDER, 'Move'               , 6)
add_gametype(AI_ORDER, 'Wander'             , 7)
