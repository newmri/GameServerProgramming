
myid = 9999;

function set_uid(x)
	myid = x;
	return myid;
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

Night = {
Level = LEVEL;
Type = TYPE;
Move = MOVE;
Exe = EXE;
pos = { X = 10, Y = 10, Zone = 2; }
}


