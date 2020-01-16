// MTE MK1 (la Churrera) v5.0
// Copyleft 2010-2014, 2020 by the Mojon Twins

// player.h

void player_init (void) {
	// Inicializa player con los valores iniciales
	// (de ahí lo de inicializar).
		
	#ifndef COMPRESSED_LEVELS
		gpx = PLAYER_INI_X << 4; p_x = gpx << 6;
		gpy = PLAYER_INI_Y << 4; p_y = gpy << 6;
	#endif	
	p_vy = 0;
	p_vx = 0;
	p_cont_salto = 1;
	p_saltando = 0;
	p_frame = 0;
	p_subframe = 0;
	#ifdef PLAYER_MOGGY_STYLE
		p_facing = FACING_DOWN;
		p_facing_v = p_facing_h = 0xff;
	#else
		p_facing = 1;
	#endif	
	p_estado = 	EST_NORMAL;
	p_ct_estado = 0;
	#if !defined(COMPRESSED_LEVELS) || defined(REFILL_ME)	
		p_life = 		PLAYER_LIFE;
	#endif
	p_objs =	0;
	p_keys = 0;
	p_killed = 0;
	p_disparando = 0;
	#ifdef MAX_AMMO
		#ifdef INITIAL_AMMO
			p_ammo = INITIAL_AMMO
		#else
			p_ammo = MAX_AMMO;
		#endif
	#endif	
	pant_final = SCR_FIN;
	#ifdef TIMER_ENABLE
		ctimer.count = 0;
		ctimer.zero = 0;
		#ifdef TIMER_LAPSE
			ctimer.frames = TIMER_LAPSE;
		#endif
		#ifdef TIMER_INITIAL
			ctimer.t = TIMER_INITIAL;
		#endif
		#ifdef TIMER_START
			ctimer.on = 1;
		#else
			ctimer.on = 0;
		#endif
	#endif
}


void player_calc_bounding_box (void) {
	#if defined (BOUNDING_BOX_8_BOTTOM)
		#asm
			ld  a, (_gpx)
			add 4
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx1), a
			ld  a, (_gpx)
			add 11
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx2), a
			ld  a, (_gpy)
			add 8
			srl a
			srl a
			srl a
			srl a
			ld  (_pty1), a
			ld  a, (_gpy)
			add 15
			srl a
			srl a
			srl a
			srl a
			ld  (_pty2), a
		#endasm
	#elif defined (BOUNDING_BOX_8_CENTERED)
		#asm
			ld  a, (_gpx)
			add 4
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx1), a
			ld  a, (_gpx)
			add 11
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx2), a
			ld  a, (_gpy)
			add 4
			srl a
			srl a
			srl a
			srl a
			ld  (_pty1), a
			ld  a, (_gpy)
			add 11
			srl a
			srl a
			srl a
			srl a
			ld  (_pty2), a
		#endasm
	#else
		#asm
			ld  a, (_gpx)
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx1), a
			ld  a, (_gpx)
			add 15
			srl a
			srl a
			srl a
			srl a
			ld  (_ptx2), a
			ld  a, (_gpy)
			srl a
			srl a
			srl a
			srl a
			ld  (_pty1), a
			ld  a, (_gpy)
			add 15
			srl a
			srl a
			srl a
			srl a
			ld  (_pty2), a
		#endasm
	#endif
}

unsigned char player_move (void) {
	wall_v = wall_h = 0;
		
	// ***************************************************************************
	//  MOVEMENT IN THE VERTICAL AXIS
	// ***************************************************************************

	#ifndef PLAYER_MOGGY_STYLE
		
		// Do gravity
		
		#asm
				; Signed comparisons are hard
				; p_vy <= PLAYER_MAX_VY_CAYENDO - PLAYER_G

				; We are going to take a shortcut.
				; If p_vy < 0, just add PLAYER_G.
				; If p_vy > 0, we can use unsigned comparition anyway.

				ld  hl, (_p_vy)
				bit 7, h
				jr  nz, _player_gravity_add 	; < 0

				ld  de, PLAYER_MAX_VY_CAYENDO - PLAYER_G
				or  a
				push hl
				sbc hl, de
				pop hl
				jr  nc, _player_gravity_maximum

			._player_gravity_add
				ld  de, PLAYER_G
				add hl, de
				jr  _player_gravity_vy_set

			._player_gravity_maximum
				ld  hl, PLAYER_MAX_VY_CAYENDO

			._player_gravity_vy_set
				ld  (_p_vy), hl

			._player_gravity_done

			#ifdef PLAYER_CUMULATIVE_JUMP
				ld  a, (_p_jmp_on)
				or  a
				jr  nz, _player_gravity_p_gotten_done
			#endif

				ld  a, (_p_gotten)
				or  a
				jr  z, _player_gravity_p_gotten_done

				xor a
				ld  (_p_vy), a

			._player_gravity_p_gotten_done
		#endasm	
	#else

		// Pad do

		if ( ! ((pad0 & sp_UP) == 0 || (pad0 & sp_DOWN) == 0)) {
			p_facing_v = 0xff;
			if (p_vy > 0) {
				p_vy -= PLAYER_RX;
				if (p_vy < 0) p_vy = 0;
			} else if (p_vy < 0) {
				p_vy += PLAYER_RX;
				if (p_vy > 0) p_vy = 0;
			}
		}

		if ((pad0 & sp_UP) == 0) {
			p_facing_v = FACING_UP;
			if (p_vy > -PLAYER_MAX_VX) p_vy -= PLAYER_AX;
		}

		if ((pad0 & sp_DOWN) == 0) {
			p_facing_v = FACING_DOWN;
			if (p_vy < PLAYER_MAX_VX) p_vy += PLAYER_AX;
		}
	#endif

	#ifdef PLAYER_HAS_JETPAC
		if ((pad0 & sp_UP) == 0) {
			p_vy -= PLAYER_INCR_JETPAC;
			if (p_vy < -PLAYER_MAX_VY_JETPAC) p_vy = -PLAYER_MAX_VY_JETPAC;
		}
	#endif

	p_y += p_vy;
	
	#if !defined (PLAYER_MOGGY_STYLE)
		if (p_gotten) p_y += ptgmy;
	#endif

	// Safe
		
	if (p_y < 0) p_y = 0;
	if (p_y > 9216) p_y = 9216;

	gpy = p_y >> 6;

	// Collision, may set possee, hit_v

	player_calc_bounding_box ();

	hit_v = 0;
	cx1 = ptx1; cx2 = ptx2;
	#if defined (PLAYER_MOGGY_STYLE)
		if (p_vy < 0)
	#else	
		if (p_vy + ptgmy < 0) 
	#endif
	{
		cy1 = cy2 = pty1;
		cm_two_points ();

		if ((at1 & 8) || (at2 & 8)) {
			#ifdef PLAYER_BOUNCE_WITH_WALLS
				p_vy = -(p_vy / 2);
			#else
				p_vy = 0;
			#endif

			#if defined (BOUNDING_BOX_8_BOTTOM)			
				gpy = ((pty1 + 1) << 4) - 8;
			#elif defined (BOUNDING_BOX_8_CENTERED)
				gpy = ((pty1 + 1) << 4) - 4;
			#elif defined (BOUNDING_BOX_TINY_BOTTOM)
				gpy = ((pty1 + 1) << 4) - 14;
			#else
				gpy = ((pty1 + 1) << 4);
			#endif

			p_y = gpy << 6;

			wall_v = WTOP;
		}
	}
	
	#if defined (PLAYER_MOGGY_STYLE)
		if (p_vy > 0)
	#else	
		if (p_vy + ptgmy > 0)
	#endif
	{
		cy1 = cy2 = pty2;
		cm_two_points ();

		#ifdef PLAYER_MOGGY_STYLE
			if ((at1 & 8) || (at2 & 8))
		#else
			// Greed Optimization tip! Remove this line and uncomment the next one:
			// (As long as you don't have type 8 blocks over type 4 blocks in your game, the short line is fine)
			if ((at1 & 8) || (at2 & 8) || (((gpy - 1) & 15) < 8 && ((at1 & 4) || (at2 & 4))))
			//if (((gpy - 1) & 15) < 7 && ((at1 & 12) || (at2 & 12))) {
		#endif			
		{
			#ifdef PLAYER_BOUNCE_WITH_WALLS
				p_vy = -(p_vy / 2);
			#else
				p_vy = 0;
			#endif
				
			#if defined (BOUNDING_BOX_8_BOTTOM) || defined (BOUNDING_BOX_TINY_BOTTOM)
				gpy = (pty2 - 1) << 4;
			#elif defined (BOUNDING_BOX_8_CENTERED)				
				gpy = ((pty2 - 1) << 4) + 4;
			#else
				gpy = (pty2 - 1) << 4;				
			#endif

			p_y = gpy << 6;
			
			wall_v = WBOTTOM;
		}
	}

	#ifndef DEACTIVATE_EVIL_TILE
		if (p_vy) hit_v = ((at1 & 1) || (at2 & 1));
	#endif
	
	gpxx = gpx >> 4;
	gpyy = gpy >> 4;

	#ifndef PLAYER_MOGGY_STYLE
		cy1 = cy2 = (gpy + 16) >> 4;
		cx1 = ptx1; cx2 = ptx2;
		cm_two_points ();
		possee = ((at1 & 12) || (at2 & 12)) && (gpy & 15) < 8;
	#endif

	// Jump

	#ifdef PLAYER_HAS_JUMP
		
		#if defined (PLAYER_CAN_FIRE) && !defined (USE_TWO_BUTTONS)
			rda = (pad0 & sp_UP) == 0;
		#elif defined (PLAYER_CAN_FIRE) && defined (USE_TWO_BUTTONS)
			rda = sp_KeyPressed (key_jump);
		#else
			rda = (pad0 & sp_FIRE) == 0;
		#endif

		if (rda) {
			if (p_saltando == 0) {
				if (possee || p_gotten || hit_v) {
					p_saltando = 1;
					p_cont_salto = 0;
					#ifdef MODE_128K
						wyz_play_sound (2);
					#else
						beep_fx (3);
					#endif
				}
			} else {
				p_vy -= (PLAYER_VY_INICIAL_SALTO + PLAYER_INCR_SALTO - (p_cont_salto >> 1));
				if (p_vy < -PLAYER_MAX_VY_SALTANDO) p_vy = -PLAYER_MAX_VY_SALTANDO;
				++ p_cont_salto;
				if (p_cont_salto == 9) p_saltando = 0;
			}
		} else p_saltando = 0;
	
	#endif

	// Bootee engine

	#ifdef PLAYER_BOOTEE
		if ( p_saltando == 0 && (possee || p_gotten || hit_v) ) {
			p_saltando = 1;
			p_cont_salto = 0;
			#ifdef MODE_128K
				wyz_play_sound (2);
			#else				
				beep_fx (3);
			#endif
		}
		
		if (p_saltando ) {
			p_vy -= (PLAYER_VY_INICIAL_SALTO + PLAYER_INCR_SALTO - (p_cont_salto>>1));
			if (p_vy < -PLAYER_MAX_VY_SALTANDO) p_vy = -PLAYER_MAX_VY_SALTANDO;
			++ p_cont_salto;
			if (p_cont_salto == 8)
				p_saltando = 0;
		}
	#endif	

	// ***************************************************************************
	//  MOVEMENT IN THE HORIZONTAL AXIS
	// ***************************************************************************

	if ( ! ((pad0 & sp_LEFT) == 0 || (pad0 & sp_RIGHT) == 0)) {
		#ifdef PLAYER_MOGGY_STYLE		
			p_facing_h = 0xff;
		#endif
		if (p_vx > 0) {
			p_vx -= PLAYER_RX;
			if (p_vx < 0) p_vx = 0;
		} else if (p_vx < 0) {
			p_vx += PLAYER_RX;
			if (p_vx > 0) p_vx = 0;
		}
	}

	if ((pad0 & sp_LEFT) == 0) {
		#ifdef PLAYER_MOGGY_STYLE
			p_facing_h = FACING_LEFT;
		#endif
		if (p_vx > -PLAYER_MAX_VX) {
			#ifndef PLAYER_MOGGY_STYLE			
				p_facing = 0;
			#endif
			p_vx -= PLAYER_AX;
		}
	}

	if ((pad0 & sp_RIGHT) == 0) {
		#ifdef PLAYER_MOGGY_STYLE	
			p_facing_h = FACING_RIGHT;
		#endif
		if (p_vx < PLAYER_MAX_VX) {
			p_vx += PLAYER_AX;
			#ifndef PLAYER_MOGGY_STYLE						
				p_facing = 1;
			#endif
		}
	}

	p_x = p_x + p_vx;
	#ifndef PLAYER_MOGGY_STYLE
		p_x += ptgmx;
	#endif
	
	// Safe
	
	if (p_x < 0) p_x = 0;
	if (p_x > 14336) p_x = 14336;

	gpox = gpx;
	gpx = p_x >> 6;
		
	// Collision. May set hit_h
	player_calc_bounding_box ();

	hit_h = 0;
	cy1 = pty1; cy2 = pty2;

	#if defined (PLAYER_MOGGY_STYLE)
		if (p_vx < 0)
	#else	
		if (p_vx + ptgmx < 0)
	#endif
	{
		cx1 = cx2 = ptx1;
		cm_two_points ();

		if ((at1 & 8) || (at2 & 8)) {
			#ifdef PLAYER_BOUNCE_WITH_WALLS
				p_vx = -(p_vx / 2);
			#else
				p_vx = 0;
			#endif

			#if defined (BOUNDING_BOX_8_BOTTOM) || defined (BOUNDING_BOX_8_CENTERED) || defined (BOUNDING_BOX_TINY_BOTTOM)				
				gpx = ((ptx1 + 1) << 4) - 4;
			#else
				gpx = ((ptx1 + 1) << 4);
			#endif

			p_x = gpx << 6;
			wall_h = WLEFT;
		}
		#ifndef DEACTIVATE_EVIL_TILE
			else hit_h = ((at1 & 1) || (at2 & 1));
		#endif
	}

	#if defined (PLAYER_MOGGY_STYLE)
		if (p_vx > 0)
	#else	
		if (p_vx + ptgmx > 0)
	#endif
	{
		cx1 = cx2 = ptx2; 
		cm_two_points ();

		if ((at1 & 8) || (at2 & 8)) {
			#ifdef PLAYER_BOUNCE_WITH_WALLS
				p_vx = -(p_vx / 2);
			#else
				p_vx = 0;
			#endif

			#if defined (BOUNDING_BOX_8_BOTTOM) || defined (BOUNDING_BOX_8_CENTERED) || defined (BOUNDING_BOX_TINY_BOTTOM)				
				gpx = (ptx1 << 4) + 4;
			#else
				gpx = (ptx1 << 4);
			#endif

			p_x = gpx << 6;
			wall_h = WRIGHT;
		}
		#ifndef DEACTIVATE_EVIL_TILE
			else hit_h = ((at1 & 1) || (at2 & 1));
		#endif
	}

	// Priority to decide facing

	#ifdef PLAYER_MOGGY_STYLE
		#ifdef TOP_OVER_SIDE
			if (p_facing_v != 0xff) {
				p_facing = p_facing_v;
			} else if (p_facing_h != 0xff) {
				p_facing = p_facing_h;
			}
		#else
			if (p_facing_h != 0xff) {
				p_facing = p_facing_h;
			} else if (p_facing_v != 0xff) {
				p_facing = p_facing_v;
			}
		#endif	
	#endif

	#ifdef FIRE_TO_PUSH	
	pushed_any = 0;
	#endif

	#if defined (PLAYER_PUSH_BOXES) || !defined (DEACTIVATE_KEYS)
		gpx = p_x >> 6;
		cx1 = (gpx + 8) >> 4;
		cy1 = (gpy + 8) >> 4;
		#ifdef PLAYER_GENITAL
			if (wall_v == WTOP) {
				// interact up			
				#if defined (BOUNDING_BOX_8_BOTTOM)
					cy1 = (gpy + 7) >> 4;
				#elif defined (BOUNDING_BOX_8_CENTERED)
					cy1 = (gpy + 3) >> 4;
				#else
					cy1 = gpy >> 3;		
				#endif

				if (attr (cx1, cy1) == 10) {
					x0 = x1 = cx1; y0 = cy1; y1 = cy1 - 1;
					process_tile ();
				}

			} else if (wall_v == WBOTTOM) {
				// interact down
				#if defined (BOUNDING_BOX_8_BOTTOM)
					cy1 = (gpy + 16) >> 4;
				#elif defined (BOUNDING_BOX_8_CENTERED)
					cy1 = (gpy + 12) >> 4;
				#else
					cy1 = (gpy + 16) >> 3;				
				#endif		
			
				if (attr (cx1, cy1) == 10) {
					x0 = x1 = cx1; y0 = cy1; y1 = cy1 + 1;
					process_tile ();
				}
			} else
		#endif	
		
		if (wall_h == WLEFT) {		
			// interact left
			#if defined (BOUNDING_BOX_8_BOTTOM) || defined (BOUNDING_BOX_8_CENTERED)
				cx1 = (gpx + 3) >> 4;
			#else
				cx1 = gpx >> 4;		
			#endif		

			if (attr (cx1, cy1) == 10) {
				y0 = y1 = cy1; x0 = cx1; x1 = cx1 - 1;
				process_tile ();
			}
		} else if (wall_h == WRIGHT) {
			// interact right
			#if defined (BOUNDING_BOX_8_BOTTOM) || defined (BOUNDING_BOX_8_CENTERED)
				cx1 = (gpx + 12) >> 4;
			#else
				cx1 = (gpx + 16) >> 4;		
			#endif		
			if (attr (cx1, cy1) == 10) {
				y0 = y1 = cy1; x0 = cx1; x1 = cx1 + 1;
				process_tile ();
			}
		}
	#endif

	#ifdef PLAYER_CAN_FIRE
		// Disparos
		#ifdef USE_TWO_BUTTONS
			#ifdef FIRE_TO_PUSH	
				if (((pad0 & sp_FIRE) == 0 || sp_KeyPressed (key_fire)) && p_disparando == 0 && !pushed_any) {
			#else
				if (((pad0 & sp_FIRE) == 0 || sp_KeyPressed (key_fire)) && p_disparando == 0) {
			#endif
		#else
			#ifdef FIRE_TO_PUSH	
				if ((pad0 & sp_FIRE) == 0 && p_disparando == 0 && !pushed_any) {
			#else
				if ((pad0 & sp_FIRE) == 0 && p_disparando == 0) {
			#endif
		#endif
			p_disparando = 1;
			bullets_fire ();
		}
		
		if ((pad0 & sp_FIRE)) 
			p_disparando = 0;

	#endif

	#ifndef DEACTIVATE_EVIL_TILE
		// Tiles que te matan. 
		// hit_v tiene preferencia sobre hit_h
		hit = 0;
		if (hit_v) {
			hit = 1;
				p_vy = addsign (-p_vy, PLAYER_MAX_VX);
		} else if (hit_h) {
			hit = 1;
				p_vx = addsign (-p_vx, PLAYER_MAX_VX);
		}
		
		if (hit) {
			#ifdef PLAYER_FLICKERS
				if (p_estado == EST_NORMAL) {
					p_estado = EST_PARP;
					p_ct_estado = 50;
			#else
				{
			#endif		
				#ifdef MODE_128K
					p_killme = 8;
				#else		
					p_killme = 4;
				#endif
			}
		}
	#endif

	// Select animation frame 
	
	#ifndef PLAYER_MOGGY_STYLE
		#ifdef PLAYER_BOOTEE
			gpit = p_facing << 2;
			if (p_vy == 0) {
				p_next_frame = player_frames [gpit];
			} else if (p_vy < 0) {
				p_next_frame = player_frames [gpit + 1];
			} else {
				p_next_frame = player_frames [gpit + 2];
			}
		#else	
			if (!possee && !p_gotten) {
				p_next_frame = player_frames [8 + p_facing];
			} else {
				gpit = p_facing << 2;
				if (p_vx == 0) {
					rda = 1;
				} else {
					rda = ((gpx + 4) >> 3) & 3;
				}
				p_next_frame = player_frames [gpit + rda];
			}
		#endif
	#else
		
		if (p_vx || p_vy) {
			++ p_subframe;
			if (p_subframe == 4) {
				p_subframe = 0;
				p_frame = !p_frame;
				#ifdef PLAYER_STEP_SOUND			
					step (); 
				#endif
			}
		}
		
		p_next_frame = player_frames [p_facing + p_frame];
	#endif
}

void player_kill (unsigned char sound) {
	p_killme = 0;

	if (p_life == 0) return;
	-- p_life;

	#ifdef MODO_128K
		wyz_play_sound (sound);
	#else
		beep_fx (sound);
	#endif

	#ifdef CP_RESET_WHEN_DYING
		#ifdef CP_RESET_ALSO_FLAGS
			mem_load ();
		#else
			n_pant = sg_pool [MAX_FLAGS];
			p_x = sg_pool [MAX_FLAGS + 1] << 10;
			p_y = sg_pool [MAX_FLAGS + 2] << 10;
		#endif	
		o_pant = 0xff; // Reload
	#endif
}

