// POKEY Frequencies Calculator
// by VinsCool
// Based on the code used for the Pitch Calculations in Raster Music Tracker 1.31+

#include "main.h"
using namespace std;

const char* notes[] =
{
	"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"
};

//arrays of tuning tables to generate...
//
//PAGE_DISTORTION_2 => Address 0xB000
int tab_64khz_2[64] = { 0 };
int tab_179mhz_2[64] = { 0 };
int tab_16bit_2[128] = { 0 };
//
//PAGE_DISTORTION_A => Address 0xB100
int tab_64khz_a_pure[64] = { 0 };
int tab_179mhz_a_pure[64] = { 0 };
int tab_16bit_a_pure[128] = { 0 };
//
//PAGE_DISTORTION_C => Address 0xB200
int tab_64khz_c_buzzy[64] = { 0 };
int tab_179mhz_c_buzzy[64] = { 0 };
int tab_16bit_c_buzzy[128] = { 0 };
//
//PAGE_DISTORTION_E => Address 0xB300
int tab_64khz_c_gritty[64] = { 0 };
int tab_179mhz_c_gritty[64] = { 0 };
int tab_16bit_c_gritty[128] = { 0 };
//
//PAGE_EXTRA_0 => Address 0xB400, 0xB480 for 15khz Pure and 0xB4C0 for 15khz Buzzy
//Croissant Sawtooth ch1	//no generator yet
//Croissant Sawtooth ch3	//no generator yet
int tab_15khz_a_pure[64] = { 0 };
int tab_15khz_c_buzzy[64] = { 0 };
//
//PAGE_EXTRA_1 => Address 0xB500
//Clarinet Lo	//no generator yet
//Clarinet Hi	//no generator yet
//unused
//unused
//
//PAGE_EXTRA_2 => Address 0xB600
int tab_64khz_c_unstable[64] = { 0 };
int tab_179mhz_c_unstable[64] = { 0 };
int tab_16bit_c_unstable[128] = { 0 };
//

int i_audf, i_audc, i_audctl, i_ch_index, FREQ_17, machine_region;
double i_tuning, p[2200] = { 0 };	//this should be more than enough for all possible values...

//AUDCTL bits
bool CLOCK_15 = 0;		//0x01
bool HPF_CH24 = 0;		//0x02
bool HPF_CH13 = 0;		//0x04
bool JOIN_34 = 0;		//0x08
bool JOIN_12 = 0;		//0x10
bool CH3_179 = 0;		//0x20
bool CH1_179 = 0;		//0x40
bool POLY9 = 0;			//0x80
//bool TWO_TONE = 0;	//0x8B
bool JOIN_16BIT = 0;	//valid 16-bit mode
bool CLOCK_179 = 0;		//valid 1.79mhz mode
//bool SAWTOOTH = 0;
//bool SAWTOOTH_INVERTED = 0;

bool IS_BUZZY_DIST_C = 0;
bool IS_GRITTY_DIST_C = 0;
bool IS_UNSTABLE_DIST_C = 0;

bool IS_METALLIC_POLY9 = 0;

int main()
{
	cout << endl << "POKEY Frequencies Calculator " VERSION " by VinsCool" << endl;
	SEPARATOR;
	cin.exceptions (std::ios::eofbit);
	try
	{
		i_tuning = get_tuning();
		machine_region = get_region();
	}
	catch (const std::ios::failure& e)
	{
		cout << "End of file reached, exiting." << endl;
		return 1;
	}
	FREQ_17 = (machine_region == 1) ? FREQ_17_PAL : FREQ_17_NTSC;
	SEPARATOR;

	fstream file;
	// Backup streambuffers of cout
	streambuf* stream_buffer_cout = cout.rdbuf();
	// Get the streambuffer of the file
	streambuf* stream_buffer_file = file.rdbuf();

	cout << fixed;
	cout << setprecision(2);
	cout << uppercase;

/////////////////////////////////////////////////////////////////////////////

	cout << endl << "Generating frequencies using input parameters... " << flush;

	file.open("frequencies.txt", ios::out);

	// Redirect cout to file
	cout.rdbuf(stream_buffer_file);

	cout << "File generated using POKEY Frequencies Calculator " VERSION << endl;
	SEPARATOR;
	cout << endl << "A-4 Tuning = " << i_tuning << " Hz" << endl;
	cout << "Machine region = ";
	if (machine_region == 1) cout << "PAL";
	else cout << "NTSC";
	cout << endl;
	SEPARATOR;
	cout << endl << "Real frequencies generated using the tuning parameter, they will be used for reference...\n" << endl;
	real_freq();
	SEPARATOR;

	//All the frequencies that could be calculated will be generated inside this entire block
	for (int d = 0x00; d < 0xE0; d += 0x20)
	{
		if (d == 0x60) continue;	//duplicate of Distortion 2
		i_audc = d;
		int distortion = d >> 4;
		bool IS_POLY9_NOISE = (d == 0x00 || d == 0x80) ? 1 : 0;	//White Noise generators only have useful pitches in Poly9 mode

		for (int c = 0; c < 4; c++)
		{
			i_ch_index = c;
			cout << endl << "Distortion " << uppercase << hex << distortion << dec;
			if (c == 0) { i_audctl = 0x00; cout << ", 64kHz "; }
			if (c == 1) { i_audctl = 0x01; cout << ", 15kHz "; }
			if (c == 2) { i_audctl = 0x20; cout << ", 1.79MHz "; }
			if (c == 3) { i_audctl = 0x28; cout << ", 16-bit "; }
			if (IS_POLY9_NOISE) { cout << "(Poly9)";  i_audctl = i_audctl | 0x80; }
			cout << endl;
			SEPARATOR;
			if (c == 3) for (int i = 0xFFFF; i >= 0; i--) { i_audf = i; generate_freq(); }	//16-bit
			else for (int i = 0xFF; i >= 0; i--) { i_audf = i; generate_freq(); }	//8-bit
			SEPARATOR;
		}
	}
	file.close();	//done, save the .txt file

	// Redirect cout back to screen
	cout.rdbuf(stream_buffer_cout);

	cout << "OK" << endl;

///////////////////////////////////////////////////////////

	//TODO: generate tables using a macro

	cout << "Generating tables using input parameters... " << flush;

	file.open("tables.txt", ios::out);

	// Redirect cout to file
	cout.rdbuf(stream_buffer_file);

	cout << "File generated using POKEY Frequencies Calculator " VERSION << endl;
	SEPARATOR;
	cout << endl << "A-4 Tuning = " << i_tuning << " Hz" << endl;
	cout << "Machine region = ";
	if (machine_region == 1) cout << "PAL";
	else cout << "NTSC";
	cout << endl;
	SEPARATOR;

///////////////////////////////////////////////////////////

	//All the tables that could be calculated will be generated inside this entire block
	for (int d = 0x00; d < 0xE0; d += 0x20)
	{
		if (d == 0x00 || d == 0x40 || d == 0x60 || d == 0x80) continue;	//no good use yet
		int distortion = d >> 4;
		int note_offset[4] = { 0 };
		
		int dist_2_offset[4] = {12, 0, 48, 24};
		int dist_a_offset[4] = {48, 24, 108, 24};
		int dist_c_buzzy_offset[4] = {24, 12, 84, 24};
		int dist_c_gritty_offset[4] = {12, 0, 60, 24};
		int dist_c_unstable_offset[4] = {36, 0, 96, 24};
		
		bool IS_15KHZ = 0;
		bool IS_179MHZ = 0;
		bool IS_16BIT = 0;
		
		if (d == 0xC0)
		{
			IS_BUZZY_DIST_C = 1;	//iteration 1: Buzzy
			IS_GRITTY_DIST_C = 0;
			IS_UNSTABLE_DIST_C = 0;
		}
		int dist_c_counter = 0;	//a shitty hack but who will call the police really?
repeat_dist_c:
		for (int c = 0; c < 4; c++)
		{
			if (d == 0x20) note_offset[c] = dist_2_offset[c]; 
			else if (d == 0xA0) note_offset[c] = dist_a_offset[c]; 
			else if (d == 0xC0)
			{
				if (IS_BUZZY_DIST_C) note_offset[c] = dist_c_buzzy_offset[c]; 
				else if (IS_GRITTY_DIST_C) note_offset[c] = dist_c_gritty_offset[c]; 
				else if (IS_UNSTABLE_DIST_C) note_offset[c] = dist_c_unstable_offset[c]; 
			}
			if (!note_offset[c]) continue;	//if no offset, no table should be created
			
			cout << endl << "Distortion " << uppercase << hex << distortion << dec;
			if (c == 0) { IS_15KHZ = 0; IS_179MHZ = 0; IS_16BIT = 0; cout << ", 64kHz "; }
			if (c == 1) { IS_15KHZ = 1; IS_179MHZ = 0; IS_16BIT = 0; cout << ", 15kHz "; }
			if (c == 2) { IS_15KHZ = 0; IS_179MHZ = 1; IS_16BIT = 0; cout << ", 1.79MHz "; }
			if (c == 3) { IS_15KHZ = 0; IS_179MHZ = 0; IS_16BIT = 1; cout << ", 16-bit "; }

			if (d == 0xC0)
			{
				if (IS_BUZZY_DIST_C) cout << "(Buzzy)";
				else if (IS_GRITTY_DIST_C) cout << "(Gritty)";
				else if (IS_UNSTABLE_DIST_C) cout << "(Unstable)";
			}
			
			cout << endl;
			SEPARATOR;
			
			macro_table_gen(d, note_offset[c], IS_15KHZ, IS_179MHZ, IS_16BIT);
			
			cout << endl;
			SEPARATOR;		
		}
		if (d == 0xC0) 
		{	dist_c_counter++;	//the number of times the counter was used in the Distortion C case
			if (dist_c_counter == 1)
			{
				IS_BUZZY_DIST_C = 0;
				IS_GRITTY_DIST_C = 1;		//iteration 2: Gritty
				goto repeat_dist_c;
			}
			else if (dist_c_counter == 2)
			{
				IS_GRITTY_DIST_C = 0;
				IS_UNSTABLE_DIST_C = 1;		//iteration 3: Unstable
				goto repeat_dist_c;
			}
			else IS_UNSTABLE_DIST_C = 0;		//done all Distortion C modes
		}
	}

///////////////////////////////////////////////////////////

	file.close();	//done, save the .txt file

	// Redirect cout back to screen
	cout.rdbuf(stream_buffer_cout);

	cout << "OK" << endl;

/////////////////////////////////////////////////////////////////////////////

	// Redirect cout back to screen
	cout.rdbuf(stream_buffer_cout);
	SEPARATOR;

	cout << endl << "Done... " << flush;
	wait(2);	//2 seconds
	cout << endl << endl;
	return 0;
}

void wait(int sec)
{
	clock_t count = clock() + sec * CLOCKS_PER_SEC;
	while (clock() < count) {}
}

void real_freq()	//Parts of this code was used in RMT 1.31+
{
	double PITCH = 0;
	double ratio = pow(2.0, 1.0 / 12.0);
	double centnum = 0;
	char n[3];
	for (int i = 0; i < 180; i++)
	{
		double tuning = i_tuning / 64;
		double PITCH = tuning * pow(ratio, i + 3);
		int notenum = (int)round(centnum * 0.01);
		int note = (notenum + 180) % 12;
		int octave = (notenum / 12) - 1;
		n[0] = notes[note][0];
		n[1] = notes[note][1];
		n[2] = 0;
		centnum += 100;
		p[i * 12] = PITCH;
		if (PITCH < 16.1) continue;	//~C-0, lowest pitch allowed
		cout << n << octave << " = " << PITCH << "Hz" << endl;
	}
}

void generate_freq()	//Parts of this code was used in RMT 1.31+
{
	//register variables
	int i = i_ch_index;
	int audctl = i_audctl;
//	int skctl = 0;
	int audf = i_audf;
//	int audf2 = 0;
//	int audf3 = 0;
	int audf16 = i_audf;		//a 16bit number is fed into it directly instead
	int audc = i_audc;
//	int vol = audc & 0x0f;
	int dist = audc & 0xf0;

	int modoffset = 0;

	//variables for pitch calculation
	double PITCH = 0;
	double divisor = 0;
	int coarse_divisor = 0;

	IS_BUZZY_DIST_C = 0;
	IS_GRITTY_DIST_C = 0;
	IS_UNSTABLE_DIST_C = 0;

	IS_METALLIC_POLY9 = 0;

	CLOCK_15 = audctl & 0x01;
	HPF_CH24 = audctl & 0x02;
	HPF_CH13 = audctl & 0x04;
	JOIN_34 = audctl & 0x08;
	JOIN_12 = audctl & 0x10;
	CH3_179 = audctl & 0x20;
	CH1_179 = audctl & 0x40;
	POLY9 = audctl & 0x80;
//	TWO_TONE = (skctl == 0x8B) ? 1 : 0;

	//combined modes for some special output...
	JOIN_16BIT = ((JOIN_12 && CH1_179 && (i == 1 || i == 5)) || (JOIN_34 && CH3_179 && (i == 3 || i == 7))) ? 1 : 0;
	CLOCK_179 = ((CH1_179 && (i == 0 || i == 4)) || (CH3_179 && (i == 2 || i == 6))) ? 1 : 0;
	if (JOIN_16BIT || CLOCK_179) CLOCK_15 = 0;	//override, these 2 take priority over 15khz mode

	/*
	SAWTOOTH = (CH1_179 && CH3_179 && HPF_CH13 && (dist == 0xA0 || dist == 0xE0) && (i == 0 || i == 4)) ? 1 : 0;
	SAWTOOTH_INVERTED = 0;

	if (i % 4 == 0)	//only in valid sawtooth channels
	audf3 = g_atarimem[idx[i + 2]];
	*/

	modoffset = 1;
	coarse_divisor = 1;
	divisor = 1;
	int v_modulo = 0;
	bool IS_VALID = 0;

	if (JOIN_16BIT) modoffset = 7;
	else if (CLOCK_179) modoffset = 4;
	else coarse_divisor = (CLOCK_15) ? 114 : 28;

	switch (dist)
	{
	case 0x60:	//Duplicate of Distortion 2
	case 0x20:
		divisor = 31;
		v_modulo = 31;
		IS_VALID = ((audf + modoffset) % v_modulo == 0) ? 0 : 1;
		break;

	case 0x40:
/*
VinsCool — Today at 01:07
after a bit of screwing around I found one of the Distortion 4 common divisor
77.5 appears to be the number used for the Smooth Timbres table
VinsCool — Today at 01:46
ok I have spent enough time already for the week end but I think I found the common divisor for the other table
that would be 232.5
*/
		break;

	case 0x00:
	case 0x80:
		divisor = 255.5;	//Metallic Buzzy
		v_modulo = 73;
		if (POLY9)
		{
			if (CLOCK_179 || JOIN_16BIT)
				IS_METALLIC_POLY9 = ((audf + modoffset) % 7 == 0) ? 1 : 0;
			else
				IS_METALLIC_POLY9 = 1;

			if (IS_METALLIC_POLY9) divisor = 36.5;
			IS_VALID = ((audf + modoffset) % v_modulo == 0) ? 0 : 1;
			if (dist == 0x00 && ((audf + modoffset) % 31 == 0)) IS_VALID = 0;
		}
		break;

	case 0xE0:	//Duplicate of Distortion A
	case 0xA0:
		IS_VALID = 1;
		break;

	case 0xC0:
		divisor = 7.5;		//Gritty
		v_modulo = (CLOCK_15) ? 5 : 15;
		IS_UNSTABLE_DIST_C = ((audf + modoffset) % 5 == 0) ? 1 : 0;
		IS_BUZZY_DIST_C = ((audf + modoffset) % 3 == 0 || CLOCK_15) ? 1 : 0;
		if (IS_UNSTABLE_DIST_C) divisor = 1.5;	//Unstable
		if (IS_BUZZY_DIST_C) divisor = 2.5;	//Buzzy
		IS_VALID = ((audf + modoffset) % v_modulo == 0) ? 0 : 1;
		break;
	}
	if (IS_VALID)
		PITCH = get_pitch(audf, coarse_divisor, divisor, modoffset);

	if (PITCH > 16.1)	//~C-0, lowest pitch allowed
	{
		char n[3];
		double tuning = i_tuning;
		double centnum = 1200 * log2(PITCH / tuning);
		int notenum = (int)round(centnum * 0.01) + 72;
		int note = ((notenum + 96) - 3) % 12;
		int octave = ((notenum - 3) / 12) - 1;
		double cents = (centnum - (notenum - 72) * 100);
		double reference = p[(notenum - 3) * 12];
		n[0] = notes[note][0];
		n[1] = notes[note][1];
		n[2] = 0;
		cout << endl << hex << "AUDF = " << i_audf;

		if (dist == 0x00 || dist == 0x80)
		{
			cout << " (";
			if (IS_METALLIC_POLY9) cout << "Metallic";
			else cout << "Fuzzy";
			cout << ")";
		}

		if (dist == 0xC0)
		{
			cout << " (";
			if (IS_UNSTABLE_DIST_C) cout << "Unstable";
			else if (IS_BUZZY_DIST_C) cout << "Buzzy";
			else cout << "Gritty";
			cout << ")";
		}

		cout << endl << "Note = " << n << dec << octave << " (";
		if (cents >= 0) cout << "+";
		cout << cents << " cents off), " << PITCH << "Hz" << endl;
		cout << "Theoretical = " << reference << "Hz" << endl;
	}
}

void generate_table(int note, double freq, int distortion, bool IS_15KHZ, bool IS_179MHZ, bool IS_16BIT)
{
	int audf = 0;
	int modoffset = 1;
	int coarse_divisor = 1;
	int v_modulo = 0;
	double divisor = 1;
	double PITCH = 0;
	
	//since globals are specifically wanted, the parameters are used to define these flags here
	CLOCK_15 = IS_15KHZ;
	CLOCK_179 = IS_179MHZ;
	JOIN_16BIT = IS_16BIT;

	if (JOIN_16BIT) modoffset = 7;
	else if (CLOCK_179) modoffset = 4;
	else coarse_divisor = (CLOCK_15) ? 114 : 28;

	switch (distortion)
	{
	case 0x20:
		divisor = 31;
		v_modulo = 31;
		audf = get_audf(freq, coarse_divisor, divisor, modoffset);

		if ((audf + modoffset) % v_modulo == 0)	//invalid values
			audf = delta_audf(audf, freq, coarse_divisor, divisor, modoffset, distortion);

		if (!JOIN_16BIT && (audf > 0xFF || audf < 0x00)) break;	//invalid 8-bit range!
		if (JOIN_16BIT) tab_16bit_2[note * 2] = audf;
		else if (CLOCK_179) tab_179mhz_2[note] = audf;
		else tab_64khz_2[note] = audf;
		break;

	case 0xA0:
		audf = get_audf(freq, coarse_divisor, divisor, modoffset);
		if (!JOIN_16BIT && (audf > 0xFF || audf < 0x00)) break;	//invalid 8-bit range!
		if (JOIN_16BIT) tab_16bit_a_pure[note * 2] = audf;
		else if (CLOCK_179) tab_179mhz_a_pure[note] = audf;
		else if (CLOCK_15) tab_15khz_a_pure[note] = audf;
		else tab_64khz_a_pure[note] = audf;
		break;

	case 0xC0:
		divisor = (IS_BUZZY_DIST_C || CLOCK_15) ? 2.5 : 7.5;
		v_modulo = (CLOCK_15) ? 5 : 15;
		if (IS_UNSTABLE_DIST_C) divisor = 1.5;
		audf = get_audf(freq, coarse_divisor, divisor, modoffset);
		
		if ((CLOCK_15 && (audf + modoffset) % v_modulo == 0) || 
		(IS_BUZZY_DIST_C && ((audf + modoffset) % 3 != 0 || (audf + modoffset) % 5 == 0)) || 
		(IS_GRITTY_DIST_C && ((audf + modoffset) % 3 == 0 || (audf + modoffset) % 5 == 0)) || 
		(IS_UNSTABLE_DIST_C && ((audf + modoffset) % 3 == 0 || (audf + modoffset) % 5 != 0))) 
		{
			audf = delta_audf(audf, freq, coarse_divisor, divisor, modoffset, distortion); //aaaaaa
			if (!JOIN_16BIT && (audf > 0xFF && audf < 0x103)) audf = 0xFF; 
			//failsafe, in case a value goes beyond $FF, force it to never go further, otherwise it is invalid
		}

		if (!JOIN_16BIT && (audf > 0xFF || audf < 0x00)) break;	//invalid 8-bit range!
		if (JOIN_16BIT)
		{
			if (IS_BUZZY_DIST_C) tab_16bit_c_buzzy[note * 2] = audf;
			else if (IS_GRITTY_DIST_C) tab_16bit_c_gritty[note * 2] = audf;
			else if (IS_UNSTABLE_DIST_C) tab_16bit_c_unstable[note * 2] = audf;
			else break;	//invalid parameter
		}
		else if (CLOCK_179)
		{
			if (IS_BUZZY_DIST_C) tab_179mhz_c_buzzy[note] = audf;
			else if (IS_GRITTY_DIST_C) tab_179mhz_c_gritty[note] = audf;
			else if (IS_UNSTABLE_DIST_C) tab_179mhz_c_unstable[note] = audf;
			else break;	//invalid parameter

		}
		else if (CLOCK_15) tab_15khz_c_buzzy[note] = audf;
		else	//64khz mode
		{
			if (IS_BUZZY_DIST_C) tab_64khz_c_buzzy[note] = audf;
			else if (IS_GRITTY_DIST_C) tab_64khz_c_gritty[note] = audf;
			else if (IS_UNSTABLE_DIST_C) tab_64khz_c_unstable[note] = audf;
			else break;	//invalid parameter
		}
		break;

	}

}

double get_pitch(int audf, int coarse_divisor, double divisor, int modoffset)
{
	double PITCH = ((FREQ_17 / (coarse_divisor * divisor)) / (audf + modoffset)) / 2;
	return PITCH;
}

int get_audf(double freq, int coarse_divisor, double divisor, int modoffset)
{
	int audf = (int)round(((FREQ_17 / (coarse_divisor * divisor)) / (2 * freq)) - modoffset);
	return audf;
}

int delta_audf(int audf, double freq, int coarse_divisor, double divisor, int modoffset, int distortion)
{
	int tmp_audf_up = audf;		//begin from the currently invalid audf
	int tmp_audf_down = audf;
	double tmp_freq_up = 0;
	double tmp_freq_down = 0;
	double PITCH = 0;

	if (distortion != 0xC0) { tmp_audf_up++; tmp_audf_down--; }	//anything not distortion C, simpliest delta method
	else if (CLOCK_15)
	{
		for (int o = 0; o < 3; o++)	//MOD5 must be avoided!
		{
			if ((tmp_audf_up + modoffset) % 5 == 0) tmp_audf_up++;
			if ((tmp_audf_down + modoffset) % 5 == 0) tmp_audf_down--;
		}	
	}
	else if (IS_BUZZY_DIST_C)	//verify MOD3 integrity
	{
		for (int o = 0; o < 6; o++) 
		{
			if ((tmp_audf_up + modoffset) % 3 != 0 || (tmp_audf_up + modoffset) % 5 == 0) tmp_audf_up++;
			if ((tmp_audf_down + modoffset) % 3 != 0 || (tmp_audf_down + modoffset) % 5 == 0) tmp_audf_down--;
		}
	}
	else if (IS_GRITTY_DIST_C)	//verify neither MOD3 or MOD5 is used
	{
		for (int o = 0; o < 6; o++)	//get the closest compromise up and down first
		{
			if ((tmp_audf_up + modoffset) % 3 == 0 || (tmp_audf_up + modoffset) % 5 == 0) tmp_audf_up++;
			if ((tmp_audf_down + modoffset) % 3 == 0 || (tmp_audf_down + modoffset) % 5 == 0) tmp_audf_down--;
		}	
	}
	else if (IS_UNSTABLE_DIST_C)	//verify MOD5 integrity
	{
		for (int o = 0; o < 6; o++)	//get the closest compromise up and down first
		{
			if ((tmp_audf_up + modoffset) % 3 == 0 || (tmp_audf_up + modoffset) % 5 != 0) tmp_audf_up++;
			if ((tmp_audf_down + modoffset) % 3 == 0 || (tmp_audf_down + modoffset) % 5 != 0) tmp_audf_down--;
		}	
	}
	else return 0;	//invalid parameter most likely

	PITCH = get_pitch(tmp_audf_up, coarse_divisor, divisor, modoffset);
	tmp_freq_up = freq - PITCH;	//first delta, up
	PITCH = get_pitch(tmp_audf_down, coarse_divisor, divisor, modoffset);
	tmp_freq_down = PITCH - freq;	//second delta, down
	PITCH = tmp_freq_down - tmp_freq_up;
	if (PITCH > 0) audf = tmp_audf_up; //positive, meaning delta up is closer than delta down
	else audf = tmp_audf_down; //negative, meaning delta down is closer than delta up
	return audf;
}

double get_tuning()
{
	double tuning;
	while (true)
	{
		read_input (&tuning, "Please input the A-4 tuning first.\nFor example: 440, 432, 439.8, 440.83751645933, 444.895778867913, etc");
		if (tuning <= 400 || tuning >= 500)
		{
			if (tuning <= 400)
			{
				cout << "Please input a value above 400." << endl;
			}
			else
			{
				cout << "Please input values below 500." << endl;
			}
			wait(2);
			SEPARATOR;
		}
		else break;
	}
	return tuning;
}

int get_region()
{
	int region;
	while (true)
	{
		read_input (&region, "Machine region?\n1- PAL\n2- NTSC");
		if (region != 1 && region != 2)
		{
			cout << endl << "You must chose between option 1 or option 2 only." << endl;
			wait(2);
			SEPARATOR;
		}
		else break;
	}
	return region;
}

template <typename T>
void read_input(T* output, std::string prompt)
{
	cout << prompt << endl;
	while (true)
	{
		cout << "? ";
		if (!(cin >> *output))
		{
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Error: Invalid parameter!" << endl;
		}
		else break;
	}
}

void macro_table_gen(int distortion, int note_offset, bool IS_15KHZ, bool IS_179MHZ, bool IS_16BIT)
{	
	//generate the array
	for (int i = 0; i < 64; i++)
	{
		int note = i + note_offset;
		double freq = p[note * 12];
		generate_table(i, freq, distortion, IS_15KHZ, IS_179MHZ, IS_16BIT);
	}
	//generate the table output in .txt. This could technically be ignored if only the data in arrays is needed, eg for RMT Registers View or Tuning Editor.
	if (!IS_16BIT)
	{	
		int tab_array[64] = { 0 };	//empty index to overwrite
		
		for (int i = 0; i < 64; i++)
		{		
			if (IS_15KHZ)
			{
				switch (distortion)
				{
					case 0xA0:
						tab_array[i] = tab_15khz_a_pure[i];
						break;
					
					case 0xC0:
						tab_array[i] = tab_15khz_c_buzzy[i];
						break;
				}
			}
			else if (IS_179MHZ)
			{
				switch (distortion)
				{
					case 0x20:
						tab_array[i] = tab_179mhz_2[i];
						break;
					
					case 0xA0:
						tab_array[i] = tab_179mhz_a_pure[i];
						break;
					
					case 0xC0:
						if (IS_BUZZY_DIST_C) tab_array[i] = tab_179mhz_c_buzzy[i];
						else if (IS_GRITTY_DIST_C) tab_array[i] = tab_179mhz_c_gritty[i];
						else if (IS_UNSTABLE_DIST_C) tab_array[i] = tab_179mhz_c_unstable[i];
						break;
				}			
			}
			else
			{
				switch (distortion)
				{
					case 0x20:
						tab_array[i] = tab_64khz_2[i];
						break;
					
					case 0xA0:
						tab_array[i] = tab_64khz_a_pure[i];
						break;
					
					case 0xC0:
						if (IS_BUZZY_DIST_C) tab_array[i] = tab_64khz_c_buzzy[i];
						else if (IS_GRITTY_DIST_C) tab_array[i] = tab_64khz_c_gritty[i];
						else if (IS_UNSTABLE_DIST_C) tab_array[i] = tab_64khz_c_unstable[i];
						break;
				}
			}
			if (i % 12 == 0) cout << endl;
			cout << "$" << hex << setfill('0') << setw(2) << tab_array[i];
			if ((i - 11) % 12 != 0 && i != 63) cout << ",";
		}	
	}
	else 	//16-bit
	{
		int tab_array[128] = { 0 };	//empty index to overwrite
		
		for (int t = 0; t < 2; t++)	//iteration of loop, once per byte order
		{
			for (int i = 0; i < 64; i++)
			{
				switch (distortion)
				{
					case 0x20:
						tab_array[i * 2] = tab_16bit_2[i * 2];
						break;
					
					case 0xA0:
						tab_array[i * 2] = tab_16bit_a_pure[i * 2];
						break;
					
					case 0xC0:
						if (IS_BUZZY_DIST_C) tab_array[i * 2] = tab_16bit_c_buzzy[i * 2];
						else if (IS_GRITTY_DIST_C) tab_array[i * 2] = tab_16bit_c_gritty[i * 2];
						else if (IS_UNSTABLE_DIST_C) tab_array[i * 2] = tab_16bit_c_unstable[i * 2];
						break;
				}
				if (t == 0) 		//iteration 1
				{
					if (i == 0) cout << endl << "16-bit MSB\n";

					if (i % 12 == 0) cout << endl;
					int msb = tab_array[i * 2] >> 8;
					cout << "$" << hex << setfill('0') << setw(2) << msb;
					if ((i - 11) % 12 != 0 && i != 63) cout << ",";
				}
				else if (t == 1) 	//iteration 2
				{
					if (i == 0) cout << endl << "\n16-bit LSB\n";

					if (i % 12 == 0) cout << endl;
					int lsb = tab_array[i * 2] & 0x00FF;
					cout << "$" << hex << setfill('0') << setw(2) << lsb;
					if ((i - 11) % 12 != 0 && i != 63) cout << ",";
				}
	
			}
		
		}
		
	}
	
}



