
myid = 9999;
MAX_MAP_TILE = 20;


pos = { x, y, zone }

function Init(x)
	math.randomseed(os.date());
	myid = x;
	return myid;
end

local xmin, xmax;
local ymin, ymax;
function set_pos(x, y, zone)
	xmin = (x * MAX_MAP_TILE);
	xmax = (MAX_MAP_TILE - 1) + xmin; 

	ymin = (y * MAX_MAP_TILE);
	ymax = (MAX_MAP_TILE - 1) + ymin; 

	pos.x = math.random(xmin, xmax);
	pos.y = math.random(ymin, ymax);
	pos.zone = zone;
end

function get_posx()
	return pos.x;
end
function get_posy()
	return pos.y;
end
function get_zone()
	return pos.zone;
end
function get_xmin()
	return xmin;
end
function get_xmax()
	return xmax;
end
function get_ymin()
	return ymin;
end
function get_ymax()
	return ymax;
end

LEVEL = 15;
DMG = 15;
DMAX_HP = 100;
DHP = 100;
TYPE = 0;
MOVE = 0;

local EXE;
if((TYPE + MOVE) < 2) then
	EXE = (LEVEL * 5) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
else
	EXE = ( (LEVEL * 5) * 2 ) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
end

NormalFixedBishop = {
Level = LEVEL;
Dmg = DMG;
MAX_HP = DMAX_HP;
HP = DHP;
Type = TYPE;
Move = MOVE;
Exe = EXE;
}

MOVE = 1;
if((TYPE + MOVE) < 2) then
	EXE = (LEVEL * 5) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
else
	EXE = ( (LEVEL * 5) * 2 ) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
end

NormalMovingBishop = {
Level = LEVEL;
Dmg = DMG;
MAX_HP = DMAX_HP;
HP = DHP;
Type = TYPE;
Move = MOVE;
Exe = EXE;
}

TYPE = 1;
MOVE = 0;
if((TYPE + MOVE) < 2) then
	EXE = (LEVEL * 5) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
else
	EXE = ( (LEVEL * 5) * 2 ) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
end

StarvedFixedBishop = {
Level = LEVEL;
Dmg = DMG;
MAX_HP = DMAX_HP;
HP = DHP;
Type = TYPE;
Move = MOVE;
Exe = EXE;
}

MOVE = 1;
if((TYPE + MOVE) < 2) then
	EXE = (LEVEL * 5) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
else
	EXE = ( (LEVEL * 5) * 2 ) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
end

StarvedMovingBishop = {
Level = LEVEL;
Dmg = DMG;
MAX_HP = DMAX_HP;
HP = DHP;
Type = TYPE;
Move = MOVE;
Exe = EXE;
}