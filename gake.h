struct gake_curstate {
	_Bool grid[4][4];
	long long frame;
	const char keys[];
};

struct gake_newstate {
	_Bool grid[4][4];
};
