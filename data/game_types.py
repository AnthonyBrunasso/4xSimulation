from store import add_gametype 

RESOURCE     = "RESOURCE"
IMPROVEMENT  = "IMPROVEMENT"
TERRAIN      = "TERRAIN"
UNIT         = "UNIT"
BUILDING     = "BUILDING"
CONSTRUCTION = "CONSTRUCTION"

# Resources to generate when running type_generator.py
add_gametype(RESOURCE, 'UNKNOWN'  , 0) # TODO: Get rid of this
add_gametype(RESOURCE, 'Gold'     , 1)
add_gametype(RESOURCE, 'Happiness', 2)
add_gametype(RESOURCE, 'Sugar'    , 3)
add_gametype(RESOURCE, 'Stone'    , 4)

add_gametype(IMPROVEMENT, 'Unknown', 0)
add_gametype(IMPROVEMENT, 'Resource', 1)

add_gametype(TERRAIN, 'Desert'   , ord('d') - ord('a'))
add_gametype(TERRAIN, 'Grassland', ord('g') - ord('a'))
add_gametype(TERRAIN, 'Mountain' , ord('m') - ord('a'))
add_gametype(TERRAIN, 'Plains'   , ord('p') - ord('a'))
add_gametype(TERRAIN, 'Water'    , ord('w') - ord('a'))
add_gametype(TERRAIN, 'Void'     , -1)

add_gametype(UNIT, 'Scout'  , 1)
add_gametype(UNIT, 'Archer' , 2)
add_gametype(UNIT, 'Phalanx', 3)

add_gametype(BUILDING, 'Town', 1)

add_gametype(CONSTRUCTION, 'Scout Unit', 0)
add_gametype(CONSTRUCTION, 'Granary'   , 1)
add_gametype(CONSTRUCTION, 'Range Unit', 2)
add_gametype(CONSTRUCTION, 'Forge'     , 3)
add_gametype(CONSTRUCTION, 'Melee Unit', 4)
add_gametype(CONSTRUCTION, 'Uber Forge', 5)
add_gametype(CONSTRUCTION, 'Invalid'   , 6)
