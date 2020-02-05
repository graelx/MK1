// MTE MK1 (la Churrera) v5.0
// Copyleft 2010-2014, 2020 by the Mojon Twins

// You can change this function. To set level to anything different than 0.

{
	#ifdef MODE_128K
		get_resource (TITLE_BIN, 16384);
	#else		
		#asm
			ld hl, _s_title
			ld de, 16384
			call depack
		#endasm
	#endif

	wyz_play_music (0);

	select_joyfunc ();

	_x = 11; _y = 16; _t = 7; _gp_gen = lang ? "-= MENU =-" : "-=SELECT=-";
	print_str ();

	_x = 11; _y = 17;         _gp_gen = lang ? "1 JUGAR   " : "1 PLAY    ";
	print_str ();	

	_x = 11; _y = 18;         _gp_gen =        "2 PASSWORD";
	print_str ();

	sp_UpdateNow ();
	level = 99; while (level == 99) {
		gpjt = sp_GetKey (); 
		switch (gpjt) {
			case '1': level = 0; 
			case '2': level = check_password ();
		}
	}

	wyz_stop_sound ();
	wyz_play_sound (SFX_START);
}
