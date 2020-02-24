// MTE MK1 (la Churrera) v5.0
// Copyleft 2010-2014, 2020 by the Mojon Twins

// Modify p_vy
// Don't forget to use the veng_selector if VENG_SELECTOR is defined.

// Movimiento choco

if ((pad0 & sp_FIRE) == 0) {
	fire_pressed = 1;
	p_thrust -= P_THRUST_ADD;
	if (p_thrust < -P_THRUST_MAX) p_thrust = -P_THRUST_MAX;
	pad0 = 0xff;
} else {
	if (fire_pressed) {
		p_vy = p_thrust;
		p_thrust = 0;
	}
	fire_pressed = 0;
}

#ifdef BREAKABLE_WALLS
	// Detect head
	if (p_vy < -P_BREAK_VELOCITY_OFFSET) {
		_x = (gpx + 8) >> 4; _y = (gpy - 1) >> 4;
		if (attr (_x, _y) & 16) {
			break_wall ();
			p_vy = -p_vy;
		}
	}
#endif

// Water gravity & friction
if (p_vy < PLAYER_MAX_VY_CAYENDO) {
	p_vy += PLAYER_G;
	if (p_vy > PLAYER_MAX_VY_CAYENDO) p_vy = PLAYER_MAX_VY_CAYENDO;
} else {
	p_vy -= P_WATER_FRICTION;
	if (p_vy < PLAYER_MAX_VY_CAYENDO) p_vy = PLAYER_MAX_VY_CAYENDO;
}
