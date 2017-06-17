
myid = 9999;
MAX_MAP_TILE = 20;


pos = { x, y, zone }

function Init(x)
	math.randomseed(os.date());
	myid = x;
	return myid;
end


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

LEVEL = 5;
TYPE = 0;
MOVE = 0;

local EXE;
if((TYPE + MOVE) < 2) then
	EXE = (LEVEL * 5) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
else
	EXE = ( (LEVEL * 5) * 2 ) + ( (LEVEL * 5) * ( TYPE  + MOVE ));
end

NormalFixedNight = {
Level = LEVEL;
Dmg = 5;
MAX_HP = 100;
HP = 100;
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

NormalMovingNight = {
Level = LEVEL;
Dmg = 5;
MAX_HP = 100;
HP = 100;
Type = TYPE;
Move = MOVE;
Exe = EXE;
}